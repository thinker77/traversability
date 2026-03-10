import pathlib
import yaml
import pytest


ROOT = pathlib.Path(__file__).resolve().parents[1]
DATAFLOW_PATH = ROOT / "config" / "dataflow.yaml"


def load_dataflow():
    with DATAFLOW_PATH.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f)


def test_dataflow_parses():
    data = load_dataflow()
    assert isinstance(data, dict), "dataflow.yaml must parse to a dict"
    assert "version" in data, "dataflow.yaml missing 'version'"
    assert "pipeline" in data, "dataflow.yaml missing 'pipeline'"
    assert "nodes" in data and isinstance(data["nodes"], list), "dataflow.yaml missing 'nodes' list"


def test_nodes_have_required_fields():
    data = load_dataflow()
    for n in data["nodes"]:
        assert isinstance(n, dict), "each node must be a mapping"
        assert "id" in n and n["id"], "node missing 'id'"
        assert "type" in n and n["type"] in {"source", "operator", "sink"}, "node 'type' invalid"


def test_operator_params_files_exist():
    data = load_dataflow()
    for n in data["nodes"]:
        if n.get("type") == "operator" and "params" in n:
            params_path = (ROOT / n["params"]).resolve()
            assert params_path.exists(), f"params file referenced by node '{n['id']}' does not exist: {n['params']}"


def test_operator_source_dirs_exist():
    data = load_dataflow()
    src_root = ROOT / "src"
    assert src_root.exists(), "src/ directory not found in repo root"
    src_dirs = {p.name for p in src_root.iterdir() if p.is_dir()}

    for n in data["nodes"]:
        if n.get("type") != "operator":
            continue
        node_id = n.get("id", "")
        base = node_id
        if base.endswith("_node"):
            base = base[: -len("_node")]

        # Accept an exact directory match for the node base name
        matched = base in src_dirs

        # fallback: check plugin name prefix if provided (e.g. package::Class)
        if not matched and "plugin" in n and isinstance(n["plugin"], str):
            plugin_prefix = n["plugin"].split("::")[0].lower()
            matched = any(plugin_prefix in d for d in src_dirs)

        assert matched, (
            f"Could not find a source directory under src/ for operator node '{node_id}'. "
            f"Expected directory named '{base}' (one of: {sorted(src_dirs)})"
        )
