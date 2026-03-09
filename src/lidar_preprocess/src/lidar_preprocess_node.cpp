#include "lidar_preprocess/lidar_preprocess_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

LidarPreprocessNode::LidarPreprocessNode(const rclcpp::NodeOptions & options)
: Node("lidar_preprocess_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  deskew_enabled_   = declare_parameter("deskew_enabled", true);
  imu_queue_size_   = declare_parameter("imu_queue_size", 200);
  min_range_m_      = declare_parameter("min_range_m", 0.5);
  max_range_m_      = declare_parameter("max_range_m", 80.0);
  voxel_leaf_size_m_= declare_parameter("voxel_leaf_size_m", 0.1);

  // ── Publishers ────────────────────────────────────────────────────────────
  points_proc_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
    "points_proc", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
    "imu", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Imu::SharedPtr msg) { onImu(msg); });

  points_raw_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    "points_raw", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::PointCloud2::SharedPtr msg) { onPointsRaw(msg); });

  RCLCPP_INFO(get_logger(), "LidarPreprocessNode ready");
}

void LidarPreprocessNode::onImu(sensor_msgs::msg::Imu::SharedPtr msg)
{
  imu_queue_.push_back(*msg);
  while (static_cast<int>(imu_queue_.size()) > imu_queue_size_) {
    imu_queue_.pop_front();
  }
}

void LidarPreprocessNode::onPointsRaw(sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
  auto cloud = msg;

  if (deskew_enabled_) {
    cloud = deskew(*cloud);
  }

  cloud = filterRange(*cloud);

  if (voxel_leaf_size_m_ > 0.0) {
    cloud = voxelDownsample(*cloud);
  }

  points_proc_pub_->publish(*cloud);
}

sensor_msgs::msg::PointCloud2::SharedPtr LidarPreprocessNode::deskew(
  const sensor_msgs::msg::PointCloud2 & cloud)
{
  // TODO: interpolate IMU angular velocity across the scan duration
  // to correct each point's position for vehicle motion during the sweep.
  auto out = std::make_shared<sensor_msgs::msg::PointCloud2>(cloud);
  return out;
}

sensor_msgs::msg::PointCloud2::SharedPtr LidarPreprocessNode::filterRange(
  const sensor_msgs::msg::PointCloud2 & cloud)
{
  // TODO: iterate over points, discard those with range < min_range_m_
  // or > max_range_m_.
  auto out = std::make_shared<sensor_msgs::msg::PointCloud2>(cloud);
  return out;
}

sensor_msgs::msg::PointCloud2::SharedPtr LidarPreprocessNode::voxelDownsample(
  const sensor_msgs::msg::PointCloud2 & cloud)
{
  // TODO: apply voxel grid filter with voxel_leaf_size_m_ leaf size.
  auto out = std::make_shared<sensor_msgs::msg::PointCloud2>(cloud);
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::LidarPreprocessNode)
