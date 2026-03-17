# ─────────────────────────────────────────────────────────────────────────────
# Stage 1 — base
#   All ROS 2 deps + colcon build tools.
#   Used directly as the dev container (source is mounted at runtime).
# ─────────────────────────────────────────────────────────────────────────────
FROM ros:jazzy-ros-base AS base

RUN apt-get update && apt-get install -y --no-install-recommends \
        python3-colcon-common-extensions \
        ros-jazzy-rclcpp \
        ros-jazzy-rclcpp-components \
        ros-jazzy-sensor-msgs \
        ros-jazzy-nav-msgs \
        ros-jazzy-tf2-msgs \
        ros-jazzy-rmw-cyclonedds-cpp \
        ros-jazzy-ros2launch \
        ros-jazzy-launch \
        ros-jazzy-launch-ros \
        python3-yaml \
 && rm -rf /var/lib/apt/lists/*

# Pre-source ROS for every shell session inside the container
RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc

WORKDIR /workspace


# ─────────────────────────────────────────────────────────────────────────────
# Stage 2 — builder
#   Copies the source tree and produces a colcon install/ tree.
#   Used as an intermediate stage for the final runtime image.
# ─────────────────────────────────────────────────────────────────────────────
FROM base AS builder

COPY . .

RUN python3 scripts/generate_launch.py

RUN . /opt/ros/jazzy/setup.sh \
 && colcon build \
        --packages-select \
            camera_preprocess segmentation \
            lidar_aggregator lidar_preprocess terrain_model lidar_projection \
            semantic_lifting bev_fusion temporal_grid traversability_costmap \
            traversability_generator \
        --cmake-args -DCMAKE_BUILD_TYPE=Release \
        --base-paths . src \
        --build-base /tmp/colcon_build \
        --install-base /workspace/install


# ─────────────────────────────────────────────────────────────────────────────
# Stage 3 — runtime
#   Minimal image containing only runtime libs + the built install tree.
# ─────────────────────────────────────────────────────────────────────────────
FROM ros:jazzy-ros-base AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        ros-jazzy-rclcpp \
        ros-jazzy-rclcpp-components \
        ros-jazzy-sensor-msgs \
        ros-jazzy-nav-msgs \
        ros-jazzy-tf2-msgs \
        ros-jazzy-rmw-cyclonedds-cpp \
        ros-jazzy-ros2launch \
        ros-jazzy-launch \
        ros-jazzy-launch-ros \
        python3-yaml \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY --from=builder /workspace/install ./install
COPY --from=builder /workspace/config  ./config
COPY --from=builder /workspace/launch  ./launch

ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ENV CYCLONEDDS_URI=/workspace/config/cyclonedds.xml

COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
