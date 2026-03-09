#pragma once

#include <map>
#include <string>

#include <grid_map_msgs/msg/grid_map.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <rclcpp/rclcpp.hpp>

namespace traversability_generator
{

class CostmapFusionNode : public rclcpp::Node
{
public:
  explicit CostmapFusionNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  double resolution_m_;
  double grid_width_m_;
  double grid_height_m_;
  std::map<std::string, double> class_costs_;   // class label → [0.0, 1.0]
  double elevation_jump_threshold_m_;
  double geo_traversability_weight_;
  int inflation_radius_cells_;
  std::string output_frame_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr bev_grid_sub_;
  rclcpp::Subscription<grid_map_msgs::msg::GridMap>::SharedPtr elevation_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr traversal_costmap_pub_;
  rclcpp::Publisher<grid_map_msgs::msg::GridMap>::SharedPtr elevation_out_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  nav_msgs::msg::OccupancyGrid::SharedPtr latest_bev_grid_;
  grid_map_msgs::msg::GridMap::SharedPtr latest_elevation_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onBevGrid(nav_msgs::msg::OccupancyGrid::SharedPtr msg);
  void onElevationMap(grid_map_msgs::msg::GridMap::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  // Combines semantic cost + geometric traversability → final cost map.
  nav_msgs::msg::OccupancyGrid::SharedPtr fuse(
    const nav_msgs::msg::OccupancyGrid & bev_grid,
    const grid_map_msgs::msg::GridMap & elevation);

  // Raises cost for cells with elevation discontinuity > threshold.
  void applyElevationJumpPenalty(
    nav_msgs::msg::OccupancyGrid & costmap,
    const grid_map_msgs::msg::GridMap & elevation);

  // Morphological dilation around obstacle cells.
  void inflate(nav_msgs::msg::OccupancyGrid & costmap);

  void tryProcess();
};

}  // namespace traversability_generator
