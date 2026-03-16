#pragma once

#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class LidarAggregatorNode : public rclcpp::Node
{
public:
  explicit LidarAggregatorNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  int num_scans_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr points_raw_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr points_aggregated_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  std::vector<sensor_msgs::msg::PointCloud2> scan_buffer_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onPointsRaw(sensor_msgs::msg::PointCloud2::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  sensor_msgs::msg::PointCloud2::SharedPtr aggregate() const;
};

}  // namespace traversability_generator
