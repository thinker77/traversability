cmake_minimum_required(VERSION 3.8)
project(bev_fusion)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(grid_map_msgs REQUIRED)
find_package(tf2_msgs REQUIRED)

add_library(bev_fusion_node SHARED
  src/bev_fusion_node.cpp
)
target_include_directories(bev_fusion_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(bev_fusion_node
  rclcpp
  rclcpp_components
  sensor_msgs
  nav_msgs
  grid_map_msgs
  tf2_msgs
)

rclcpp_components_register_node(bev_fusion_node
  PLUGIN "traversability_generator::BevFusionNode"
  EXECUTABLE bev_fusion_main
)

install(TARGETS bev_fusion_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
