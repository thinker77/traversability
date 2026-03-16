#include "lidar_aggregator/lidar_aggregator_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

LidarAggregatorNode::LidarAggregatorNode(const rclcpp::NodeOptions & options)
: Node("lidar_aggregator_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  num_scans_ = declare_parameter("num_scans", 3);

  // ── Publishers ────────────────────────────────────────────────────────────
  points_aggregated_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
    "points_aggregated", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  points_raw_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    "points_raw", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::PointCloud2::SharedPtr msg) { onPointsRaw(msg); });

  RCLCPP_INFO(get_logger(), "LidarAggregatorNode ready (num_scans=%d)", num_scans_);
}

void LidarAggregatorNode::onPointsRaw(sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
  scan_buffer_.push_back(*msg);
  while (static_cast<int>(scan_buffer_.size()) > num_scans_) {
    scan_buffer_.erase(scan_buffer_.begin());
  }

  auto aggregated = aggregate();
  points_aggregated_pub_->publish(*aggregated);
}

sensor_msgs::msg::PointCloud2::SharedPtr LidarAggregatorNode::aggregate() const
{
  // TODO: concatenate all scans in scan_buffer_ into a single PointCloud2,
  // transforming each scan into the frame of the most recent one before merging.
  auto out = std::make_shared<sensor_msgs::msg::PointCloud2>(scan_buffer_.back());
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::LidarAggregatorNode)
