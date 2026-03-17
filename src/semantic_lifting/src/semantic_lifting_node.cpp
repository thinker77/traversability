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
  tf_sub_ = create_subscription<tf2_msgs::msg::TFMessage>(
    "tf", rclcpp::QoS(10).reliable(),
    [this](tf2_msgs::msg::TFMessage::SharedPtr msg) { onTf(msg); });

  seg_mask_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "seg_mask", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onSegMask(msg); });

  projected_depth_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "projected_depth", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onProjectedDepth(msg); });

  RCLCPP_INFO(get_logger(), "SemanticLiftingNode ready");
}

void SemanticLiftingNode::onTf(tf2_msgs::msg::TFMessage::SharedPtr msg)
{
  // TODO: feed transforms into tf2 buffer for frame alignment during lifting.
  (void)msg;
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
  if (!latest_seg_mask_ || !latest_depth_) {
    return;
  }

  const rclcpp::Time mask_t(latest_seg_mask_->header.stamp);
  const rclcpp::Time depth_t(latest_depth_->header.stamp);
  if (std::abs((mask_t - depth_t).seconds()) > 0.05) {
    return;
  }

  auto points = liftToPoints(*latest_seg_mask_, *latest_depth_);
  semantic_points_pub_->publish(*points);

  latest_seg_mask_.reset();
  latest_depth_.reset();
}

sensor_msgs::msg::PointCloud2::SharedPtr SemanticLiftingNode::liftToPoints(
  const sensor_msgs::msg::Image & seg_mask,
  const sensor_msgs::msg::Image & depth) const
{
  // TODO: for each pixel in seg_mask, read the depth value from the depth image,
  // back-project using the camera intrinsics (obtained via tf2 or a fixed
  // calibration param) to obtain a 3-D point, and attach the semantic class
  // label. Return as PointCloud2 with fields x, y, z, label.
  (void)seg_mask;
  (void)depth;
  auto out = std::make_shared<sensor_msgs::msg::PointCloud2>();
  out->header = seg_mask.header;
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::SemanticLiftingNode)
