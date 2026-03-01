import pytest
from ai_backend.validation.gdscript_validator import validate_gdscript_syntax_string
from ai_backend.validation.json_validator import extract_and_validate_json

VALID_GDSCRIPT = """
extends CharacterBody2D

const SPEED: float = 200.0
const JUMP_VELOCITY: float = -450.0

func _physics_process(delta: float) -> void:
\tif not is_on_floor():
\t\tvelocity += get_gravity() * delta
\tif Input.is_action_just_pressed("ui_accept") and is_on_floor():
\t\tvelocity.y = JUMP_VELOCITY
\tvar direction: float = Input.get_axis("ui_left", "ui_right")
\tvelocity.x = direction * SPEED
\tmove_and_slide()
"""

INVALID_GDSCRIPT_SYNTAX = """
extends Node2D

func broken(
\tpass
"""

def test_valid_gdscript_passes():
    errors = validate_gdscript_syntax_string(VALID_GDSCRIPT)
    assert errors == []

def test_syntax_error_is_caught():
    errors = validate_gdscript_syntax_string(INVALID_GDSCRIPT_SYNTAX)
    assert len(errors) > 0
    assert "ParseError" in errors[0] or "unexpected" in errors[0].lower()

def test_json_extractor_handles_markdown_blocks():
    raw_response = '''Sure, here is your game spec:
```json
{
  "scene_name": "main",
  "root_node": {"name": "Root", "type": "Node2D"},
  "scripts": [],
  "connections": [],
  "autoloads": []
}
```
Good luck!
'''
    spec = extract_and_validate_json(raw_response)
    assert spec.scene_name == "main"
    assert spec.root_node.name == "Root"
    
def test_json_extractor_fails_on_bad_schema():
    raw_response = '''{ "scene_name": "main" }''' # Missing required root_node
    with pytest.raises(ValueError, match="AI returned invalid GameSceneSpec JSON"):
        extract_and_validate_json(raw_response)
