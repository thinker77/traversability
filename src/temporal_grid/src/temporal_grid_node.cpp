#include "temporal_grid/temporal_grid_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

TemporalGridFusionNode::TemporalGridFusionNode(const rclcpp::NodeOptions & options)
: Node("temporal_grid_fusion_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  history_length_ = declare_parameter("history_length", 5);
  decay_factor_   = declare_parameter("decay_factor",   0.8);

  // ── Publishers ────────────────────────────────────────────────────────────
  stable_grid_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "stable_grid", rclcpp::QoS(10).reliable());

  // ── Subscribers ───────────────────────────────────────────────────────────
  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "odometry", rclcpp::SensorDataQoS(),
    [this](nav_msgs::msg::Odometry::SharedPtr msg) { onOdom(msg); });

  feature_grid_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "feature_grid", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onFeatureGrid(msg); });

  RCLCPP_INFO(get_logger(), "TemporalGridFusionNode ready (history=%d, decay=%.2f)",
    history_length_, decay_factor_);
}

void TemporalGridFusionNode::onOdom(nav_msgs::msg::Odometry::SharedPtr msg)
{
  // TODO: use odometry for motion-compensated warping of historical grids
  // into the current ego frame before temporal fusion.
  latest_odom_ = msg;
}

void TemporalGridFusionNode::onFeatureGrid(sensor_msgs::msg::Image::SharedPtr msg)
{
  grid_history_.push_back(*msg);
  while (static_cast<int>(grid_history_.size()) > history_length_) {
    grid_history_.pop_front();
  }

  auto stable = fuseHistory();
  stable_grid_pub_->publish(*stable);
}

sensor_msgs::msg::Image::SharedPtr TemporalGridFusionNode::fuseHistory() const
{
  // TODO: warp each historical grid into the current frame using latest_odom_
  // delta poses, then apply exponential moving average with decay_factor_^age
  // weighting to produce a temporally-stable feature grid.
  auto out = std::make_shared<sensor_msgs::msg::Image>(grid_history_.back());
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::TemporalGridFusionNode)
