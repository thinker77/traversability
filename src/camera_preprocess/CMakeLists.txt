cmake_minimum_required(VERSION 3.8)
project(camera_preprocess)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)

add_library(camera_preprocess_node SHARED
  src/camera_preprocess_node.cpp
)
target_include_directories(camera_preprocess_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(camera_preprocess_node
  rclcpp
  rclcpp_components
  sensor_msgs
)

rclcpp_components_register_node(camera_preprocess_node
  PLUGIN "traversability_generator::CameraPreprocessNode"
  EXECUTABLE camera_preprocess_main
)

install(TARGETS camera_preprocess_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
