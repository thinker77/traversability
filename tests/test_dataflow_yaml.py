import pathlib
import yaml
import pytest


ROOT = pathlib.Path(__file__).resolve().parents[1]
DATAFLOW_PATH = ROOT / "config" / "dataflow.yaml"


def load_dataflow():
    with DATAFLOW_PATH.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f)


def iter_nodes(data: dict):
    """Yield every node dict regardless of schema (flat or process-based)."""
    if "processes" in data:
        for proc in data["processes"]:
            yield from proc.get("nodes", [])
    else:
        yield from data.get("nodes", [])


def test_dataflow_parses():
    data = load_dataflow()
    assert isinstance(data, dict), "dataflow.yaml must parse to a dict"
    assert "version" in data, "dataflow.yaml missing 'version'"
    assert "pipeline" in data, "dataflow.yaml missing 'pipeline'"
    assert "processes" in data and isinstance(data["processes"], list), \
        "dataflow.yaml missing 'processes' list"


def test_processes_have_required_fields():
    data = load_dataflow()
    for proc in data["processes"]:
        assert isinstance(proc, dict), "each process must be a mapping"
        assert "id" in proc and proc["id"], "process missing 'id'"
        assert "executable" in proc and proc["executable"], "process missing 'executable'"


def test_nodes_have_required_fields():
    data = load_dataflow()
    for node in iter_nodes(data):
        assert isinstance(node, dict), "each node must be a mapping"
        assert "id" in node and node["id"], "node missing 'id'"


def test_operator_params_files_exist():
    data = load_dataflow()
    for node in iter_nodes(data):
        if "params" in node:
            params_path = (ROOT / node["params"]).resolve()
            assert params_path.exists(), \
                f"params file referenced by node '{node['id']}' does not exist: {node['params']}"


def test_operator_source_dirs_exist():
    data = load_dataflow()
    src_root = ROOT / "src"
    assert src_root.exists(), "src/ directory not found in repo root"
    src_dirs = {p.name for p in src_root.iterdir() if p.is_dir()}

    for node in iter_nodes(data):
        node_id = node.get("id", "")
        base = node_id[: -len("_node")] if node_id.endswith("_node") else node_id

        matched = base in src_dirs

        if not matched and "plugin" in node and isinstance(node["plugin"], str):
            plugin_prefix = node["plugin"].split("::")[0].lower()
            matched = any(plugin_prefix in d for d in src_dirs)

        assert matched, (
            f"Could not find a source directory under src/ for node '{node_id}'. "
            f"Expected directory named '{base}' (one of: {sorted(src_dirs)})"
        )
