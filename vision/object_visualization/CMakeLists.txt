cmake_minimum_required(VERSION 2.8.3)
project(object_visualization)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  geometry_msgs
  robo7_msgs
  phidgets
  pcl_conversions
  pcl_ros
  robo7_srvs
)

catkin_package(
 CATKIN_DEPENDS roscpp std_msgs geometry_msgs robo7_msgs phidgets pcl_conversions pcl_ros robo7_srvs
)


include_directories(
 ${catkin_INCLUDE_DIRS}
)

add_executable(object_to_robot src/object_to_robot.cpp)
target_link_libraries(object_to_robot ${catkin_LIBRARIES})
add_dependencies(object_to_robot ${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
