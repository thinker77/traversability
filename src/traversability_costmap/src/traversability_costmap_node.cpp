#include "traversability_costmap/traversability_costmap_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

TraversabilityCostmapNode::TraversabilityCostmapNode(const rclcpp::NodeOptions & options)
: Node("traversability_costmap_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  lethal_cost_threshold_ = declare_parameter("lethal_cost_threshold", 0.7);
  inflation_radius_m_    = declare_parameter("inflation_radius_m",    0.5);

  // ── Publishers ────────────────────────────────────────────────────────────
  costmap_pub_ = create_publisher<nav_msgs::msg::OccupancyGrid>(
    "costmap", rclcpp::QoS(10).reliable());

  // ── Subscribers ───────────────────────────────────────────────────────────
  stable_grid_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "stable_grid", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onStableGrid(msg); });

  RCLCPP_INFO(get_logger(), "TraversabilityCostmapNode ready");
}

void TraversabilityCostmapNode::onStableGrid(sensor_msgs::msg::Image::SharedPtr msg)
{
  costmap_pub_->publish(*toCostmap(*msg));
}

nav_msgs::msg::OccupancyGrid::SharedPtr TraversabilityCostmapNode::toCostmap(
  const sensor_msgs::msg::Image & grid) const
{
  // TODO: interpret each cell of the stable BEV feature grid as a traversability
  // score in [0, 1] and encode it as an OccupancyGrid value in [0, 100].
  // Apply inflation (expand lethal cells by inflation_radius_m_) and
  // remap to the nav2 costmap convention (0=free, 99=inscribed, 100=lethal).
  // Cells above lethal_cost_threshold_ are marked as lethal (100).
  (void)grid;
  auto out = std::make_shared<nav_msgs::msg::OccupancyGrid>();
  out->header      = grid.header;
  out->info.width  = grid.width;
  out->info.height = grid.height;
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::TraversabilityCostmapNode)
