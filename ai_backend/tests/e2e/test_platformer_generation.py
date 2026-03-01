import pytest
import json
import os
from pathlib import Path
from ai_backend.providers.router import ProviderRouter, ProviderConfig, AIProviderType
from ai_backend.builder.scene_builder import GodotSceneBuilder
from ai_backend.validation.gdscript_validator import validate_gdscript_syntax

PLATFORMER_PROMPT = "Create a 2D platformer where a robot collects batteries and avoids drones."

# This test requires ANTHROPIC_API_KEY to be set in the environment
@pytest.mark.e2e
@pytest.mark.asyncio
async def test_full_platformer_generation_pipeline(tmp_path: Path):
    if not os.environ.get("ANTHROPIC_API_KEY"):
        pytest.skip("ANTHROPIC_API_KEY not set, skipping E2E test")
        
    config = ProviderConfig(primary=AIProviderType.CLAUDE, fallback=AIProviderType.OPENAI)
    router = ProviderRouter(config)

    system_prompt = (Path(__file__).parent.parent.parent / "ai_backend" / "prompts" / "system_base.txt").read_text()
    system_prompt += "\n\n" + (Path(__file__).parent.parent.parent / "ai_backend" / "prompts" / "platformer_system.txt").read_text()
    
    full_prompt = f"{system_prompt}\n\nUser: {PLATFORMER_PROMPT}"

    raw_response = await router.generate(full_prompt, max_new_tokens=4096)

    # Extract JSON manually for test
    from ai_backend.validation.json_validator import _extract_json_dict
    json_dict = _extract_json_dict(raw_response)
    
    from ai_backend.builder.schema import GameSceneSpec
    scene_spec = GameSceneSpec.model_validate(json_dict)

    # Validate all GDScript
    syntax_errors = validate_gdscript_syntax(scene_spec)
    assert syntax_errors == [], f"GDScript syntax errors: {syntax_errors}"

    # Build scene
    builder = GodotSceneBuilder(tmp_path)
    written_files = builder.build(scene_spec)

    # Assert build success
    assert (tmp_path / "scenes" / "main.tscn").exists()
    assert (tmp_path / "scripts" / "player_controller.gd").exists()
    assert len(written_files) >= 2

    tscn = (tmp_path / "scenes" / "main.tscn").read_text()
    assert "[gd_scene" in tscn
    assert "CharacterBody2D" in tscn
    assert "ext_resource" in tscn
