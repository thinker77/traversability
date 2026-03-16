cmake_minimum_required(VERSION 3.8)
project(lidar_ground_filter)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)

add_library(lidar_ground_filter_node SHARED
  src/lidar_ground_filter_node.cpp
)
target_include_directories(lidar_ground_filter_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(lidar_ground_filter_node
  rclcpp
  rclcpp_components
  sensor_msgs
)

rclcpp_components_register_node(lidar_ground_filter_node
  PLUGIN "traversability_generator::LidarGroundFilterNode"
  EXECUTABLE lidar_ground_filter_main
)

install(TARGETS lidar_ground_filter_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
