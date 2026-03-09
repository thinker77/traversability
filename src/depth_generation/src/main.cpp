#include "rclcpp/rclcpp.hpp"
#include "depth_generation/depth_generation_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::DepthGenerationNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
