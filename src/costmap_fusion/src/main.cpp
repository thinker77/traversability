#include "rclcpp/rclcpp.hpp"
#include "costmap_fusion/costmap_fusion_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::CostmapFusionNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
