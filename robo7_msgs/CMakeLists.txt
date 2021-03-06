cmake_minimum_required(VERSION 2.8.3)
project(robo7_msgs)

find_package(catkin REQUIRED COMPONENTS roscpp rospy std_msgs geometry_msgs message_generation sensor_msgs phidgets
)

add_message_files(
  DIRECTORY msg
  FILES
  PWM.msg
  BatteryStatus.msg
  Encoders.msg
  ServoMotors.msg
  WheelAngularVelocities.msg
  Odometry.msg
  ControllerParams.msg
  XY_coordinates.msg
  path.msg
  paths.msg
  wallList.msg
  cornerList.msg
  Matrix4.msg
  Matrix3.msg
  MeasureRequest.msg
  trajectory_point.msg
  trajectory.msg
  target_trajectory_point.msg
  target_trajectory.msg
  destination_point.msg
  detectedState.msg
  MeasureFeedback.msg
  occupancy_matrix.msg
  occupancy_row.msg
  grid_matrix.msg
  grid_row.msg
  aObject.msg
  allObjects.msg
  former_position.msg
  dead_reck_param.msg
  robotPositionTest.msg
  wallPoint.msg
  aWall.msg
  the_robot_position.msg
  mapping_grid.msg
  matrix_row.msg
  matrix.msg
  activation_states.msg
  detectedObstacle.msg
  classifiedObj.msg
  allObstacles.msg
  detectedState.msg
)

generate_messages(
  DEPENDENCIES
  std_msgs
  geometry_msgs
  sensor_msgs
  phidgets
)

catkin_package(
  INCLUDE_DIRS
  LIBRARIES
  CATKIN_DEPENDS geometry_msgs message_runtime std_msgs sensor_msgs phidgets
  DEPENDS
)
