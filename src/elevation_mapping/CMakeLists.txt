cmake_minimum_required(VERSION 3.8)
project(elevation_mapping)

# Stub — replace with elevation_mapping_cupy via FetchContent in production

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(grid_map_msgs REQUIRED)

add_library(elevation_mapping_node SHARED
  src/elevation_mapping_node.cpp
)
target_include_directories(elevation_mapping_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(elevation_mapping_node
  rclcpp
  rclcpp_components
  sensor_msgs
  nav_msgs
  grid_map_msgs
)

rclcpp_components_register_node(elevation_mapping_node
  PLUGIN "traversability_generator::ElevationMappingNode"
  EXECUTABLE elevation_mapping_main
)

install(TARGETS elevation_mapping_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
