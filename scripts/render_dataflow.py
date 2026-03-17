#!/usr/bin/env python3
"""
render_dataflow.py

Reads config/dataflow.yaml (process-based schema) and saves two PNG diagrams:

  build/dataflow_processes.png  – process-level overview
                                  (camera_preprocessor / lidar_preprocessor / terrain_modeler,
                                   DDS arrows labelled with topics)

  build/dataflow_nodes.png      – node-level detail
                                  (nodes inside each process, intra-process topic arrows,
                                   DDS inputs/exits labelled on the band margins)

Requirements:
    pip install matplotlib pyyaml

Usage:
    python3 scripts/render_dataflow.py
    python3 scripts/render_dataflow.py --dataflow config/dataflow.yaml \\
                                        --outdir   build/ \\
                                        --dpi      180
"""

import argparse
import sys
from collections import defaultdict
from pathlib import Path

import yaml
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch


# ── Palette ────────────────────────────────────────────────────────────────────

BG            = "#0d1117"
PROC_PALETTE  = [
    ("#0b1e35", "#2980b9"),   # sensor_preprocessor  – blue
    ("#0a1f14", "#27ae60"),   # semantic_generator   – green
    "#251508", "#e67e22",     # terrain_modeler      – orange  (fc, ec tuple below)
]
# Palette as proper tuples
PROC_PALETTE = [
    ("#0b1e35", "#2980b9"),
    ("#0a1f14", "#27ae60"),
    ("#251508", "#e67e22"),
]
PLANNER_COL   = ("#150d24", "#9b59b6")

NODE_FC       = "#111927"
NODE_EC       = "#4a90d9"
INTRA_COL     = "#4a90d9"
DDS_COL       = "#f39c12"
TEXT_COL      = "white"
TOPIC_COL     = "#a8d8f0"
DDS_TOPIC_COL = "#ffd580"

# ── Node / layout constants ────────────────────────────────────────────────────

NW = 2.7    # node box width
NH = 0.65   # node box height


# ── YAML loading ───────────────────────────────────────────────────────────────

def load_yaml(path: Path) -> dict:
    with open(path) as f:
        return yaml.safe_load(f)


# ─────────────────────────────────────────────────────────────────────────────
# IMAGE 1 – Process Overview
# ─────────────────────────────────────────────────────────────────────────────

def render_process_overview(dataflow: dict, out_path: Path, dpi: int) -> None:
    """
    Vertical stack of process boxes with labelled DDS arrows between them.
    Non-adjacent connections (skip-level) are drawn offset to the right.
    """
    processes = dataflow["processes"]

    # Map topic → process id that publishes it (via dds_outputs)
    topic_pub_proc: dict[str, str] = {}
    for proc in processes:
        for o in proc.get("dds_outputs", []):
            topic_pub_proc[o["topic"]] = proc["id"]

    # Collect cross-process DDS edges: (src_proc_id, dst_proc_id) → [topics]
    dds_edges: dict[tuple, list] = defaultdict(list)
    for proc in processes:
        for inp in proc.get("dds_inputs", []):
            src = topic_pub_proc.get(inp["topic"])
            if src and src != proc["id"]:
                dds_edges[(src, proc["id"])].append(inp["topic"])

    proc_ids = [p["id"] for p in processes]

    BW          = 5.8
    NODE_ROW_H  = 0.42   # height per node name row inside the box
    TITLE_H     = 0.55   # space for the process title at top of box
    BOX_PAD     = 0.25   # bottom padding inside box
    Y_GAP       = 1.8    # vertical gap between boxes

    # Pre-compute per-process box heights
    box_heights: dict[str, float] = {}
    for proc in processes:
        n = len(proc.get("nodes", []))
        box_heights[proc["id"]] = TITLE_H + n * NODE_ROW_H + BOX_PAD

    total_h = sum(box_heights.values()) + Y_GAP * (len(processes) - 1) + 1.4
    fig, ax = plt.subplots(figsize=(11.0, total_h))
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(BG)
    ax.axis("off")

    # Compute box center y positions (stacked with variable heights)
    proc_center_y: dict[str, float] = {}
    cur_top = 0.0
    for proc in processes:
        bh = box_heights[proc["id"]]
        proc_center_y[proc["id"]] = cur_top - bh / 2
        cur_top -= bh + Y_GAP

    # Draw process boxes
    for proc in processes:
        pid   = proc["id"]
        pidx  = proc_ids.index(pid)
        fc, ec = PROC_PALETTE[pidx]
        bh    = box_heights[pid]
        y     = proc_center_y[pid]   # centre of box
        nodes = [n["id"] for n in proc.get("nodes", [])]

        rect = FancyBboxPatch(
            (-BW / 2, y - bh / 2), BW, bh,
            boxstyle="round,pad=0.12",
            facecolor=fc, edgecolor=ec,
            linewidth=2.2, zorder=4,
        )
        ax.add_patch(rect)

        # Process title near top of box
        title_y = y + bh / 2 - TITLE_H / 2
        ax.text(0, title_y, pid,
                ha="center", va="center",
                fontsize=12, color=TEXT_COL, fontweight="bold",
                fontfamily="monospace", zorder=5)

        # Divider line under title
        div_y = y + bh / 2 - TITLE_H
        ax.plot([-BW / 2 + 0.2, BW / 2 - 0.2], [div_y, div_y],
                color=ec, lw=0.8, alpha=0.5, zorder=5)

        # Node names below the divider
        for i, nid in enumerate(nodes):
            ny = div_y - (i + 0.6) * NODE_ROW_H
            ax.text(0, ny, nid,
                    ha="center", va="center",
                    fontsize=8.5, color=TOPIC_COL,
                    fontfamily="monospace", zorder=5)

    # Draw arrows
    for (src, dst), topics in dds_edges.items():
        if src not in proc_center_y or dst not in proc_center_y:
            continue
        y0 = proc_center_y[src] - box_heights[src] / 2   # bottom of source box
        y1 = proc_center_y[dst] + box_heights[dst] / 2   # top of destination box

        src_idx = proc_ids.index(src)
        dst_idx = proc_ids.index(dst)
        skip    = dst_idx - src_idx

        x_off = 0.0 if skip == 1 else BW / 2 + 0.6
        lw    = 2.2 if skip == 1 else 1.6

        ax.annotate(
            "",
            xy=(x_off, y1), xytext=(x_off, y0),
            arrowprops=dict(
                arrowstyle="-|>",
                color=DDS_COL,
                lw=lw,
                mutation_scale=15,
                connectionstyle="arc3,rad=0.0",
            ),
            zorder=2,
        )

        label  = "\n".join(topics)
        x_text = x_off + 0.12
        ax.text(x_text, (y0 + y1) / 2, label,
                ha="left", va="center",
                fontsize=6.0, color=DDS_TOPIC_COL,
                fontfamily="monospace", zorder=6)

    # Legend
    handles = [mpatches.Patch(color=DDS_COL, label="DDS  (cross-process)")]
    leg = ax.legend(handles=handles, loc="lower right",
                    framealpha=0.3, facecolor=BG, edgecolor="#30363d",
                    labelcolor=TEXT_COL, fontsize=8)
    leg.get_title().set_color(TEXT_COL)

    pipeline = dataflow.get("pipeline", "dataflow")
    ax.set_title(f"{pipeline}  –  Process Overview",
                 color=TEXT_COL, fontsize=13, pad=10)

    ax.autoscale_view()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=dpi, bbox_inches="tight", facecolor=BG)
    plt.close(fig)
    print(f"[render_dataflow] wrote {out_path}  ({dpi} dpi)")


# ─────────────────────────────────────────────────────────────────────────────
# IMAGE 2 – Node Detail
# ─────────────────────────────────────────────────────────────────────────────

def render_node_detail(dataflow: dict, out_path: Path, dpi: int) -> None:
    """
    Custom hardcoded layout matching the exact pipeline topology:

      camera_preprocessor  – vertical chain: camera_branch → semantic_segmentation_node
      lidar_preprocessor   – vertical chain: aggregator → preprocess → terrain_model → projection
      terrain_modeler      – semantic_lifting → bev_fusion → temporal_grid_fusion → traversability_costmap
    """
    processes = dataflow["processes"]
    proc_ids  = [p["id"] for p in processes]

    # ── Band geometry ──────────────────────────────────────────────────────────
    BAND_W   = 22.0
    BAND_H   = {"camera_preprocessor": 6.0,
                "lidar_preprocessor":  9.5,
                "terrain_modeler":     6.5}
    BAND_GAP = 1.8
    EXT_W, EXT_H = 3.6, 0.52

    band_top: dict[str, float] = {}
    cur = 0.0
    for pid in proc_ids:
        band_top[pid] = cur
        cur -= BAND_H[pid] + BAND_GAP

    total_data_h = -cur + 0.5
    fig, ax = plt.subplots(figsize=((BAND_W + 7.5) * 0.48, total_data_h * 0.48))
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(BG)
    ax.axis("off")

    # ── Primitive helpers ──────────────────────────────────────────────────────

    def band_bg(pid: str) -> None:
        bt = band_top[pid]
        bh = BAND_H[pid]
        pidx   = proc_ids.index(pid)
        fc, ec = PROC_PALETTE[pidx]
        ax.add_patch(FancyBboxPatch(
            (0, bt - bh), BAND_W, bh,
            boxstyle="round,pad=0.12", facecolor=fc, edgecolor=ec,
            linewidth=2.5, zorder=1, alpha=0.78,
        ))
        ax.text(BAND_W / 2, bt - 0.27, f"[ {pid} ]",
                ha="center", va="top", fontsize=10, color=ec,
                fontweight="bold", fontfamily="monospace", zorder=6)

    def nd(x: float, y: float, label: str) -> None:
        ax.add_patch(FancyBboxPatch(
            (x - NW / 2, y - NH / 2), NW, NH,
            boxstyle="round,pad=0.07", facecolor=NODE_FC, edgecolor=NODE_EC,
            linewidth=1.5, zorder=4,
        ))
        ax.text(x, y, label, ha="center", va="center",
                fontsize=7.0, color=TEXT_COL, fontweight="bold",
                fontfamily="monospace", zorder=5)

    def ext(x: float, y: float, label: str) -> None:
        """Dashed box for external hardware/system sources."""
        ax.add_patch(FancyBboxPatch(
            (x - EXT_W / 2, y - EXT_H / 2), EXT_W, EXT_H,
            boxstyle="round,pad=0.05", facecolor="#0d1520", edgecolor="#6b7280",
            linewidth=1.0, linestyle=(0, (3, 2)), zorder=4,
        ))
        ax.text(x, y, label, ha="center", va="center",
                fontsize=6.0, color="#9ca3af",
                fontfamily="monospace", zorder=5)

    def arr(x0: float, y0: float, x1: float, y1: float,
            topic: str = "", lc=INTRA_COL, tc=TOPIC_COL,
            rad: float = 0.0, fs: float = 5.5,
            ldx: float = 0.12, ldy: float = 0.0,
            lha: str = "left") -> None:
        ax.annotate("", xy=(x1, y1), xytext=(x0, y0),
                    arrowprops=dict(arrowstyle="-|>", color=lc, lw=1.4,
                                    mutation_scale=10,
                                    connectionstyle=f"arc3,rad={rad:.3f}"),
                    zorder=3)
        if topic:
            ax.text((x0 + x1) / 2 + ldx, (y0 + y1) / 2 + ldy,
                    topic, ha=lha, va="center",
                    fontsize=fs, color=tc, fontfamily="monospace", zorder=7)

    def dds_in(x0: float, y0: float, x1: float, y1: float,
               topic: str, rad: float = 0.0,
               fs: float = 5.5, ldx: float = 0.14,
               lha: str = "left") -> None:
        arr(x0, y0, x1, y1, topic=topic,
            lc=DDS_COL, tc=DDS_TOPIC_COL, rad=rad, fs=fs, ldx=ldx, lha=lha)

    def exit_r(x: float, y: float, topic: str, dy: float = 0.0) -> None:
        """Arrow exiting the band to the right, originating from node right-edge."""
        x0, y0 = x + NW / 2, y + dy
        x1, y1 = BAND_W + 0.5, y0
        ax.annotate("", xy=(x1, y1), xytext=(x0, y0),
                    arrowprops=dict(arrowstyle="-|>", color=DDS_COL,
                                    lw=1.2, mutation_scale=9), zorder=3)
        ax.text(x1 + 0.12, y1, topic, ha="left", va="center",
                fontsize=5.2, color=DDS_TOPIC_COL,
                fontfamily="monospace", zorder=7)

    # ══════════════════════════════════════════════════════════════════════════
    # 1. camera_preprocessor
    #
    #  [camera hardware] ──/camera/image_raw ──> [camera_branch]
    #                    ──/camera/camera_info        ├──> /camera/image_rect       →
    #                                                 └──> /camera/camera_info_rect →
    #                                                 │  /camera/image_rect (intra)
    #                                                 ▼
    #                                    [semantic_segmentation_node]
    #                                                 └──> /segmentation/mask →
    # ══════════════════════════════════════════════════════════════════════════
    band_bg("camera_preprocessor")
    bt = band_top["camera_preprocessor"]

    CX = 7.0
    ext(CX, bt - 0.76, "camera hardware")
    arr(CX, bt - 0.76 - EXT_H / 2,
        CX, bt - 2.5 + NH / 2,
        topic="/camera/image_raw\n/camera/camera_info", ldx=0.14)
    nd(CX, bt - 2.5, "camera_branch")
    exit_r(CX, bt - 2.5, "/camera/image_rect",       dy=+0.24)
    exit_r(CX, bt - 2.5, "/camera/camera_info_rect",  dy=-0.24)
    arr(CX, bt - 2.5 - NH / 2,
        CX, bt - 4.8 + NH / 2,
        topic="/camera/image_rect", ldx=0.14)
    nd(CX, bt - 4.8, "semantic_segmentation_node")
    exit_r(CX, bt - 4.8, "/segmentation/mask")

    # ══════════════════════════════════════════════════════════════════════════
    # 2. lidar_preprocessor
    #
    #  [lidar hardware] ──> [lidar_aggregator_node]
    #                              │ /lidar/points_aggregated
    #                              ▼
    #                    [lidar_preprocess_node] ◄── /imu/data  /tf
    #                              │ /lidar/points_filtered
    #                              ▼
    #                      [terrain_model_node]  ◄── /ego/odometry  /tf
    #         ──> /terrain/{elevation,slope,roughness,obstacle}_map
    #                              │ /lidar/points_no_tall_objects
    #                              ▼
    #                    [lidar_projection_node]
    #         ──> /lidar/points_projection
    #         ──> /lidar/projected_depth
    # ══════════════════════════════════════════════════════════════════════════
    band_bg("lidar_preprocessor")
    bt = band_top["lidar_preprocessor"]

    LX = 6.0
    ext(LX, bt - 0.76, "lidar hardware")
    arr(LX, bt - 0.76 - EXT_H / 2,
        LX, bt - 2.2 + NH / 2,
        topic="/lidar/points_raw", ldx=0.14)
    nd(LX, bt - 2.2, "lidar_aggregator_node")
    arr(LX, bt - 2.2 - NH / 2,
        LX, bt - 3.9 + NH / 2,
        topic="/lidar/points_aggregated", ldx=0.14)
    nd(LX, bt - 3.9, "lidar_preprocess_node")

    # /imu/data + /tf → lidar_preprocess_node (from left band edge)
    dds_in(0.0, bt - 3.9, LX - NW / 2, bt - 3.9,
           "/imu/data   /tf", ldx=0.08, fs=4.8)

    arr(LX, bt - 3.9 - NH / 2,
        LX, bt - 5.7 + NH / 2,
        topic="/lidar/points_filtered", ldx=0.14)
    nd(LX, bt - 5.7, "terrain_model_node")

    # /ego/odometry + /tf → terrain_model_node (from left band edge)
    dds_in(0.0, bt - 5.7, LX - NW / 2, bt - 5.7,
           "/ego/odometry   /tf", ldx=0.08, fs=4.8)

    # Terrain map exits (right)
    exit_r(LX, bt - 5.7, "/terrain/elevation_map",  dy=+0.42)
    exit_r(LX, bt - 5.7, "/terrain/slope_map",      dy=+0.14)
    exit_r(LX, bt - 5.7, "/terrain/roughness_map",  dy=-0.14)
    exit_r(LX, bt - 5.7, "/terrain/obstacle_map",   dy=-0.42)

    arr(LX, bt - 5.7 - NH / 2,
        LX, bt - 7.6 + NH / 2,
        topic="/lidar/points_no_tall_objects", ldx=0.14)
    nd(LX, bt - 7.6, "lidar_projection_node")
    exit_r(LX, bt - 7.6, "/lidar/points_projection", dy=+0.24)
    exit_r(LX, bt - 7.6, "/lidar/projected_depth",   dy=-0.24)

    # ══════════════════════════════════════════════════════════════════════════
    # 3. terrain_modeler
    #
    #  /segmentation/mask (DDS)    ──> [semantic_lifting_node]
    #  /lidar/projected_depth (DDS) ──>          │ /terrain/semantic_points
    #  /tf (DDS)                   ──>           │
    #  terrain maps (DDS)          ──> [bev_fusion_node]
    #  /ego/odometry (DDS)         ──> [temporal_grid_fusion_node]
    #
    #  semantic_lifting → bev_fusion → temporal_grid_fusion → traversability_costmap
    #                                                                  └──> /costmap →
    # ══════════════════════════════════════════════════════════════════════════
    band_bg("terrain_modeler")
    bt = band_top["terrain_modeler"]

    SLF_X = 3.0;  BEV_X = 8.5;  TMP_X = 14.0;  TRV_X = 19.5
    NODE_Y = bt - 3.5

    nd(SLF_X, NODE_Y, "semantic_lifting_node")
    nd(BEV_X, NODE_Y, "bev_fusion_node")
    nd(TMP_X, NODE_Y, "temporal_grid_fusion_node")
    nd(TRV_X, NODE_Y, "traversability_costmap_node")

    # DDS inputs to semantic_lifting_node (from top of band)
    dds_in(SLF_X, bt, SLF_X, NODE_Y + NH / 2,
           "/segmentation/mask\n/lidar/projected_depth\n/tf",
           rad=0.0, ldx=0.14, fs=4.8)

    # DDS inputs to bev_fusion_node (terrain maps, from top of band)
    dds_in(BEV_X, bt, BEV_X, NODE_Y + NH / 2,
           "/terrain/{elevation,slope,\n  roughness,obstacle}_map",
           rad=0.0, ldx=0.14, fs=4.8)

    # DDS input: /ego/odometry → temporal_grid_fusion_node (from top of band)
    dds_in(TMP_X, bt, TMP_X, NODE_Y + NH / 2,
           "/ego/odometry", ldx=0.14, fs=4.8)

    # Intra-process edges
    arr(SLF_X + NW / 2, NODE_Y,
        BEV_X - NW / 2, NODE_Y,
        topic="/terrain/semantic_points", ldy=0.38, fs=5.0)
    arr(BEV_X + NW / 2, NODE_Y,
        TMP_X - NW / 2, NODE_Y,
        topic="/bev/feature_grid", ldy=0.38)
    arr(TMP_X + NW / 2, NODE_Y,
        TRV_X - NW / 2, NODE_Y,
        topic="/bev/stable_grid", ldy=0.38)

    # Exit
    exit_r(TRV_X, NODE_Y, "/costmap")

    # ── Legend and title ──────────────────────────────────────────────────────
    handles = [
        mpatches.Patch(color=INTRA_COL, label="intra-process"),
        mpatches.Patch(color=DDS_COL,   label="DDS in  /  exits process"),
    ]
    ax.legend(handles=handles, loc="lower right",
              framealpha=0.3, facecolor=BG, edgecolor="#30363d",
              labelcolor=TEXT_COL, fontsize=8)

    pipeline = dataflow.get("pipeline", "dataflow")
    ax.set_title(f"{pipeline}  –  Node Detail",
                 color=TEXT_COL, fontsize=13, pad=10)

    ax.autoscale_view()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=dpi, bbox_inches="tight", facecolor=BG)
    plt.close(fig)
    print(f"[render_dataflow] wrote {out_path}  ({dpi} dpi)")


# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────

def main() -> None:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument("--dataflow", default="config/dataflow.yaml",
                    help="Path to dataflow YAML (default: config/dataflow.yaml)")
    ap.add_argument("--outdir",   default="build/",
                    help="Output directory (default: build/)")
    ap.add_argument("--dpi",      type=int, default=180,
                    help="Output resolution in DPI (default: 180)")
    args = ap.parse_args()

    df_path = Path(args.dataflow)
    if not df_path.exists():
        print(f"ERROR: {df_path} not found", file=sys.stderr)
        sys.exit(1)

    dataflow = load_yaml(df_path)
    outdir   = Path(args.outdir)
    dpi      = args.dpi

    render_process_overview(dataflow, outdir / "dataflow_processes.png", dpi)
    render_node_detail(dataflow,      outdir / "dataflow_nodes.png",     dpi)


if __name__ == "__main__":
    main()
