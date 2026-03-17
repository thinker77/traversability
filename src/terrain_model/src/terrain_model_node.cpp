#include "terrain_model/terrain_model_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

TerrainModelNode::TerrainModelNode(const rclcpp::NodeOptions & options)
: Node("terrain_model_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  resolution_m_                 = declare_parameter("resolution_m",                  0.1);
  map_length_x_m_               = declare_parameter("map_length_x_m",               20.0);
  map_length_y_m_               = declare_parameter("map_length_y_m",               20.0);
  sensor_cutoff_min_depth_m_    = declare_parameter("sensor_cutoff_min_depth_m",     0.2);
  sensor_cutoff_max_depth_m_    = declare_parameter("sensor_cutoff_max_depth_m",    80.0);
  time_tolerance_sec_           = declare_parameter("time_tolerance_sec",            0.2);
  tall_object_height_threshold_m_ = declare_parameter("tall_object_height_threshold_m", 0.5);

  // ── Publishers ────────────────────────────────────────────────────────────
  elevation_map_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "elevation_map", rclcpp::QoS(10).reliable());
  slope_map_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "slope_map", rclcpp::QoS(10).reliable());
  roughness_map_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "roughness_map", rclcpp::QoS(10).reliable());
  obstacle_map_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "obstacle_map", rclcpp::QoS(10).reliable());
  points_no_tall_objects_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
    "points_no_tall_objects", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "odometry", rclcpp::SensorDataQoS(),
    [this](nav_msgs::msg::Odometry::SharedPtr msg) { onOdom(msg); });

  tf_sub_ = create_subscription<tf2_msgs::msg::TFMessage>(
    "tf", rclcpp::QoS(10).reliable(),
    [this](tf2_msgs::msg::TFMessage::SharedPtr msg) { onTf(msg); });

  points_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    "points_filtered", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::PointCloud2::SharedPtr msg) { onPoints(msg); });

  RCLCPP_INFO(get_logger(), "TerrainModelNode ready (res=%.2fm, map=%.0fx%.0fm)",
    resolution_m_, map_length_x_m_, map_length_y_m_);
}

void TerrainModelNode::onOdom(nav_msgs::msg::Odometry::SharedPtr msg)
{
  latest_odom_ = msg;
}

void TerrainModelNode::onTf(tf2_msgs::msg::TFMessage::SharedPtr msg)
{
  // TODO: feed transforms into tf2 buffer for point cloud frame alignment.
  (void)msg;
}

void TerrainModelNode::onPoints(sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
  if (!latest_odom_) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000,
      "Waiting for odometry before processing point cloud");
    return;
  }

  auto filtered = removeTallObjects(*msg);
  points_no_tall_objects_pub_->publish(*filtered);

  buildTerrainMaps(*msg, msg->header);
}

sensor_msgs::msg::PointCloud2::SharedPtr TerrainModelNode::removeTallObjects(
  const sensor_msgs::msg::PointCloud2 & cloud) const
{
  // TODO: remove points above tall_object_height_threshold_m_ relative to the
  // estimated ground plane so that lidar_projection_node sees a ground-level
  // cloud without vegetation, vehicles, or other tall obstacles.
  auto out = std::make_shared<sensor_msgs::msg::PointCloud2>(cloud);
  return out;
}

void TerrainModelNode::buildTerrainMaps(
  const sensor_msgs::msg::PointCloud2 & cloud,
  const std_msgs::msg::Header & header)
{
  // TODO: project points onto a 2-D grid of resolution_m_ and compute per-cell:
  //   elevation_map  – mean z height
  //   slope_map      – maximum gradient between adjacent cells
  //   roughness_map  – z variance within each cell
  //   obstacle_map   – binary: 1 where height > tall_object_height_threshold_m_
  // Publish all four as sensor_msgs/Image (32FC1 encoding, same grid dimensions).
  (void)cloud;

  auto make_grid = [&]() {
    auto img = std::make_shared<sensor_msgs::msg::Image>();
    img->header   = header;
    img->height   = static_cast<uint32_t>(map_length_y_m_ / resolution_m_);
    img->width    = static_cast<uint32_t>(map_length_x_m_ / resolution_m_);
    img->encoding = "32FC1";
    img->step     = img->width * 4;
    img->data.assign(static_cast<size_t>(img->height) * img->width * 4, 0);
    return img;
  };

  elevation_map_pub_->publish(*make_grid());
  slope_map_pub_->publish(*make_grid());
  roughness_map_pub_->publish(*make_grid());
  obstacle_map_pub_->publish(*make_grid());
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::TerrainModelNode)
