#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ElevationMappingNode
//
// Thin ROS 2 wrapper around elevation_mapping_cupy (ETH Zurich).
// elevation_mapping_cupy is a CMake/Python package — it is brought in via
// rules_foreign_cc in WORKSPACE.bazel and its ROS 2 node is launched directly.
//
// This header exists as a placeholder so the Bazel target graph is complete.
// In practice, elevation_mapping_cupy's own node is added to the composable
// container in perception.launch.py using its registered plugin name.
//
// See: https://github.com/leggedrobotics/elevation_mapping_cupy
// ─────────────────────────────────────────────────────────────────────────────

#include <grid_map_msgs/msg/grid_map.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class ElevationMappingNode : public rclcpp::Node
{
public:
  explicit ElevationMappingNode(const rclcpp::NodeOptions & options);

private:
  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr lidar_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<grid_map_msgs::msg::GridMap>::SharedPtr elevation_map_pub_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onLidar(sensor_msgs::msg::PointCloud2::SharedPtr msg);
  void onOdom(nav_msgs::msg::Odometry::SharedPtr msg);
};

}  // namespace traversability_generator
