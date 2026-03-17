#include "segmentation/segmentation_node.hpp"

#include <stdexcept>

#include <rclcpp_components/register_node_macro.hpp>

namespace traversability_generator
{

SemanticSegmentationNode::SemanticSegmentationNode(const rclcpp::NodeOptions & options)
: Node("semantic_segmentation_node", options)
{
  // ── Parameters ────────────────────────────────────────────────────────────
  model_path_              = declare_parameter("model_path", std::string("models/seg_model.onnx"));
  input_width_             = declare_parameter("input_width", 640);
  input_height_            = declare_parameter("input_height", 384);
  confidence_threshold_    = declare_parameter("confidence_threshold", 0.5f);
  class_labels_            = declare_parameter("class_labels",
    std::vector<std::string>{"road", "grass", "vegetation", "obstacle", "vehicle", "unknown"});
  edge_refinement_enabled_ = declare_parameter("edge_refinement_enabled", true);
  backend_type_            = declare_parameter("backend", std::string("onnxruntime"));

  // ── Inference backend ─────────────────────────────────────────────────────
  backend_ = createBackend(backend_type_);
  backend_->loadModel(model_path_);

  // ── Publishers ────────────────────────────────────────────────────────────
  seg_mask_pub_ = create_publisher<sensor_msgs::msg::Image>(
    "seg_mask", rclcpp::SensorDataQoS());

  // ── Subscribers ───────────────────────────────────────────────────────────
  image_sub_ = create_subscription<sensor_msgs::msg::Image>(
    "image_rect", rclcpp::SensorDataQoS(),
    [this](sensor_msgs::msg::Image::SharedPtr msg) { onImage(msg); });

  RCLCPP_INFO(get_logger(), "SemanticSegmentationNode ready (backend=%s, model=%s)",
    backend_type_.c_str(), model_path_.c_str());
}

void SemanticSegmentationNode::onImage(sensor_msgs::msg::Image::SharedPtr msg)
{
  auto input_tensor = preprocess(*msg);
  auto raw_mask = backend_->infer(input_tensor, input_width_, input_height_);
  auto mask = postprocess(raw_mask, msg->header);

  if (edge_refinement_enabled_) {
    refineEdges(*mask);
  }

  seg_mask_pub_->publish(*mask);
}

std::vector<float> SemanticSegmentationNode::preprocess(const sensor_msgs::msg::Image & image)
{
  // TODO:
  // 1. Decode image bytes to float32 H×W×3.
  // 2. Resize to input_width_ × input_height_.
  // 3. Normalize: pixel = (pixel/255 - mean) / std  (already done in camera_branch
  //    if normalize_enabled=true — skip here if so).
  // 4. Transpose HWC → CHW, flatten to vector<float>.
  (void)image;
  return std::vector<float>(3 * input_width_ * input_height_, 0.0f);
}

sensor_msgs::msg::Image::SharedPtr SemanticSegmentationNode::postprocess(
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

void SemanticSegmentationNode::refineEdges(sensor_msgs::msg::Image & mask)
{
  // TODO: apply edge refinement to sharpen class boundaries.
  (void)mask;
}

std::unique_ptr<InferenceBackend> SemanticSegmentationNode::createBackend(
  const std::string & type)
{
  // TODO: return OnnxRuntimeBackend or TensorRTBackend based on type.
  // Both implement the InferenceBackend interface.
  // Selected at runtime from YAML — no recompilation needed between T4 and Thor.
  (void)type;
  throw std::runtime_error("InferenceBackend not yet implemented. "
    "Implement OnnxRuntimeBackend and TensorRTBackend.");
}

}  // namespace traversability_generator

RCLCPP_COMPONENTS_REGISTER_NODE(traversability_generator::SemanticSegmentationNode)
