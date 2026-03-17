#pragma once

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace traversability_generator
{

class TraversabilityCostmapNode : public rclcpp::Node
{
public:
  explicit TraversabilityCostmapNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  double lethal_cost_threshold_;
  double inflation_radius_m_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr stable_grid_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr costmap_pub_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onStableGrid(sensor_msgs::msg::Image::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  nav_msgs::msg::OccupancyGrid::SharedPtr toCostmap(
    const sensor_msgs::msg::Image & grid) const;
};

}  // namespace traversability_generator
