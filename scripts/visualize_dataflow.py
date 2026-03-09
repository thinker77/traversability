#!/usr/bin/env python3
"""
visualize_dataflow.py

Reads config/dataflow.yaml and generates two outputs:
  - build/dataflow.html  — interactive vis.js graph (open in any browser)
  - build/dataflow.dot   — Graphviz dot file (render with: dot -Tpng dataflow.dot -o dataflow.png)

Usage:
    python3 scripts/visualize_dataflow.py [--dataflow config/dataflow.yaml]
                                          [--output-dir build]
"""

import argparse
import json
import sys
from pathlib import Path

import yaml

# ── Color palette ─────────────────────────────────────────────────────────────
NODE_STYLE = {
    "source":   {"color": "#2ecc71", "border": "#27ae60", "font": "#ffffff"},
    "operator": {"color": "#2980b9", "border": "#1a5276", "font": "#ffffff"},
    "sink":     {"color": "#e67e22", "border": "#ca6f1e", "font": "#ffffff"},
}

MSG_COLOR = {
    "sensor_msgs/PointCloud2":    "#8e44ad",
    "sensor_msgs/Image":          "#c0392b",
    "sensor_msgs/CameraInfo":     "#d35400",
    "sensor_msgs/Imu":            "#16a085",
    "nav_msgs/Odometry":          "#2c3e50",
    "nav_msgs/OccupancyGrid":     "#27ae60",
    "geometry_msgs/PoseStamped":  "#8e44ad",
    "grid_map_msgs/GridMap":      "#f39c12",
}


# ── Graph builder ─────────────────────────────────────────────────────────────

def build_graph(dataflow: dict) -> tuple[list, list]:
    """Returns (vis_nodes, vis_edges) dicts ready for vis.js JSON."""
    nodes = dataflow["nodes"]

    # topic → node_id that publishes it
    topic_publisher: dict[str, str] = {}
    for node in nodes:
        for port in node.get("outputs", []):
            topic_publisher[port["topic"]] = node["id"]

    vis_nodes = []
    vis_edges = []
    edge_id = 0

    for node in nodes:
        ntype = node["type"]
        style = NODE_STYLE.get(ntype, NODE_STYLE["operator"])

        inputs  = node.get("inputs",  [])
        outputs = node.get("outputs", [])

        in_ports  = "\n".join(f"  ← {p['name']} ({p['topic']})" for p in inputs)
        out_ports = "\n".join(f"  → {p['name']} ({p['topic']})" for p in outputs)
        tooltip = (
            f"[{ntype.upper()}] {node['id']}\n"
            + (f"plugin: {node.get('plugin', '-')}\n" if "plugin" in node else "")
            + (f"params: {node.get('params', '-')}\n" if "params" in node else "")
            + ("\nINPUTS:\n" + in_ports if in_ports else "")
            + ("\nOUTPUTS:\n" + out_ports if out_ports else "")
        )

        vis_nodes.append({
            "id":    node["id"],
            "label": node["id"].replace("_node", "\nnode").replace("_", " "),
            "title": tooltip,
            "color": {
                "background": style["color"],
                "border":     style["border"],
                "highlight":  {"background": style["border"], "border": "#ffffff"},
            },
            "font":  {"color": style["font"], "size": 13, "face": "monospace"},
            "shape": "box",
            "margin": 10,
            "shadow": True,
        })

        # Draw edges for each input whose publisher exists in the graph
        for port in inputs:
            publisher_id = topic_publisher.get(port["topic"])
            if publisher_id:
                msg_type = port.get("msg_type", "")
                edge_color = MSG_COLOR.get(msg_type, "#7f8c8d")
                vis_edges.append({
                    "id":     edge_id,
                    "from":   publisher_id,
                    "to":     node["id"],
                    "label":  port["topic"],
                    "title":  f"{msg_type}\nqos: {port.get('qos', '-')}",
                    "color":  {"color": edge_color, "highlight": "#ffffff"},
                    "font":   {"size": 10, "color": "#ecf0f1", "face": "monospace",
                               "strokeWidth": 2, "strokeColor": "#1a1a2e"},
                    "arrows": {"to": {"enabled": True, "scaleFactor": 0.8}},
                    "smooth": {"type": "cubicBezier", "forceDirection": "horizontal"},
                    "width":  2,
                })
                edge_id += 1

    return vis_nodes, vis_edges


# ── HTML renderer ─────────────────────────────────────────────────────────────

HTML_TEMPLATE = """\
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>{pipeline} — Dataflow Graph</title>
<script src="https://unpkg.com/vis-network@9.1.9/dist/vis-network.min.js"></script>
<style>
  * {{ box-sizing: border-box; margin: 0; padding: 0; }}
  body {{
    background: #0d1117;
    color: #e6edf3;
    font-family: 'Segoe UI', monospace, sans-serif;
    display: flex;
    flex-direction: column;
    height: 100vh;
  }}
  header {{
    padding: 12px 20px;
    background: #161b22;
    border-bottom: 1px solid #30363d;
    display: flex;
    align-items: center;
    gap: 16px;
    flex-shrink: 0;
  }}
  header h1 {{
    font-size: 15px;
    font-weight: 600;
    color: #58a6ff;
    letter-spacing: 0.5px;
  }}
  header span {{
    font-size: 12px;
    color: #8b949e;
  }}
  #controls {{
    display: flex;
    gap: 8px;
    margin-left: auto;
  }}
  button {{
    background: #21262d;
    border: 1px solid #30363d;
    color: #e6edf3;
    padding: 5px 12px;
    border-radius: 6px;
    cursor: pointer;
    font-size: 12px;
  }}
  button:hover {{ background: #30363d; }}
  #legend {{
    display: flex;
    gap: 16px;
    padding: 8px 20px;
    background: #0d1117;
    border-bottom: 1px solid #21262d;
    flex-shrink: 0;
    flex-wrap: wrap;
    align-items: center;
  }}
  .legend-item {{
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 11px;
    color: #8b949e;
  }}
  .legend-box {{
    width: 14px; height: 14px;
    border-radius: 3px;
    border: 1px solid rgba(255,255,255,0.2);
  }}
  .legend-line {{
    width: 24px; height: 3px;
    border-radius: 2px;
  }}
  #graph {{
    flex: 1;
    background: #0d1117;
  }}
  #info-panel {{
    position: absolute;
    bottom: 20px;
    right: 20px;
    width: 280px;
    background: #161b22;
    border: 1px solid #30363d;
    border-radius: 8px;
    padding: 14px;
    font-size: 12px;
    line-height: 1.6;
    display: none;
    white-space: pre-wrap;
    font-family: monospace;
    color: #e6edf3;
    z-index: 100;
    max-height: 50vh;
    overflow-y: auto;
  }}
  #info-panel h3 {{
    color: #58a6ff;
    margin-bottom: 8px;
    font-size: 13px;
  }}
</style>
</head>
<body>

<header>
  <h1>⬡ {pipeline}</h1>
  <span>dataflow graph · {n_nodes} nodes · {n_edges} wires</span>
  <div id="controls">
    <button onclick="network.fit()">Fit</button>
    <button onclick="network.setOptions({{layout: {{hierarchical: {{enabled: true, direction: 'LR'}}}}}})">Hierarchical</button>
    <button onclick="network.setOptions({{layout: {{hierarchical: {{enabled: false}}}}}})">Free</button>
    <button onclick="document.getElementById('info-panel').style.display='none'">✕ Info</button>
  </div>
</header>

<div id="legend">
  <span style="font-size:11px;color:#8b949e;margin-right:4px;">Nodes:</span>
  <div class="legend-item">
    <div class="legend-box" style="background:#2ecc71;border-color:#27ae60;"></div> source
  </div>
  <div class="legend-item">
    <div class="legend-box" style="background:#2980b9;border-color:#1a5276;"></div> operator
  </div>
  <div class="legend-item">
    <div class="legend-box" style="background:#e67e22;border-color:#ca6f1e;"></div> sink
  </div>
  <span style="margin-left:12px;font-size:11px;color:#8b949e;">Wires (msg type):</span>
  <div class="legend-item"><div class="legend-line" style="background:#8e44ad;"></div> PointCloud2 / PoseStamped</div>
  <div class="legend-item"><div class="legend-line" style="background:#c0392b;"></div> Image</div>
  <div class="legend-item"><div class="legend-line" style="background:#f39c12;"></div> GridMap</div>
  <div class="legend-item"><div class="legend-line" style="background:#27ae60;"></div> OccupancyGrid</div>
  <div class="legend-item"><div class="legend-line" style="background:#2c3e50;"></div> Odometry</div>
  <div class="legend-item"><div class="legend-line" style="background:#7f8c8d;"></div> other</div>
</div>

<div id="graph"></div>
<div id="info-panel"><h3 id="info-title"></h3><pre id="info-body"></pre></div>

<script>
const nodesData = {nodes_json};
const edgesData = {edges_json};

const container = document.getElementById("graph");
const network = new vis.Network(container, {{
  nodes: new vis.DataSet(nodesData),
  edges: new vis.DataSet(edgesData),
}}, {{
  layout: {{
    hierarchical: {{
      enabled: true,
      direction: "LR",
      sortMethod: "directed",
      levelSeparation: 220,
      nodeSpacing: 120,
      treeSpacing: 180,
    }}
  }},
  physics: {{ enabled: false }},
  interaction: {{
    hover: true,
    tooltipDelay: 200,
    navigationButtons: false,
    keyboard: true,
  }},
}});

// Click node or edge → show info panel
network.on("click", (params) => {{
  const panel = document.getElementById("info-panel");
  const title = document.getElementById("info-title");
  const body  = document.getElementById("info-body");
  if (params.nodes.length > 0) {{
    const node = nodesData.find(n => n.id === params.nodes[0]);
    title.textContent = node.id;
    body.textContent  = node.title;
    panel.style.display = "block";
  }} else if (params.edges.length > 0) {{
    const edge = edgesData.find(e => e.id === params.edges[0]);
    title.textContent = edge.label;
    body.textContent  = edge.title;
    panel.style.display = "block";
  }} else {{
    panel.style.display = "none";
  }}
}});
</script>
</body>
</html>
"""


# ── Dot renderer ──────────────────────────────────────────────────────────────

DOT_NODE_ATTRS = {
    "source":   'style=filled fillcolor="#2ecc71" fontcolor=white',
    "operator": 'style=filled fillcolor="#2980b9" fontcolor=white',
    "sink":     'style=filled fillcolor="#e67e22" fontcolor=white',
}


def render_dot(dataflow: dict, vis_nodes: list, vis_edges: list) -> str:
    node_map = {n["id"]: n for n in dataflow["nodes"]}
    lines = [
        'digraph dataflow {',
        '  rankdir=LR;',
        '  bgcolor="#0d1117";',
        '  node [shape=box fontname="monospace" fontsize=11 margin="0.2,0.1"];',
        '  edge [fontname="monospace" fontsize=9];',
        '',
    ]

    for vn in vis_nodes:
        node = node_map[vn["id"]]
        attrs = DOT_NODE_ATTRS.get(node["type"], DOT_NODE_ATTRS["operator"])
        label = vn["id"]
        lines.append(f'  "{label}" [{attrs} label="{label}"];')

    lines.append('')
    for ve in vis_edges:
        topic = ve["label"].replace("/", "/\\n")
        lines.append(
            f'  "{ve["from"]}" -> "{ve["to"]}" '
            f'[label="{topic}" color="{ve["color"]["color"]}" fontcolor="#cccccc"];'
        )

    lines.append('}')
    return '\n'.join(lines)


# ── Main ──────────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dataflow",   default="config/dataflow.yaml")
    parser.add_argument("--output-dir", default="build")
    args = parser.parse_args()

    dataflow_path = Path(args.dataflow)
    if not dataflow_path.exists():
        print(f"ERROR: {dataflow_path} not found", file=sys.stderr)
        sys.exit(1)

    with open(dataflow_path) as f:
        dataflow = yaml.safe_load(f)

    vis_nodes, vis_edges = build_graph(dataflow)

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # ── HTML ──────────────────────────────────────────────────────────────────
    html = HTML_TEMPLATE.format(
        pipeline  = dataflow.get("pipeline", "dataflow"),
        n_nodes   = len(vis_nodes),
        n_edges   = len(vis_edges),
        nodes_json= json.dumps(vis_nodes, indent=2),
        edges_json= json.dumps(vis_edges, indent=2),
    )
    html_path = out_dir / "dataflow.html"
    html_path.write_text(html, encoding="utf-8")
    print(f"[visualize] wrote {html_path}  ← open in browser")

    # ── Dot ───────────────────────────────────────────────────────────────────
    dot = render_dot(dataflow, vis_nodes, vis_edges)
    dot_path = out_dir / "dataflow.dot"
    dot_path.write_text(dot, encoding="utf-8")
    print(f"[visualize] wrote {dot_path}   ← render with: dot -Tpng {dot_path} -o build/dataflow.png")


if __name__ == "__main__":
    main()
