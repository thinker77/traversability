#pragma once

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class BevFusionNode : public rclcpp::Node
{
public:
  explicit BevFusionNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  double resolution_m_;
  double grid_width_m_;
  double grid_height_m_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr elevation_map_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr slope_map_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr roughness_map_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr obstacle_map_sub_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr semantic_points_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr feature_grid_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  sensor_msgs::msg::Image::SharedPtr latest_elevation_;
  sensor_msgs::msg::Image::SharedPtr latest_slope_;
  sensor_msgs::msg::Image::SharedPtr latest_roughness_;
  sensor_msgs::msg::Image::SharedPtr latest_obstacle_;
  sensor_msgs::msg::PointCloud2::SharedPtr latest_semantic_points_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onElevationMap(sensor_msgs::msg::Image::SharedPtr msg);
  void onSlopeMap(sensor_msgs::msg::Image::SharedPtr msg);
  void onRoughnessMap(sensor_msgs::msg::Image::SharedPtr msg);
  void onObstacleMap(sensor_msgs::msg::Image::SharedPtr msg);
  void onSemanticPoints(sensor_msgs::msg::PointCloud2::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  void tryFuse();
  sensor_msgs::msg::Image::SharedPtr fuse() const;
};

}  // namespace traversability_generator
