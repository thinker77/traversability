cmake_minimum_required(VERSION 3.8)
project(semantic_lifting)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_msgs REQUIRED)

add_library(semantic_lifting_node SHARED
  src/semantic_lifting_node.cpp
)
target_include_directories(semantic_lifting_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(semantic_lifting_node
  rclcpp
  rclcpp_components
  sensor_msgs
  tf2_msgs
)

rclcpp_components_register_node(semantic_lifting_node
  PLUGIN "traversability_generator::SemanticLiftingNode"
  EXECUTABLE semantic_lifting_main
)

install(TARGETS semantic_lifting_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
