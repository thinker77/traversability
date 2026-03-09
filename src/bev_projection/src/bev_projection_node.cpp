#include "bev_projection/bev_projection_node.hpp"

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

BevProjectionNode::BevProjectionNode(const rclcpp::NodeOptions & options)
: Node("bev_projection_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  resolution_m_       = declare_parameter("resolution_m", 0.1);
  grid_width_m_       = declare_parameter("grid_width_m", 20.0);
  grid_height_m_      = declare_parameter("grid_height_m", 20.0);
  projection_surface_ = declare_parameter("projection_surface", std::string("elevation"));
  aggregation_method_ = declare_parameter("aggregation_method", std::string("majority_vote"));
  unknown_class_id_   = declare_parameter("unknown_class_id", 5);

  // ── Publishers ────────────────────────────────────────────────────────────
  bev_grid_pub_ = create_publisher<nav_msgs::msg::OccupancyGrid>(
    "bev_semantic_grid", rclcpp::QoS(10).reliable());

  // ── Subscribers ───────────────────────────────────────────────────────────
  depth_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "depth_dense", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) {
      latest_depth_ = msg;
      tryProcess();
    });

  elevation_sub_ = create_subscription<grid_map_msgs::msg::GridMap>(
    "elevation_map", rclcpp::QoS(10).reliable(),
    [this](grid_map_msgs::msg::GridMap::SharedPtr msg) {
      latest_elevation_ = msg;
    });

  seg_mask_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "seg_mask", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) {
      // seg_mask drives processing
      if (latest_depth_) {
        auto bev = project(*msg, *latest_depth_);
        bev_grid_pub_->publish(*bev);
      }
    });

  RCLCPP_INFO(get_logger(), "BevProjectionNode ready (surface=%s, res=%.2fm)",
    projection_surface_.c_str(), resolution_m_);
}

void BevProjectionNode::onDepth(sensor_msgs::msg::Image::SharedPtr msg)
{
  latest_depth_ = msg;
}

void BevProjectionNode::onElevationMap(grid_map_msgs::msg::GridMap::SharedPtr msg)
{
  latest_elevation_ = msg;
}

void BevProjectionNode::onSegMask(sensor_msgs::msg::Image::SharedPtr msg)
{
  (void)msg;
}

void BevProjectionNode::tryProcess() {}

nav_msgs::msg::OccupancyGrid::SharedPtr BevProjectionNode::project(
  const sensor_msgs::msg::Image & seg_mask,
  const sensor_msgs::msg::Image & depth)
{
  // TODO:
  // For each pixel (u, v) in seg_mask:
  //   1. Read depth d = depth[v, u].
  //   2. Unproject: X = (u - cx) * d / fx,  Y = (v - cy) * d / fy,  Z = d
  //      (in camera frame, using intrinsics from camera_info).
  //   3. Transform (X, Y, Z) to vehicle/world frame.
  //   4. If projection_surface_ == "elevation": snap Z to elevation map height.
  //   5. Compute BEV grid cell (col, row) from (X, Y) and resolution_m_.
  //   6. Accumulate class vote for that cell.
  // After all pixels: resolve each cell via aggregation_method_.
  (void)seg_mask;
  (void)depth;

  auto grid = std::make_shared<nav_msgs::msg::OccupancyGrid>();
  grid->header.stamp = seg_mask.header.stamp;
  grid->header.frame_id = "base_link";
  grid->info.resolution = static_cast<float>(resolution_m_);
  grid->info.width  = static_cast<uint32_t>(grid_width_m_  / resolution_m_);
  grid->info.height = static_cast<uint32_t>(grid_height_m_ / resolution_m_);
  grid->data.assign(grid->info.width * grid->info.height, unknown_class_id_);
  return grid;
}

float BevProjectionNode::getProjectionHeight(float bev_x, float bev_y)
{
  // TODO: look up (bev_x, bev_y) in latest_elevation_ GridMap elevation layer.
  (void)bev_x;
  (void)bev_y;
  return 0.0f;
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::BevProjectionNode)
