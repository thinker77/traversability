#include "sensor_sync/sensor_sync_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

SensorSyncNode::SensorSyncNode(const rclcpp::NodeOptions & options)
: Node("sensor_sync_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  sync_slop_sec_   = declare_parameter("sync_slop_sec", 0.05);
  sync_queue_size_ = declare_parameter("sync_queue_size", 10);
  odom_queue_size_ = declare_parameter("odom_queue_size", 200);

  // ── Publishers ────────────────────────────────────────────────────────────
  lidar_synced_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
    "lidar_synced", rclcpp::SensorDataQoS());
  image_synced_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "image_synced", rclcpp::SensorDataQoS());
  ego_pose_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>(
    "ego_pose_synced", rclcpp::SensorDataQoS());

  // ── Approximate time sync ─────────────────────────────────────────────────
  lidar_sub_.subscribe(this, "points_proc");
  image_sub_.subscribe(this, "image_proc");

  sync_ = std::make_shared<ApproxSync>(
    ApproxSync(sync_queue_size_, rclcpp::Duration::from_seconds(sync_slop_sec_)),
    lidar_sub_, image_sub_);
  sync_->registerCallback(&SensorSyncNode::onSynced, this);

  // ── Odometry ──────────────────────────────────────────────────────────────
  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "odom", rclcpp::SensorDataQoS(),
    [this](nav_msgs::msg::Odometry::SharedPtr msg) { onOdom(msg); });

  RCLCPP_INFO(get_logger(), "SensorSyncNode ready (slop=%.3fs)", sync_slop_sec_);
}

void SensorSyncNode::onOdom(nav_msgs::msg::Odometry::SharedPtr msg)
{
  odom_queue_.push_back(*msg);
  while (static_cast<int>(odom_queue_.size()) > odom_queue_size_) {
    odom_queue_.pop_front();
  }
}

void SensorSyncNode::onSynced(
  const sensor_msgs::msg::PointCloud2::ConstSharedPtr & lidar,
  const sensor_msgs::msg::Image::ConstSharedPtr & image)
{
  lidar_synced_pub_->publish(*lidar);
  image_synced_pub_->publish(*image);

  const rclcpp::Time target_time(lidar->header.stamp);
  auto pose = interpolatePose(target_time);
  ego_pose_pub_->publish(pose);
}

geometry_msgs::msg::PoseStamped SensorSyncNode::interpolatePose(
  const rclcpp::Time & target_time)
{
  geometry_msgs::msg::PoseStamped pose;
  pose.header.stamp = target_time;
  pose.header.frame_id = "odom";

  // TODO: binary search odom_queue_ for the two entries bracketing
  // target_time, then linearly interpolate position and slerp orientation.
  if (!odom_queue_.empty()) {
    pose.pose = odom_queue_.back().pose.pose;
  }

  return pose;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::SensorSyncNode)
