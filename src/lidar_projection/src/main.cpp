#include "rclcpp/rclcpp.hpp"
#include "lidar_projection/lidar_projection_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::LidarProjectionNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
