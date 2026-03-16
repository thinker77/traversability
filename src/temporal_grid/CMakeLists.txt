cmake_minimum_required(VERSION 3.8)
project(temporal_grid)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_msgs REQUIRED)

add_library(temporal_grid_node SHARED
  src/temporal_grid_node.cpp
)
target_include_directories(temporal_grid_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(temporal_grid_node
  rclcpp
  rclcpp_components
  sensor_msgs
  nav_msgs
  tf2_msgs
)

rclcpp_components_register_node(temporal_grid_node
  PLUGIN "traversability_generator::TemporalGridFusionNode"
  EXECUTABLE temporal_grid_main
)

install(TARGETS temporal_grid_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
