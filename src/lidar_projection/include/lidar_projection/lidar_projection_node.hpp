#pragma once

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class LidarProjectionNode : public rclcpp::Node
{
public:
  explicit LidarProjectionNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  int image_height_;
  int image_width_;
  double max_depth_m_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr points_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr projected_depth_pub_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onPoints(sensor_msgs::msg::PointCloud2::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  sensor_msgs::msg::Image::SharedPtr project(
    const sensor_msgs::msg::PointCloud2 & cloud) const;
};

}  // namespace traversability_generator
