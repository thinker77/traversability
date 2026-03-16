#include "lidar_projection/lidar_projection_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

LidarProjectionNode::LidarProjectionNode(const rclcpp::NodeOptions & options)
: Node("lidar_projection_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  image_height_ = declare_parameter("image_height", 480);
  image_width_  = declare_parameter("image_width",  640);
  max_depth_m_  = declare_parameter("max_depth_m",  80.0);

  // ── Publishers ────────────────────────────────────────────────────────────
  projected_depth_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "projected_depth", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  points_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    "points_ground_removed", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::PointCloud2::SharedPtr msg) { onPoints(msg); });

  RCLCPP_INFO(get_logger(), "LidarProjectionNode ready (%dx%d)", image_width_, image_height_);
}

void LidarProjectionNode::onPoints(sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
  auto depth = project(*msg);
  projected_depth_pub_->publish(*depth);
}

sensor_msgs::msg::Image::SharedPtr LidarProjectionNode::project(
  const sensor_msgs::msg::PointCloud2 & cloud) const
{
  // TODO: project each 3-D point onto the image plane using the camera
  // intrinsics loaded from the camera_info topic and store the z-distance
  // (range) at the corresponding pixel. Discard points with z > max_depth_m_
  // or behind the camera (z <= 0).
  (void)cloud;
  auto depth = std::make_shared<sensor_msgs::msg::Image>();
  depth->header   = cloud.header;
  depth->height   = static_cast<uint32_t>(image_height_);
  depth->width    = static_cast<uint32_t>(image_width_);
  depth->encoding = "32FC1";
  depth->step     = depth->width * 4;
  depth->data.assign(static_cast<size_t>(image_height_) * image_width_ * 4, 0);
  return depth;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::LidarProjectionNode)
