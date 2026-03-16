#!/bin/bash
# entrypoint.sh — sources ROS 2 Jazzy and launches the traversability pipeline

set -e

# Source ROS 2 Jazzy base and the colcon install overlay
source /opt/ros/jazzy/setup.bash
source /workspace/install/setup.bash

echo "────────────────────────────────────────────────────────"
echo "  traversability_generator  |  ROS 2 Jazzy"
echo "  RMW: ${RMW_IMPLEMENTATION}"
echo "────────────────────────────────────────────────────────"

# If arguments are passed, run them directly (e.g. bash, ros2 topic list)
if [ "$#" -gt 0 ]; then
    exec "$@"
fi

# Default: launch the full pipeline
exec ros2 launch /workspace/launch/perception.launch.py
