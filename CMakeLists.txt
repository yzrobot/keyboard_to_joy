cmake_minimum_required(VERSION 2.8.3)
project(keyboard_to_joy)

find_package(catkin REQUIRED COMPONENTS roscpp sensor_msgs)

include_directories(include ${catkin_INCLUDE_DIRS})

catkin_package()

add_executable(keyboard_to_joy src/keyboard_to_joy.cpp)
target_link_libraries(keyboard_to_joy ${catkin_LIBRARIES})

install(TARGETS keyboard_to_joy
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
