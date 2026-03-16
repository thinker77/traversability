#pragma once

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class SemanticLiftingNode : public rclcpp::Node
{
public:
  explicit SemanticLiftingNode(const rclcpp::NodeOptions & options);

private:
  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr seg_mask_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr projected_depth_sub_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr semantic_points_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  sensor_msgs::msg::CameraInfo::SharedPtr camera_info_;
  sensor_msgs::msg::Image::SharedPtr latest_seg_mask_;
  sensor_msgs::msg::Image::SharedPtr latest_depth_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onSegMask(sensor_msgs::msg::Image::SharedPtr msg);
  void onProjectedDepth(sensor_msgs::msg::Image::SharedPtr msg);
  void onCameraInfo(sensor_msgs::msg::CameraInfo::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  void tryLift();
  sensor_msgs::msg::PointCloud2::SharedPtr liftToPoints(
    const sensor_msgs::msg::Image & seg_mask,
    const sensor_msgs::msg::Image & depth,
    const sensor_msgs::msg::CameraInfo & camera_info) const;
};

}  // namespace traversability_generator
