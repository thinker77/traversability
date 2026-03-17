#pragma once

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <tf2_msgs/msg/tf_message.hpp>

namespace traversability_generator
{

class TerrainModelNode : public rclcpp::Node
{
public:
  explicit TerrainModelNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  double resolution_m_;
  double map_length_x_m_;
  double map_length_y_m_;
  double sensor_cutoff_min_depth_m_;
  double sensor_cutoff_max_depth_m_;
  double time_tolerance_sec_;
  double tall_object_height_threshold_m_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr points_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<tf2_msgs::msg::TFMessage>::SharedPtr tf_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr elevation_map_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr slope_map_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr roughness_map_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr obstacle_map_pub_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr points_no_tall_objects_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  nav_msgs::msg::Odometry::SharedPtr latest_odom_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onPoints(sensor_msgs::msg::PointCloud2::SharedPtr msg);
  void onOdom(nav_msgs::msg::Odometry::SharedPtr msg);
  void onTf(tf2_msgs::msg::TFMessage::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  sensor_msgs::msg::PointCloud2::SharedPtr removeTallObjects(
    const sensor_msgs::msg::PointCloud2 & cloud) const;
  void buildTerrainMaps(
    const sensor_msgs::msg::PointCloud2 & cloud,
    const std_msgs::msg::Header & header);
};

}  // namespace traversability_generator
