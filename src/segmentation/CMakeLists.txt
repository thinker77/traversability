cmake_minimum_required(VERSION 3.8)
project(segmentation)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)

add_library(segmentation_node SHARED
  src/segmentation_node.cpp
)
target_include_directories(segmentation_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(segmentation_node
  rclcpp
  rclcpp_components
  sensor_msgs
)

rclcpp_components_register_node(segmentation_node
  PLUGIN "traversability_generator::SegmentationNode"
  EXECUTABLE segmentation_main
)

install(TARGETS segmentation_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
