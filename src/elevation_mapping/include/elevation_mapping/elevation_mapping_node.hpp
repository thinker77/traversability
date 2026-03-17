#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ElevationMappingNode — stub (superseded by terrain_model package)
//
// This package is no longer part of the active pipeline.
// Terrain modelling is now handled by src/terrain_model/TerrainModelNode.
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
