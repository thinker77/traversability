#include "bev_fusion/bev_fusion_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

BevFusionNode::BevFusionNode(const rclcpp::NodeOptions & options)
: Node("bev_fusion_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  resolution_m_  = declare_parameter("resolution_m",  0.1);
  grid_width_m_  = declare_parameter("grid_width_m",  20.0);
  grid_height_m_ = declare_parameter("grid_height_m", 20.0);

  // ── Publishers ────────────────────────────────────────────────────────────
  feature_grid_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "feature_grid", rclcpp::QoS(10).reliable());

  // ── Subscribers ───────────────────────────────────────────────────────────
  elevation_map_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "elevation_map", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onElevationMap(msg); });

  slope_map_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "slope_map", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onSlopeMap(msg); });

  roughness_map_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "roughness_map", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onRoughnessMap(msg); });

  obstacle_map_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "obstacle_map", rclcpp::QoS(10).reliable(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onObstacleMap(msg); });

  semantic_points_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
    "semantic_points", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::PointCloud2::SharedPtr msg) { onSemanticPoints(msg); });

  RCLCPP_INFO(get_logger(), "BevFusionNode ready (%.1f x %.1f m @ %.2f m/cell)",
    grid_width_m_, grid_height_m_, resolution_m_);
}

void BevFusionNode::onElevationMap(sensor_msgs::msg::Image::SharedPtr msg)
{
  latest_elevation_ = msg;
  tryFuse();
}

void BevFusionNode::onSlopeMap(sensor_msgs::msg::Image::SharedPtr msg)
{
  latest_slope_ = msg;
  tryFuse();
}

void BevFusionNode::onRoughnessMap(sensor_msgs::msg::Image::SharedPtr msg)
{
  latest_roughness_ = msg;
  tryFuse();
}

void BevFusionNode::onObstacleMap(sensor_msgs::msg::Image::SharedPtr msg)
{
  latest_obstacle_ = msg;
  tryFuse();
}

void BevFusionNode::onSemanticPoints(sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
  latest_semantic_points_ = msg;
  tryFuse();
}

void BevFusionNode::tryFuse()
{
  if (!latest_elevation_ || !latest_slope_ ||
      !latest_roughness_ || !latest_obstacle_ || !latest_semantic_points_) {
    return;
  }

  auto grid = fuse();
  feature_grid_pub_->publish(*grid);

  latest_elevation_.reset();
  latest_slope_.reset();
  latest_roughness_.reset();
  latest_obstacle_.reset();
  latest_semantic_points_.reset();
}

sensor_msgs::msg::Image::SharedPtr BevFusionNode::fuse() const
{
  // TODO: project terrain maps and semantic_points into a unified BEV grid
  // with resolution_m_ cell size, grid_width_m_ x grid_height_m_ extent.
  // Each cell stores a multi-channel feature vector:
  //   [elevation, slope, roughness, obstacle_score, semantic_class, confidence].
  auto out = std::make_shared<sensor_msgs::msg::Image>();
  out->header = latest_elevation_->header;
  return out;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::BevFusionNode)
