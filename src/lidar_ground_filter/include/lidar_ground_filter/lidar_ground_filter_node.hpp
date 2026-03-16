#pragma once

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class LidarGroundFilterNode : public rclcpp::Node
{
public:
  explicit LidarGroundFilterNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  double ground_height_threshold_m_;
  double max_slope_deg_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr points_filtered_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr points_ground_removed_pub_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onPointsFiltered(sensor_msgs::msg::PointCloud2::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  sensor_msgs::msg::PointCloud2::SharedPtr removeGround(
    const sensor_msgs::msg::PointCloud2 & cloud) const;
};

}  // namespace traversability_generator
