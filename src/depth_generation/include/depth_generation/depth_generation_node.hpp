#pragma once

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class DepthGenerationNode : public rclcpp::Node
{
public:
  explicit DepthGenerationNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  std::string densification_method_;
  double max_depth_m_;
  std::string output_encoding_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr lidar_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr depth_dense_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  sensor_msgs::msg::CameraInfo::SharedPtr camera_info_;
  sensor_msgs::msg::PointCloud2::SharedPtr latest_lidar_;
  sensor_msgs::msg::Image::SharedPtr latest_image_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onLidar(sensor_msgs::msg::PointCloud2::SharedPtr msg);
  void onImage(sensor_msgs::msg::Image::SharedPtr msg);
  void onCameraInfo(sensor_msgs::msg::CameraInfo::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  // Projects LiDAR points onto the image plane → sparse depth map
  sensor_msgs::msg::Image::SharedPtr projectLidarToImage(
    const sensor_msgs::msg::PointCloud2 & cloud,
    const sensor_msgs::msg::CameraInfo & info);

  // Densifies sparse depth map via the configured method
  sensor_msgs::msg::Image::SharedPtr densify(const sensor_msgs::msg::Image & sparse_depth);

  void tryProcess();
};

}  // namespace traversability_generator
