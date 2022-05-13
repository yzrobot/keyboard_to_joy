#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <signal.h>
#include <stdio.h>
#ifndef _WIN32
# include <termios.h>
# include <unistd.h>
#else
# include <windows.h>
#endif

#define KEYCODE_RIGHT 0x43
#define KEYCODE_LEFT 0x44
#define KEYCODE_UP 0x41
#define KEYCODE_DOWN 0x42

#define KEYCODE_A 0x61
#define KEYCODE_D 0x64
#define KEYCODE_E 0x65
#define KEYCODE_Q 0x71
#define KEYCODE_S 0x73
#define KEYCODE_W 0x77

class KeyboardReader
{
public:
  KeyboardReader()
#ifndef _WIN32
    : kfd(0)
#endif
  {
#ifndef _WIN32
    // get the console in raw mode
    tcgetattr(kfd, &cooked);
    struct termios raw;
    memcpy(&raw, &cooked, sizeof(struct termios));
    raw.c_lflag &=~ (ICANON | ECHO);
    // Setting a new line, then end of file
    raw.c_cc[VEOL] = 1;
    raw.c_cc[VEOF] = 2;
    tcsetattr(kfd, TCSANOW, &raw);
#endif
  }
  void readOne(char * c)
  {
#ifndef _WIN32
    int rc = read(kfd, c, 1);
    if (rc < 0)
      {
	throw std::runtime_error("read failed");
      }
#else
    for(;;)
      {
	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
	INPUT_RECORD buffer;
	DWORD events;
	PeekConsoleInput(handle, &buffer, 1, &events);
	if(events > 0)
	  {
	    ReadConsoleInput(handle, &buffer, 1, &events);
	    if (buffer.Event.KeyEvent.wVirtualKeyCode == VK_LEFT)
	      {
		*c = KEYCODE_LEFT;
		return;
	      }
	    else if (buffer.Event.KeyEvent.wVirtualKeyCode == VK_UP)
	      {
		*c = KEYCODE_UP;
		return;
	      }
	    else if (buffer.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT)
	      {
		*c = KEYCODE_RIGHT;
		return;
	      }
	    else if (buffer.Event.KeyEvent.wVirtualKeyCode == VK_DOWN)
	      {
		*c = KEYCODE_DOWN;
		return;
	      }
	    else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x44)
	      {
		*c = KEYCODE_D;
		return;
	      }
	    else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x45)
	      {
		*c = KEYCODE_E;
		return;
	      }
	    else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x51)
	      {
		*c = KEYCODE_Q;
		return;
	      }
	    else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x53)
	      {
		*c = KEYCODE_S;
		return;
	      }
	    else if (buffer.Event.KeyEvent.wVirtualKeyCode == 0x57)
	      {
		*c = KEYCODE_W;
		return;
	      }
	  }
      }
#endif
  }
  void shutdown()
  {
#ifndef _WIN32
    tcsetattr(kfd, TCSANOW, &cooked);
#endif
  }
private:
#ifndef _WIN32
  int kfd;
  struct termios cooked;
#endif
};

KeyboardReader input;

class Keyboard2Joy
{
public:
  Keyboard2Joy();
  void keyLoop();

private:
  ros::NodeHandle nh_;
  int axes_, buttons_, left_, right_, up_, down_, x_a_, o_b_, square_x_, triangle_y_, r1_rb_, r2_rt_, l1_lb_, l2_lt_;
  ros::Publisher joy_pub_;
};

Keyboard2Joy::Keyboard2Joy()
{
  nh_.param("axes", axes_, axes_);
  nh_.param("buttons", buttons_, buttons_);
  
  nh_.param("left", left_, left_);
  nh_.param("right", left_, left_);
  nh_.param("up", up_, up_);
  nh_.param("down", down_, down_);
  
  nh_.param("x_a", x_a_, x_a_);
  nh_.param("o_b", o_b_, o_b_);
  nh_.param("square_x", square_x_, square_x_);
  nh_.param("triangle_y", triangle_y_, triangle_y_);
  
  nh_.param("l1_lb", l1_lb_, l1_lb_);
  nh_.param("l2_lt", l2_lt_, l2_lt_);
  
  nh_.param("r1_rb", r1_rb_, r1_rb_);
  nh_.param("r2_rt", r2_rt_, r2_rt_);
  
  joy_pub_ = nh_.advertise<sensor_msgs::Joy>("joy", 1);
}

void quit(int sig)
{
  (void)sig;
  input.shutdown();
  ros::shutdown();
  exit(0);
}


int main(int argc, char** argv)
{
  ros::init(argc, argv, "keyboard_to_joy");
  Keyboard2Joy keyboard_to_joy;

  signal(SIGINT,quit);

  keyboard_to_joy.keyLoop();
  quit(0);
  
  return(0);
}


void Keyboard2Joy::keyLoop()
{
  char c;
  bool dirty=false;
  sensor_msgs::Joy joy;
  joy.axes.resize(axes_);
  joy.buttons.resize(buttons_);
  
  puts("Reading from keyboard");
  puts("---------------------------");
  puts("Use arrow keys 'q' 'w' 'e' 'a' 's' 'd' to simulate the joystick.");
  
  for(;;)
    {
      // get the next event from the keyboard  
      try
	{
	  input.readOne(&c);
	}
      catch (const std::runtime_error &)
	{
	  perror("read():");
	  return;
	}
      
      ROS_DEBUG("value: 0x%02X\n", c);
      
      for (size_t i = 0; i < joy.axes.size(); i++)
	{
	  joy.axes[i] = 0;
	}
      for (size_t i = 0; i < joy.buttons.size(); i++)
	{
	  joy.buttons[i] = 0;
	}
      
      switch(c)
	{
	case KEYCODE_LEFT:
	  ROS_DEBUG("LEFT");
	  joy.axes[left_] = 1;
	  dirty = true;
	  break;
	case KEYCODE_RIGHT:
	  ROS_DEBUG("RIGHT");
	  joy.axes[right_] = -1;
	  dirty = true;
	  break;
	case KEYCODE_UP:
	  ROS_DEBUG("UP");
	  joy.axes[up_] = 1;
	  dirty = true;
	  break;
	case KEYCODE_DOWN:
	  ROS_DEBUG("DOWN");
	  joy.axes[down_] = -1;
	  dirty = true;
	  break;
	case KEYCODE_A:
	  ROS_DEBUG("A");
	  joy.buttons[square_x_] = 1;
	  dirty = true;
	  break;
	case KEYCODE_D:
	  ROS_DEBUG("D");
	  joy.buttons[o_b_] = 1;
	  dirty = true;
	  break;
	case KEYCODE_E:
	  ROS_DEBUG("E");
	  joy.buttons[r1_rb_] = 1;
	  dirty = true;
	  break;
	case KEYCODE_Q:
	  ROS_DEBUG("Q");
	  joy.buttons[l1_lb_] = 1;
	  dirty = true;
	  break;
	case KEYCODE_S:
	  ROS_DEBUG("S");
	  joy.buttons[x_a_] = 1;
	  dirty = true;
	  break;
	case KEYCODE_W:
	  ROS_DEBUG("W");
	  joy.buttons[triangle_y_] = 1;
	  dirty = true;
	  break;
	}
      
      if(dirty ==true)
	{
	  joy_pub_.publish(joy);
	  dirty=false;
	}
    }

  return;
}



