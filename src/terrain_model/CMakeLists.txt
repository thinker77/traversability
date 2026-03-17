cmake_minimum_required(VERSION 3.8)
project(terrain_model)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_msgs REQUIRED)

add_library(terrain_model_node SHARED
  src/terrain_model_node.cpp
)
target_include_directories(terrain_model_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(terrain_model_node
  rclcpp
  rclcpp_components
  sensor_msgs
  nav_msgs
  tf2_msgs
)

rclcpp_components_register_node(terrain_model_node
  PLUGIN "traversability_generator::TerrainModelNode"
  EXECUTABLE terrain_model_main
)

install(TARGETS terrain_model_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
