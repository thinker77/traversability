#pragma once

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace traversability_generator
{

class CameraPreprocessNode : public rclcpp::Node
{
public:
  explicit CameraPreprocessNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  bool undistort_enabled_;
  bool normalize_enabled_;
  std::vector<double> mean_;
  std::vector<double> std_;
  int output_width_;
  int output_height_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_raw_sub_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_proc_pub_;
  rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_proc_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  sensor_msgs::msg::CameraInfo::SharedPtr camera_info_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onImage(sensor_msgs::msg::Image::SharedPtr msg);
  void onCameraInfo(sensor_msgs::msg::CameraInfo::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  sensor_msgs::msg::Image::SharedPtr undistort(const sensor_msgs::msg::Image & image);
  sensor_msgs::msg::Image::SharedPtr normalize(const sensor_msgs::msg::Image & image);
  sensor_msgs::msg::Image::SharedPtr resize(const sensor_msgs::msg::Image & image);
};

}  // namespace traversability_generator
