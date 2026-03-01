import asyncio
import os
import sys
from pathlib import Path
from unittest.mock import AsyncMock, patch

# Add parent to path for relative imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from providers.base import AIProvider
from agents.pipeline import AgentPipeline

class MockProvider(AIProvider):
    async def complete(self, system_prompt: str, user_prompt: str, **kwargs) -> str:
        if "GameSpec" not in user_prompt and "Scene Hierarchy" not in user_prompt:
            # Architect Request
            return '''
            {
                "title": "Mock Platformer",
                "genre": "platformer",
                "description": "A 2D platformer with coin collection",
                "physics": "2d",
                "systems": ["player_controller", "score_system"],
                "art_style": "pixel",
                "has_enemies": false,
                "has_ui": true,
                "complexity": 3,
                "main_scene": {
                    "name": "Main",
                    "type": "Node"
                }
            }
            '''
        elif "GameSpec JSON:" in user_prompt:
            # Scene Agent Request
            return '''
            {
                "name": "Main",
                "type": "Node2D",
                "children": [
                    {
                        "name": "Player",
                        "type": "CharacterBody2D",
                        "script": "player.gd",
                        "properties": {"position": [100, 100]},
                        "children": [{"name": "CollisionShape2D", "type": "CollisionShape2D"}]
                    }
                ]
            }
            '''
        elif "Scene Hierarchy:" in user_prompt:
            # Coder Agent Request
            # Intentionally writing valid gdscript to pass gdtoolkit mock
            return '''
            {
                "player.gd": "extends CharacterBody2D\\n\\nfunc _physics_process(delta):\\n\\tpass"
            }
            '''
        return "{}"

async def main():
    project_root = Path("/tmp/mock_godot_project")
    project_root.mkdir(parents=True, exist_ok=True)
    
    provider = MockProvider()
    pipeline = AgentPipeline(provider=provider, project_root=str(project_root))
    
    # Mock the gdtoolkit validator to avoid environment dependency
    with patch('agents.validator_agent.validate_gdscript', return_value=(True, "")):
        print("Starting E2E pipeline...")
        async for chunk in pipeline.generate_game("Make a 2D platformer with coin collection"):
            print(f"Stream: {chunk.strip()}")
            
        # Verify files were generated
        main_tscn = project_root / "scenes" / "main.tscn"
        player_gd = project_root / "scripts" / "player.gd"
        
        assert main_tscn.exists(), "main.tscn not found!"
        assert player_gd.exists(), "player.gd not found!"
        print(f"SUCCESS: Pipeline built {main_tscn} and {player_gd}")

if __name__ == "__main__":
    asyncio.run(main())
