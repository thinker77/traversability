#include "rclcpp/rclcpp.hpp"
#include "temporal_grid/temporal_grid_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::TemporalGridFusionNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
