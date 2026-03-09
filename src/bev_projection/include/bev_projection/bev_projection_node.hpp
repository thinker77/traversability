#pragma once

#include <string>

#include <grid_map_msgs/msg/grid_map.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace traversability_generator
{

class BevProjectionNode : public rclcpp::Node
{
public:
  explicit BevProjectionNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  double resolution_m_;
  double grid_width_m_;
  double grid_height_m_;
  std::string projection_surface_;  // "flat" | "elevation"
  std::string aggregation_method_;
  int unknown_class_id_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr seg_mask_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr depth_sub_;
  rclcpp::Subscription<grid_map_msgs::msg::GridMap>::SharedPtr elevation_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr bev_grid_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  sensor_msgs::msg::Image::SharedPtr latest_depth_;
  grid_map_msgs::msg::GridMap::SharedPtr latest_elevation_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onSegMask(sensor_msgs::msg::Image::SharedPtr msg);
  void onDepth(sensor_msgs::msg::Image::SharedPtr msg);
  void onElevationMap(grid_map_msgs::msg::GridMap::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  // Unprojects each pixel to 3D using depth + camera intrinsics,
  // then bins the resulting 3D semantic point into the BEV grid.
  nav_msgs::msg::OccupancyGrid::SharedPtr project(
    const sensor_msgs::msg::Image & seg_mask,
    const sensor_msgs::msg::Image & depth);

  // Returns z-height for a BEV cell from the elevation map (or 0 if "flat").
  float getProjectionHeight(float bev_x, float bev_y);

  void tryProcess();
};

}  // namespace traversability_generator
