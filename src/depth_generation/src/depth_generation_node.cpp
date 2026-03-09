#include "depth_generation/depth_generation_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

DepthGenerationNode::DepthGenerationNode(const rclcpp::NodeOptions & options)
: Node("depth_generation_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  densification_method_ = declare_parameter("densification_method", std::string("ip_basic"));
  max_depth_m_          = declare_parameter("max_depth_m", 80.0);
  output_encoding_      = declare_parameter("output_encoding", std::string("32FC1"));

  // ── Publishers ────────────────────────────────────────────────────────────
  depth_dense_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "depth_dense", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  camera_info_sub_ = create_subscription<sensor_msgs::msg::CameraInfo>(
    "camera_info_proc", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::CameraInfo::SharedPtr msg) { camera_info_ = msg; });

  lidar_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    "lidar_synced", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::PointCloud2::SharedPtr msg) {
      latest_lidar_ = msg;
      tryProcess();
    });

  image_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "image_synced", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) {
      latest_image_ = msg;
      tryProcess();
    });

  RCLCPP_INFO(get_logger(), "DepthGenerationNode ready (method=%s)",
    densification_method_.c_str());
}

void DepthGenerationNode::tryProcess()
{
  if (!latest_lidar_ || !latest_image_ || !camera_info_) {
    return;
  }

  // Only process if lidar and image have matching timestamps
  const rclcpp::Time lidar_t(latest_lidar_->header.stamp);
  const rclcpp::Time image_t(latest_image_->header.stamp);
  if (std::abs((lidar_t - image_t).seconds()) > 0.05) {
    return;
  }

  auto sparse = projectLidarToImage(*latest_lidar_, *camera_info_);
  auto dense  = densify(*sparse);

  depth_dense_pub_->publish(*dense);

  latest_lidar_.reset();
  latest_image_.reset();
}

sensor_msgs::msg::Image::SharedPtr DepthGenerationNode::projectLidarToImage(
  const sensor_msgs::msg::PointCloud2 & cloud,
  const sensor_msgs::msg::CameraInfo & info)
{
  // TODO:
  // 1. For each point in cloud, transform to camera frame using extrinsics.
  // 2. Project onto image plane using camera intrinsic matrix K from info.
  // 3. Store depth (z in camera frame) at the pixel coordinate.
  // 4. Discard points behind camera (z <= 0) or beyond max_depth_m_.
  (void)cloud;
  (void)info;
  auto depth = std::make_shared<sensor_msgs::msg::Image>();
  depth->header = cloud.header;
  depth->encoding = output_encoding_;
  return depth;
}

sensor_msgs::msg::Image::SharedPtr DepthGenerationNode::densify(
  const sensor_msgs::msg::Image & sparse_depth)
{
  // TODO: implement densification:
  //   "ip_basic"  → IP-Basic algorithm (fast morphological fill)
  //   "nearest"   → nearest-neighbor interpolation
  //   "none"      → pass through
  auto out = std::make_shared<sensor_msgs::msg::Image>(sparse_depth);
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::DepthGenerationNode)
