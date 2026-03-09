#pragma once

#include <deque>

#include <geometry_msgs/msg/pose_stamped.hpp>
#include <message_filters/approximate_time_synchronizer.h>
#include <message_filters/subscriber.h>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace traversability_generator
{

class SensorSyncNode : public rclcpp::Node
{
public:
  explicit SensorSyncNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  double sync_slop_sec_;
  int sync_queue_size_;
  int odom_queue_size_;

  // ── Subscribers (sinks) — synchronized ───────────────────────────────────
  message_filters::Subscriber<sensor_msgs::msg::PointCloud2> lidar_sub_;
  message_filters::Subscriber<sensor_msgs::msg::Image> image_sub_;

  using ApproxSync = message_filters::ApproximateTimeSynchronizer<
    sensor_msgs::msg::PointCloud2,
    sensor_msgs::msg::Image>;
  std::shared_ptr<ApproxSync> sync_;

  // Odometry is consumed independently for pose interpolation
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr lidar_synced_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_synced_pub_;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr ego_pose_pub_;

  // ── Internal state ────────────────────────────────────────────────────────
  std::deque<nav_msgs::msg::Odometry> odom_queue_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onSynced(
    const sensor_msgs::msg::PointCloud2::ConstSharedPtr & lidar,
    const sensor_msgs::msg::Image::ConstSharedPtr & image);
  void onOdom(nav_msgs::msg::Odometry::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  geometry_msgs::msg::PoseStamped interpolatePose(const rclcpp::Time & target_time);
};

}  // namespace traversability_generator
