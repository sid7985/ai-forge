from pathlib import Path
from typing import List, Dict, Any
from schema.game_spec import GameSpec
from schema.scene_spec import SceneNodeSpec

class SceneBuilder:
    """
    Converts GameSpec + SceneSpec JSON → valid Godot 4 .tscn + .gd files.
    Deterministic: same input always produces identical output.
    """

    def __init__(self, project_root: str | Path):
        self.project_root = Path(project_root)
        self.ext_resources: List[Dict[str, str]] = []
        self.resource_id_counter = 1

    def build(self, game_spec: GameSpec, scene_spec: SceneNodeSpec, scripts: dict[str, str]) -> Path:
        """Returns the main scene path."""
        self.ext_resources.clear()
        self.resource_id_counter = 1

        # 1. Write all GDScript files first
        for filename, content in scripts.items():
            self._write_script(filename, content)

        # 2. Build .tscn from root node
        tscn_path = self._build_scene(game_spec, scene_spec)
        return tscn_path

    def _write_script(self, filename: str, content: str) -> Path:
        dest = self.project_root / "scripts" / filename
        dest.parent.mkdir(parents=True, exist_ok=True)
        # Ensure it ends with newline
        if not content.endswith('\n'):
            content += '\n'
        dest.write_text(content, encoding="utf-8")
        return dest

    def _build_scene(self, game_spec: GameSpec, scene_spec: SceneNodeSpec) -> Path:
        # First pass: collect ext_resources
        self._collect_ext_resources(scene_spec)
        
        lines: List[str] = []
        # Header
        lines.append(f'[gd_scene load_steps={len(self.ext_resources) + 1} format=3]')
        lines.append("")
        
        # External Resources
        for res in self.ext_resources:
            lines.append(f'[ext_resource type="{res["type"]}" path="{res["path"]}" id="{res["id"]}"]')
        if self.ext_resources:
            lines.append("")

        # Root Node
        root = scene_spec
        lines.append(f'[node name="{root.name}" type="{root.type}"]')
        if root.script:
            res_id = self._get_resource_id(f"res://scripts/{root.script}")
            if res_id:
                lines.append(f'script = ExtResource("{res_id}")')
        
        # Children Nodes
        self._emit_children(root.children, parent=".", lines=lines)

        tscn_content = "\n".join(lines) + "\n"
        # We use standard main.tscn for now
        dest = self.project_root / "scenes" / "main.tscn"
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_text(tscn_content, encoding="utf-8")
        return dest

    def _collect_ext_resources(self, node: SceneNodeSpec) -> None:
        if node.script:
            self._register_resource("Script", f"res://scripts/{node.script}")
        # Assuming properties could contain texture paths
        texture = node.properties.get("texture")
        if texture:
            self._register_resource("Texture2D", texture)
            
        for child in node.children:
            self._collect_ext_resources(child)

    def _emit_children(self, children: List[SceneNodeSpec], parent: str, lines: List[str]) -> None:
        for child in children:
            lines.append("")
            node_line = f'[node name="{child.name}" type="{child.type}" parent="{parent}"]'
            lines.append(node_line)
            
            position = child.properties.get("position")
            if position:
                lines.append(f'position = Vector2({position[0]}, {position[1]})')
                
            if child.script:
                res_id = self._get_resource_id(f"res://scripts/{child.script}")
                if res_id:
                    lines.append(f'script = ExtResource("{res_id}")')
                    
            texture = child.properties.get("texture")
            if texture:
                res_id = self._get_resource_id(texture)
                if res_id:
                    lines.append(f'texture = ExtResource("{res_id}")')
                    
            if child.type == "CollisionShape2D":
                shape_type = child.properties.get("shape_type", "CapsuleShape2D")
                if shape_type == "CapsuleShape2D":
                    r = child.properties.get("radius", 10)
                    h = child.properties.get("height", 20)
                    res_id = f"SubResource(\"{shape_type}_auto\")"
                    lines.insert(0, f'[sub_resource type="{shape_type}" id="{shape_type}_auto"]')
                    lines.insert(1, f'radius = {r}')
                    lines.insert(2, f'height = {h}')
                    lines.insert(3, "")
                    lines.append(f'shape = {res_id}')
                elif shape_type == "CircleShape2D":
                    r = child.properties.get("radius", 10)
                    res_id = f"SubResource(\"{shape_type}_auto\")"
                    lines.insert(0, f'[sub_resource type="{shape_type}" id="{shape_type}_auto"]')
                    lines.insert(1, f'radius = {r}')
                    lines.insert(2, "")
                    lines.append(f'shape = {res_id}')

            # Recurse
            next_parent = f"{parent}/{child.name}" if parent != "." else child.name
            self._emit_children(child.children, parent=next_parent, lines=lines)

    def _register_resource(self, res_type: str, path: str) -> None:
        # Check if already registered
        for res in self.ext_resources:
            if res["path"] == path:
                return
                
        self.ext_resources.append({
            "type": res_type,
            "path": path,
            "id": f"{self.resource_id_counter}_ext"
        })
        self.resource_id_counter += 1

    def _get_resource_id(self, path: str) -> str | None:
        for res in self.ext_resources:
            if res["path"] == path:
                return res["id"]
        return None
