#include "elevation_mapping/elevation_mapping_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

ElevationMappingNode::ElevationMappingNode(const rclcpp::NodeOptions & options)
: Node("elevation_mapping_node", options)
{
  // ── Publishers ────────────────────────────────────────────────────────────
  elevation_map_pub_ = create_publisher<grid_map_msgs::msg::GridMap>(
    "elevation_map", rclcpp::QoS(10).reliable());

  // ── Subscribers ───────────────────────────────────────────────────────────
  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "odom", rclcpp::SensorDataQoS(),
    [this](nav_msgs::msg::Odometry::SharedPtr msg) { onOdom(msg); });

  lidar_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    "lidar_synced", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::PointCloud2::SharedPtr msg) { onLidar(msg); });

  RCLCPP_INFO(get_logger(),
    "ElevationMappingNode: stub — replace with elevation_mapping_cupy in production. "
    "See src/elevation_mapping/include/elevation_mapping/elevation_mapping_node.hpp");
}

void ElevationMappingNode::onOdom(nav_msgs::msg::Odometry::SharedPtr msg)
{
  // TODO: forward to elevation_mapping_cupy pose input.
  (void)msg;
}

void ElevationMappingNode::onLidar(sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
  // TODO: forward point cloud to elevation_mapping_cupy.
  // elevation_mapping_cupy will publish the GridMap on its own topic;
  // remap that topic to /elevation_map in the launch file.
  (void)msg;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::ElevationMappingNode)
