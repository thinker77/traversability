#include "rclcpp/rclcpp.hpp"
#include "terrain_model/terrain_model_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::TerrainModelNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
