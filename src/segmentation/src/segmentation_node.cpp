#include "segmentation/segmentation_node.hpp"

#include <stdexcept>

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

SegmentationNode::SegmentationNode(const rclcpp::NodeOptions & options)
: Node("segmentation_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  model_path_                  = declare_parameter("model_path", std::string("models/seg_model.onnx"));
  input_width_                 = declare_parameter("input_width", 640);
  input_height_                = declare_parameter("input_height", 384);
  confidence_threshold_        = declare_parameter("confidence_threshold", 0.5f);
  class_labels_                = declare_parameter("class_labels",
    std::vector<std::string>{"road", "grass", "vegetation", "obstacle", "vehicle", "unknown"});
  edge_refinement_enabled_     = declare_parameter("edge_refinement_enabled", true);
  occlusion_handling_enabled_  = declare_parameter("occlusion_handling_enabled", true);
  backend_type_                = declare_parameter("backend", std::string("onnxruntime"));

  // ── Inference backend ─────────────────────────────────────────────────────
  backend_ = createBackend(backend_type_);
  backend_->loadModel(model_path_);

  // ── Publishers ────────────────────────────────────────────────────────────
  seg_mask_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "seg_mask", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  depth_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "depth_dense", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onDepth(msg); });

  image_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "image_synced", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onImage(msg); });

  RCLCPP_INFO(get_logger(), "SegmentationNode ready (backend=%s, model=%s)",
    backend_type_.c_str(), model_path_.c_str());
}

void SegmentationNode::onDepth(sensor_msgs::msg::Image::SharedPtr msg)
{
  latest_depth_ = msg;
}

void SegmentationNode::onImage(sensor_msgs::msg::Image::SharedPtr msg)
{
  auto input_tensor = preprocess(*msg);
  auto raw_mask = backend_->infer(input_tensor, input_width_, input_height_);
  auto mask = postprocess(raw_mask, msg->header);

  if (edge_refinement_enabled_ && latest_depth_) {
    refineEdges(*mask, *latest_depth_);
  }

  seg_mask_pub_->publish(*mask);
}

std::vector<float> SegmentationNode::preprocess(const sensor_msgs::msg::Image & image)
{
  // TODO:
  // 1. Decode image bytes to float32 H×W×3.
  // 2. Resize to input_width_ × input_height_.
  // 3. Normalize: pixel = (pixel/255 - mean) / std  (already done in camera_preprocess
  //    if normalize_enabled=true — skip here if so).
  // 4. Transpose HWC → CHW, flatten to vector<float>.
  (void)image;
  return std::vector<float>(3 * input_width_ * input_height_, 0.0f);
}

sensor_msgs::msg::Image::SharedPtr SegmentationNode::postprocess(
  const std::vector<uint8_t> & raw_mask,
  const std_msgs::msg::Header & header)
{
  // TODO:
  // 1. Resize raw_mask from model output resolution back to original image size.
  // 2. Pack into sensor_msgs::msg::Image with encoding "mono8".
  auto mask = std::make_shared<sensor_msgs::msg::Image>();
  mask->header = header;
  mask->encoding = "mono8";
  mask->data = raw_mask;
  return mask;
}

void SegmentationNode::refineEdges(
  sensor_msgs::msg::Image & mask,
  const sensor_msgs::msg::Image & depth)
{
  // TODO:
  // Use depth discontinuities to sharpen class boundaries at object edges.
  // Depth edges (large gradient in depth image) → snap seg boundary to depth edge.
  (void)mask;
  (void)depth;
}

std::unique_ptr<InferenceBackend> SegmentationNode::createBackend(const std::string & type)
{
  // TODO: return OnnxRuntimeBackend or TensorRTBackend based on type.
  // Both implement the InferenceBackend interface.
  // Selected at runtime from YAML — no recompilation needed between T4 and Thor.
  (void)type;
  throw std::runtime_error("InferenceBackend not yet implemented. "
    "Implement OnnxRuntimeBackend and TensorRTBackend.");
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::SegmentationNode)
