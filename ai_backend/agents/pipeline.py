import json
import logging
from typing import AsyncGenerator
from schema.game_spec import GameSpec
from agents.architect_agent import ArchitectAgent
from agents.scene_agent import SceneAgent
from agents.coder_agent import CoderAgent
from agents.validator_agent import ValidatorAgent
from agents.scene_builder import SceneBuilder
from providers.base import AIProvider

logger = logging.getLogger(__name__)

class AgentPipeline:
    def __init__(self, provider: AIProvider, project_root: str):
        self.provider = provider
        self.project_root = project_root
        self.architect = ArchitectAgent(provider)
        self.scene = SceneAgent(provider)
        self.coder = CoderAgent(provider)
        self.validator = ValidatorAgent()
        self.builder = SceneBuilder(project_root)

    async def generate_game(self, prompt: str) -> AsyncGenerator[str, None]:
        # Helper to yield Server-Sent Events (SSE) data
        def make_sse(step: str, count: int, max_steps: int, message: str, status: int, extra: dict = None) -> str:
            data = {"step": step, "progress": int((count / max_steps) * 100), "message": message, "status": status}
            if extra:
                data.update(extra)
            return f"data: {json.dumps(data)}\n\n"

        max_steps = 5
        
        try:
            # Step 1: Architect
            yield make_sse("Architect Agent", 0, max_steps, "Analyzing game concept...", 1)
            game_spec = await self.architect.run(prompt)
            yield make_sse("Architect Agent", 1, max_steps, f"Designed: {game_spec.title} ({game_spec.genre})", 2)

            # Step 2: Scene Designer
            yield make_sse("Scene Agent", 1, max_steps, "Designing node hierarchy...", 1)
            scene_spec = await self.scene.run(game_spec)
            yield make_sse("Scene Agent", 2, max_steps, "Node hierarchy complete.", 2)

            # Step 3: Coder & Validator Loop (Max 3 rounds)
            yield make_sse("Coder Agent", 2, max_steps, "Writing GDScript logic...", 1)
            scripts = await self.coder.run(game_spec, scene_spec)
            
            yield make_sse("Validator Agent", 3, max_steps, "Validating syntax...", 1)
            
            for attempt in range(3):
                validation_result = self.validator.validate(scripts)
                if validation_result.is_valid:
                    yield make_sse("Validator Agent", 4, max_steps, "Validation passed 0 errors.", 2)
                    break
                
                yield make_sse("Coder Agent", 3, max_steps, f"Fixing {len(validation_result.errors)} errors (Attempt {attempt+1}/3)...", 1)
                scripts = await self.coder.fix_errors(scripts, validation_result.errors)
            
            if not validation_result.is_valid:
                logger.warning("Validation failed after 3 attempts. Proceeding with partial scripts.")
                yield make_sse("Validator Agent", 4, max_steps, "Validation failed. Using partial scripts.", 3)

            # Step 5: Scene Builder
            yield make_sse("Scene Builder", 4, max_steps, "Writing files to disk...", 1)
            main_scene_path = self.builder.build(game_spec, scene_spec, scripts)
            
            # Final 
            yield make_sse("Complete", 5, max_steps, "Generation complete.", 2, {"main_scene": main_scene_path})

        except Exception as e:
            logger.error(f"Pipeline failed: {e}")
            yield make_sse("Error", 0, max_steps, str(e), 3)
