import json
import logging
from providers.base import AIProvider
from schema.game_spec import GameSpec
from schema.scene_spec import SceneNodeSpec
from validation.json_validator import extract_json_from_text

logger = logging.getLogger(__name__)

class CoderAgent:
    def __init__(self, provider: AIProvider):
        self.provider = provider
    
    async def run(self, game_spec: GameSpec, scene_spec: SceneNodeSpec) -> dict[str, str]:
        system_prompt = self._get_system_prompt("coder_system.txt")
        user_prompt = (
            f"GameSpec:\n{game_spec.model_dump_json(indent=2)}\n\n"
            f"Scene Hierarchy:\n{scene_spec.model_dump_json(indent=2)}\n\n"
            "Please generate GDScript files for explicitly defined scripts. "
            "Output a JSON object mapping filename to code snippet string."
        )
        
        response_text = await self.provider.complete(system_prompt, user_prompt)
        return self._parse_response(response_text)

    async def fix_errors(self, scripts: dict[str, str], errors: list[str]) -> dict[str, str]:
        system_prompt = self._get_system_prompt("coder_fix_system.txt")
        user_prompt = (
            "The previous script generation resulted in these gdtoolkit errors:\n"
            f"{json.dumps(errors, indent=2)}\n\n"
            "Current Scripts:\n"
            f"{json.dumps(scripts, indent=2)}\n\n"
            "Please patch the scripts to fix the errors and return the FULL JSON mapping of filename to code snippet string constraint compliant."
        )
        
        response_text = await self.provider.complete(system_prompt, user_prompt)
        return self._parse_response(response_text)

    def _parse_response(self, text: str) -> dict[str, str]:
        try:
            json_str = extract_json_from_text(text)
            return json.loads(json_str)
        except Exception as e:
            logger.error(f"CoderAgent parse fail: {e}")
            raise RuntimeError("Failed to parse GDScript JSON map")

    def _get_system_prompt(self, filename: str) -> str:
        with open(f"prompts/{filename}", "r") as f:
            return f.read()
