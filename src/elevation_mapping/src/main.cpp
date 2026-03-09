#include "rclcpp/rclcpp.hpp"
#include "elevation_mapping/elevation_mapping_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::ElevationMappingNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
