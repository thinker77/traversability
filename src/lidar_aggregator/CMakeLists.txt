cmake_minimum_required(VERSION 3.8)
project(lidar_aggregator)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)

add_library(lidar_aggregator_node SHARED
  src/lidar_aggregator_node.cpp
)
target_include_directories(lidar_aggregator_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(lidar_aggregator_node
  rclcpp
  rclcpp_components
  sensor_msgs
)

rclcpp_components_register_node(lidar_aggregator_node
  PLUGIN "traversability_generator::LidarAggregatorNode"
  EXECUTABLE lidar_aggregator_main
)

install(TARGETS lidar_aggregator_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
