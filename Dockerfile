# ─────────────────────────────────────────────────────────────────────────────
# Stage 1 — builder
#   Installs colcon + all build-time deps, compiles every node.
# ─────────────────────────────────────────────────────────────────────────────
FROM ros:jazzy-ros-base AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        python3-colcon-common-extensions \
        ros-jazzy-rclcpp \
        ros-jazzy-rclcpp-components \
        ros-jazzy-sensor-msgs \
        ros-jazzy-nav-msgs \
        ros-jazzy-tf2-msgs \
        ros-jazzy-grid-map-msgs \
        ros-jazzy-rmw-cyclonedds-cpp \
        python3-yaml \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

RUN python3 scripts/generate_launch.py

RUN . /opt/ros/jazzy/setup.sh \
 && colcon build \
        --packages-select \
            camera_preprocess lidar_aggregator lidar_preprocess \
            lidar_ground_filter elevation_mapping segmentation \
            lidar_projection semantic_lifting bev_fusion \
            temporal_grid traversability_costmap \
            traversability_generator \
        --cmake-args -DCMAKE_BUILD_TYPE=Release \
        --base-paths . src \
        --build-base /workspace/build/colcon \
        --install-base /workspace/install


# ─────────────────────────────────────────────────────────────────────────────
# Stage 2 — runtime
#   Minimal image with only runtime libraries + built artifacts.
# ─────────────────────────────────────────────────────────────────────────────
FROM ros:jazzy-ros-base AS runtime

# Runtime ROS 2 packages + CycloneDDS + launch system
RUN apt-get update && apt-get install -y --no-install-recommends \
        ros-jazzy-rclcpp \
        ros-jazzy-rclcpp-components \
        ros-jazzy-sensor-msgs \
        ros-jazzy-nav-msgs \
        ros-jazzy-tf2-msgs \
        ros-jazzy-grid-map-msgs \
        ros-jazzy-rmw-cyclonedds-cpp \
        ros-jazzy-ros2launch \
        ros-jazzy-launch \
        ros-jazzy-launch-ros \
        python3-yaml \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Copy colcon install tree from the builder stage
COPY --from=builder /workspace/install ./install

# Copy config, launch, and param files
COPY --from=builder /workspace/config  ./config
COPY --from=builder /workspace/launch  ./launch

# CycloneDDS RMW and config
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ENV CYCLONEDDS_URI=/workspace/config/cyclonedds.xml

COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
