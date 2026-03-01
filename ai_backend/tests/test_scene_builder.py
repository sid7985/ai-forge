import pytest
from pathlib import Path
from ai_backend.builder.scene_builder import GodotSceneBuilder
from ai_backend.builder.schema import GameSceneSpec, NodeSpec, ScriptSpec

MINIMAL_SPEC = GameSceneSpec(
    scene_name="test_level",
    root_node=NodeSpec(
        name="Main",
        type="Node2D",
        children=[
            NodeSpec(
                name="Player",
                type="CharacterBody2D",
                position=[400.0, 300.0],
                script="player_controller.gd",
                children=[
                    NodeSpec(name="Sprite2D", type="Sprite2D"),
                    NodeSpec(name="CollisionShape2D", type="CollisionShape2D", shape={"type": "CapsuleShape2D"}),
                ]
            )
        ]
    ),
    scripts=[
        ScriptSpec(
            filename="player_controller.gd",
            content="extends CharacterBody2D\n\nfunc _physics_process(delta: float) -> void:\n\tmove_and_slide()\n"
        )
    ],
    connections=[],
    autoloads=[]
)

def test_scene_builder_creates_tscn(tmp_path: Path):
    builder = GodotSceneBuilder(tmp_path)
    builder.build(MINIMAL_SPEC)
    tscn_file = tmp_path / "scenes" / "test_level.tscn"
    assert tscn_file.exists()
    content = tscn_file.read_text()
    assert "[gd_scene" in content
    assert 'name="Player"' in content
    assert 'type="CharacterBody2D"' in content
    assert "CapsuleShape2D" in content

def test_scene_builder_creates_scripts(tmp_path: Path):
    builder = GodotSceneBuilder(tmp_path)
    builder.build(MINIMAL_SPEC)
    script_file = tmp_path / "scripts" / "player_controller.gd"
    assert script_file.exists()
    assert "extends CharacterBody2D" in script_file.read_text()

def test_scene_builder_is_deterministic(tmp_path: Path):
    """Same spec must always produce identical output."""
    b1 = GodotSceneBuilder(tmp_path / "run1")
    b2 = GodotSceneBuilder(tmp_path / "run2")
    f1 = (tmp_path / "run1" / "scenes" / "test_level.tscn")
    f2 = (tmp_path / "run2" / "scenes" / "test_level.tscn")
    
    b1.build(MINIMAL_SPEC)
    b2.build(MINIMAL_SPEC)
    
    assert f1.read_text() == f2.read_text()

def test_scene_builder_registers_scripts_as_ext_resource(tmp_path: Path):
    builder = GodotSceneBuilder(tmp_path)
    builder.build(MINIMAL_SPEC)
    content = (tmp_path / "scenes" / "test_level.tscn").read_text()
    assert '[ext_resource type="Script"' in content
    assert 'script = ExtResource(' in content
