#pragma once

#include <deque>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class LidarPreprocessNode : public rclcpp::Node
{
public:
  explicit LidarPreprocessNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  bool deskew_enabled_;
  int imu_queue_size_;
  double min_range_m_;
  double max_range_m_;
  double voxel_leaf_size_m_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr points_raw_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr points_proc_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  std::deque<sensor_msgs::msg::Imu> imu_queue_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onPointsRaw(sensor_msgs::msg::PointCloud2::SharedPtr msg);
  void onImu(sensor_msgs::msg::Imu::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  sensor_msgs::msg::PointCloud2::SharedPtr deskew(
    const sensor_msgs::msg::PointCloud2 & cloud);
  sensor_msgs::msg::PointCloud2::SharedPtr filterRange(
    const sensor_msgs::msg::PointCloud2 & cloud);
  sensor_msgs::msg::PointCloud2::SharedPtr voxelDownsample(
    const sensor_msgs::msg::PointCloud2 & cloud);
};

}  // namespace traversability_generator
