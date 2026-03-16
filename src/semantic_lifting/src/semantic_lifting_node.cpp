#include "semantic_lifting/semantic_lifting_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

SemanticLiftingNode::SemanticLiftingNode(const rclcpp::NodeOptions & options)
: Node("semantic_lifting_node", options)
{
  // ── Publishers ────────────────────────────────────────────────────────────
  semantic_points_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
    "semantic_points", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  camera_info_sub_ = create_subscription<sensor_msgs::msg::CameraInfo>(
    "camera_info_rect", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::CameraInfo::SharedPtr msg) { onCameraInfo(msg); });

  seg_mask_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "seg_mask", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onSegMask(msg); });

  projected_depth_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "projected_depth", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onProjectedDepth(msg); });

  RCLCPP_INFO(get_logger(), "SemanticLiftingNode ready");
}

void SemanticLiftingNode::onCameraInfo(sensor_msgs::msg::CameraInfo::SharedPtr msg)
{
  camera_info_ = msg;
}

void SemanticLiftingNode::onSegMask(sensor_msgs::msg::Image::SharedPtr msg)
{
  latest_seg_mask_ = msg;
  tryLift();
}

void SemanticLiftingNode::onProjectedDepth(sensor_msgs::msg::Image::SharedPtr msg)
{
  latest_depth_ = msg;
  tryLift();
}

void SemanticLiftingNode::tryLift()
{
  if (!latest_seg_mask_ || !latest_depth_ || !camera_info_) {
    return;
  }

  const rclcpp::Time mask_t(latest_seg_mask_->header.stamp);
  const rclcpp::Time depth_t(latest_depth_->header.stamp);
  if (std::abs((mask_t - depth_t).seconds()) > 0.05) {
    return;
  }

  auto points = liftToPoints(*latest_seg_mask_, *latest_depth_, *camera_info_);
  semantic_points_pub_->publish(*points);

  latest_seg_mask_.reset();
  latest_depth_.reset();
}

sensor_msgs::msg::PointCloud2::SharedPtr SemanticLiftingNode::liftToPoints(
  const sensor_msgs::msg::Image & seg_mask,
  const sensor_msgs::msg::Image & depth,
  const sensor_msgs::msg::CameraInfo & camera_info) const
{
  // TODO: for each pixel in seg_mask, read the depth value from depth image,
  // back-project using camera_info intrinsics (K matrix) to obtain a 3-D point,
  // and attach the semantic class label. Return as PointCloud2 with fields
  // x, y, z, label.
  (void)seg_mask;
  (void)depth;
  (void)camera_info;
  auto out = std::make_shared<sensor_msgs::msg::PointCloud2>();
  out->header = seg_mask.header;
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::SemanticLiftingNode)
