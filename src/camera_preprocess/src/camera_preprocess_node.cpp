#include "camera_preprocess/camera_preprocess_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

CameraPreprocessNode::CameraPreprocessNode(const rclcpp::NodeOptions & options)
: Node("camera_preprocess_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  undistort_enabled_ = declare_parameter("undistort_enabled", true);
  normalize_enabled_ = declare_parameter("normalize_enabled", true);
  mean_              = declare_parameter("mean", std::vector<double>{0.485, 0.456, 0.406});
  std_               = declare_parameter("std",  std::vector<double>{0.229, 0.224, 0.225});
  output_width_      = declare_parameter("output_width", 0);
  output_height_     = declare_parameter("output_height", 0);

  // ── Publishers ────────────────────────────────────────────────────────────
  image_proc_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "image_proc", rclcpp::SensorDataQoS());
  camera_info_proc_pub_ = create_publisher<sensor_msgs::msg::CameraInfo>(
    "camera_info_proc", rclcpp::QoS(10).reliable());

  // ── Subscribers ───────────────────────────────────────────────────────────
  camera_info_sub_ = create_subscription<sensor_msgs::msg::CameraInfo>(
    "camera_info", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::CameraInfo::SharedPtr msg) { onCameraInfo(msg); });

  image_raw_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "image_raw", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onImage(msg); });

  RCLCPP_INFO(get_logger(), "CameraPreprocessNode ready");
}

void CameraPreprocessNode::onCameraInfo(sensor_msgs::msg::CameraInfo::SharedPtr msg)
{
  camera_info_ = msg;
  camera_info_proc_pub_->publish(*msg);
}

void CameraPreprocessNode::onImage(sensor_msgs::msg::Image::SharedPtr msg)
{
  if (!camera_info_) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000,
      "Waiting for camera_info before processing images");
    return;
  }

  auto image = msg;

  if (undistort_enabled_) {
    image = undistort(*image);
  }

  if (output_width_ > 0 && output_height_ > 0) {
    image = resize(*image);
  }

  if (normalize_enabled_) {
    image = normalize(*image);
  }

  image_proc_pub_->publish(*image);
}

sensor_msgs::msg::Image::SharedPtr CameraPreprocessNode::undistort(
  const sensor_msgs::msg::Image & image)
{
  // TODO: apply cv::undistort using camera_info_ D and K matrices.
  auto out = std::make_shared<sensor_msgs::msg::Image>(image);
  return out;
}

sensor_msgs::msg::Image::SharedPtr CameraPreprocessNode::resize(
  const sensor_msgs::msg::Image & image)
{
  // TODO: apply cv::resize to output_width_ x output_height_.
  auto out = std::make_shared<sensor_msgs::msg::Image>(image);
  return out;
}

sensor_msgs::msg::Image::SharedPtr CameraPreprocessNode::normalize(
  const sensor_msgs::msg::Image & image)
{
  // TODO: convert to float32, subtract mean_, divide by std_ per channel.
  auto out = std::make_shared<sensor_msgs::msg::Image>(image);
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::CameraPreprocessNode)
