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
  traversability_grid_pub_ = create_publisher<nav_msgs::msg::OccupancyGrid>(
    "traversability_grid", rclcpp::QoS(10).reliable());

  planner_costmap_pub_ = create_publisher<nav_msgs::msg::OccupancyGrid>(
    "planner_costmap", rclcpp::QoS(10).reliable());

  // ── Subscribers ───────────────────────────────────────────────────────────
  stable_grid_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "stable_grid", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onStableGrid(msg); });

  RCLCPP_INFO(get_logger(), "TraversabilityCostmapNode ready");
}

void TraversabilityCostmapNode::onStableGrid(sensor_msgs::msg::Image::SharedPtr msg)
{
  auto trav_grid = toTraversabilityGrid(*msg);
  traversability_grid_pub_->publish(*trav_grid);

  auto costmap = toPlannerCostmap(*trav_grid);
  planner_costmap_pub_->publish(*costmap);
}

nav_msgs::msg::OccupancyGrid::SharedPtr TraversabilityCostmapNode::toTraversabilityGrid(
  const sensor_msgs::msg::Image & grid) const
{
  // TODO: interpret each cell of the stable BEV feature grid as a traversability
  // score in [0, 1] and encode it as an OccupancyGrid value in [0, 100].
  // Cells above lethal_cost_threshold_ are marked as lethal (100).
  (void)grid;
  auto out = std::make_shared<nav_msgs::msg::OccupancyGrid>();
  out->header = grid.header;
  out->info.width    = grid.width;
  out->info.height   = grid.height;
  return out;
}

nav_msgs::msg::OccupancyGrid::SharedPtr TraversabilityCostmapNode::toPlannerCostmap(
  const nav_msgs::msg::OccupancyGrid & traversability_grid) const
{
  // TODO: apply inflation (expand lethal cells by inflation_radius_m_) and
  // remap values to the nav2 costmap convention (0=free, 100=lethal, 99=inscribed).
  auto out = std::make_shared<nav_msgs::msg::OccupancyGrid>(traversability_grid);
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::TraversabilityCostmapNode)
