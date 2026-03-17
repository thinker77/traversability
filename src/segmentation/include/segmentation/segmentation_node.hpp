#pragma once

#include <string>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace traversability_generator
{

// ── Inference backend interface ───────────────────────────────────────────────
// Concrete implementations: OnnxRuntimeBackend, TensorRTBackend
class InferenceBackend
{
public:
  virtual ~InferenceBackend() = default;

  virtual void loadModel(const std::string & model_path) = 0;

  // input:  RGB image (H x W x 3, float32, normalized)
  // output: class-id map (H x W, uint8)
  virtual std::vector<uint8_t> infer(
    const std::vector<float> & input,
    int width, int height) = 0;
};

// ─────────────────────────────────────────────────────────────────────────────

class SemanticSegmentationNode : public rclcpp::Node
{
public:
  explicit SemanticSegmentationNode(const rclcpp::NodeOptions & options);

private:
  // ── Parameters ────────────────────────────────────────────────────────────
  std::string model_path_;
  int input_width_;
  int input_height_;
  float confidence_threshold_;
  std::vector<std::string> class_labels_;
  bool edge_refinement_enabled_;
  std::string backend_type_;

  // ── Inference backend (runtime-selected) ──────────────────────────────────
  std::unique_ptr<InferenceBackend> backend_;

  // ── Subscribers (sinks) ───────────────────────────────────────────────────
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;

  // ── Publishers (sources) ──────────────────────────────────────────────────
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr seg_mask_pub_;

  // ── Callbacks ─────────────────────────────────────────────────────────────
  void onImage(sensor_msgs::msg::Image::SharedPtr msg);

  // ── Processing ────────────────────────────────────────────────────────────
  std::vector<float> preprocess(const sensor_msgs::msg::Image & image);
  sensor_msgs::msg::Image::SharedPtr postprocess(
    const std::vector<uint8_t> & raw_mask,
    const std_msgs::msg::Header & header);
  void refineEdges(sensor_msgs::msg::Image & mask);

  // ── Factory ───────────────────────────────────────────────────────────────
  std::unique_ptr<InferenceBackend> createBackend(const std::string & type);
};

}  // namespace traversability_generator
