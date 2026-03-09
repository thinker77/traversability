#include "costmap_fusion/costmap_fusion_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

CostmapFusionNode::CostmapFusionNode(const rclcpp::NodeOptions & options)
: Node("costmap_fusion_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  resolution_m_              = declare_parameter("resolution_m", 0.1);
  grid_width_m_              = declare_parameter("grid_width_m", 20.0);
  grid_height_m_             = declare_parameter("grid_height_m", 20.0);
  elevation_jump_threshold_m_= declare_parameter("elevation_jump_threshold_m", 0.3);
  geo_traversability_weight_ = declare_parameter("geo_traversability_weight", 0.4);
  inflation_radius_cells_    = declare_parameter("inflation_radius_cells", 3);
  output_frame_              = declare_parameter("output_frame", std::string("base_link"));

  // Semantic class costs
  const std::vector<std::string> labels =
    {"road", "grass", "vegetation", "obstacle", "vehicle", "unknown"};
  const std::vector<double> defaults = {0.1, 0.3, 0.6, 1.0, 1.0, 0.8};
  for (size_t i = 0; i < labels.size(); ++i) {
    class_costs_[labels[i]] = declare_parameter("class_costs." + labels[i], defaults[i]);
  }

  // ── Publishers ────────────────────────────────────────────────────────────
  traversal_costmap_pub_ = create_publisher<nav_msgs::msg::OccupancyGrid>(
    "traversal_costmap", rclcpp::QoS(10).reliable());
  elevation_out_pub_ = create_publisher<grid_map_msgs::msg::GridMap>(
    "elevation_out", rclcpp::QoS(10).reliable());

  // ── Subscribers ───────────────────────────────────────────────────────────
  elevation_sub_ = create_subscription<grid_map_msgs::msg::GridMap>(
    "elevation_map", rclcpp::QoS(10).reliable(),
    [this](grid_map_msgs::msg::GridMap::SharedPtr msg) {
      latest_elevation_ = msg;
      tryProcess();
    });

  bev_grid_sub_ = create_subscription<nav_msgs::msg::OccupancyGrid>(
    "bev_semantic_grid", rclcpp::QoS(10).reliable(),
    [this](nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
      latest_bev_grid_ = msg;
      tryProcess();
    });

  RCLCPP_INFO(get_logger(), "CostmapFusionNode ready (frame=%s)", output_frame_.c_str());
}

void CostmapFusionNode::onBevGrid(nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
  latest_bev_grid_ = msg;
}

void CostmapFusionNode::onElevationMap(grid_map_msgs::msg::GridMap::SharedPtr msg)
{
  latest_elevation_ = msg;
}

void CostmapFusionNode::tryProcess()
{
  if (!latest_bev_grid_ || !latest_elevation_) {
    return;
  }

  auto costmap = fuse(*latest_bev_grid_, *latest_elevation_);
  applyElevationJumpPenalty(*costmap, *latest_elevation_);
  inflate(*costmap);

  traversal_costmap_pub_->publish(*costmap);
  elevation_out_pub_->publish(*latest_elevation_);
}

nav_msgs::msg::OccupancyGrid::SharedPtr CostmapFusionNode::fuse(
  const nav_msgs::msg::OccupancyGrid & bev_grid,
  const grid_map_msgs::msg::GridMap & elevation)
{
  // TODO:
  // For each cell i in bev_grid:
  //   1. semantic_class = bev_grid.data[i]
  //   2. base_cost = class_costs_[class_labels[semantic_class]]
  //   3. geo_score = elevation traversability layer value at matching cell
  //      (slope/roughness combined → 0=easy, 1=impassable)
  //   4. final_cost = base_cost * (1 - geo_traversability_weight_)
  //                 + geo_score * geo_traversability_weight_
  //   5. Clamp to [0, 100] for OccupancyGrid convention.
  (void)elevation;
  auto costmap = std::make_shared<nav_msgs::msg::OccupancyGrid>(bev_grid);
  return costmap;
}

void CostmapFusionNode::applyElevationJumpPenalty(
  nav_msgs::msg::OccupancyGrid & costmap,
  const grid_map_msgs::msg::GridMap & elevation)
{
  // TODO:
  // For each cell, compute max elevation difference to its 8-neighbors.
  // If diff > elevation_jump_threshold_m_ → set cost to 100 (obstacle).
  // This catches unlabeled static objects (rocks, logs) that appear as
  // elevation discontinuities even if the seg model missed them.
  (void)costmap;
  (void)elevation;
}

void CostmapFusionNode::inflate(nav_msgs::msg::OccupancyGrid & costmap)
{
  // TODO: morphological dilation — for each obstacle cell (cost==100),
  // raise cost of all cells within inflation_radius_cells_ to obstacle cost.
  (void)costmap;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::CostmapFusionNode)
