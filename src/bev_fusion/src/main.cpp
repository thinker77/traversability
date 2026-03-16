#include "rclcpp/rclcpp.hpp"
#include "bev_fusion/bev_fusion_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::BevFusionNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
