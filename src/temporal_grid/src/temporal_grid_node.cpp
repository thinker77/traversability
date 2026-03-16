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
  feature_grid_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "feature_grid", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onFeatureGrid(msg); });

  RCLCPP_INFO(get_logger(), "TemporalGridFusionNode ready (history=%d, decay=%.2f)",
    history_length_, decay_factor_);
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
  // TODO: apply exponential moving average across grid_history_ using
  // decay_factor_ to produce a temporally-stable feature grid.
  // Older grids contribute less (weight = decay_factor_^age).
  auto out = std::make_shared<sensor_msgs::msg::Image>(grid_history_.back());
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::TemporalGridFusionNode)
