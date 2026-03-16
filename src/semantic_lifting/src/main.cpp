#include "rclcpp/rclcpp.hpp"
#include "semantic_lifting/semantic_lifting_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<traversability_generator::SemanticLiftingNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
