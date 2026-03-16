cmake_minimum_required(VERSION 3.8)
project(traversability_costmap)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

add_library(traversability_costmap_node SHARED
  src/traversability_costmap_node.cpp
)
target_include_directories(traversability_costmap_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(traversability_costmap_node
  rclcpp
  rclcpp_components
  sensor_msgs
  nav_msgs
)

rclcpp_components_register_node(traversability_costmap_node
  PLUGIN "traversability_generator::TraversabilityCostmapNode"
  EXECUTABLE traversability_costmap_main
)

install(TARGETS traversability_costmap_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
