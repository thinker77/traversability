#include "rclcpp/rclcpp.hpp"
#include "sensor_sync/sensor_sync_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::SensorSyncNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
