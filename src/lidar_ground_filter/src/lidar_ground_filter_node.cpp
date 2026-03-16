#include "lidar_ground_filter/lidar_ground_filter_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

LidarGroundFilterNode::LidarGroundFilterNode(const rclcpp::NodeOptions & options)
: Node("lidar_ground_filter_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  ground_height_threshold_m_ = declare_parameter("ground_height_threshold_m", 0.2);
  max_slope_deg_             = declare_parameter("max_slope_deg", 8.0);

  // ── Publishers ────────────────────────────────────────────────────────────
  points_ground_removed_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
    "points_ground_removed", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  points_filtered_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    "points_filtered", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::PointCloud2::SharedPtr msg) { onPointsFiltered(msg); });

  RCLCPP_INFO(get_logger(), "LidarGroundFilterNode ready");
}

void LidarGroundFilterNode::onPointsFiltered(sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
  auto out = removeGround(*msg);
  points_ground_removed_pub_->publish(*out);
}

sensor_msgs::msg::PointCloud2::SharedPtr LidarGroundFilterNode::removeGround(
  const sensor_msgs::msg::PointCloud2 & cloud) const
{
  // TODO: segment ground plane (e.g., RANSAC plane fit or height-threshold scan line)
  // and return only non-ground points. Use ground_height_threshold_m_ and max_slope_deg_.
  auto out = std::make_shared<sensor_msgs::msg::PointCloud2>(cloud);
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::LidarGroundFilterNode)
