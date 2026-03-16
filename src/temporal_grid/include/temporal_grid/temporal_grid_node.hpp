#pragma once

#include <deque>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace traversability_generator
{

class TemporalGridFusionNode : public rclcpp::Node
{
public:
  explicit TemporalGridFusionNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  int history_length_;
  double decay_factor_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr feature_grid_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr stable_grid_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  std::deque<sensor_msgs::msg::Image> grid_history_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onFeatureGrid(sensor_msgs::msg::Image::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  sensor_msgs::msg::Image::SharedPtr fuseHistory() const;
};

}  // namespace traversability_generator
