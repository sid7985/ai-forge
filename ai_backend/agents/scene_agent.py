import json
import logging
from providers.base import AIProvider
from schema.game_spec import GameSpec
from schema.scene_spec import SceneNodeSpec
from validation.json_validator import extract_json_from_text

logger = logging.getLogger(__name__)

class SceneAgent:
    def __init__(self, provider: AIProvider):
        self.provider = provider
    
    async def run(self, game_spec: GameSpec) -> SceneNodeSpec:
        system_prompt = self._get_system_prompt()
        user_prompt = f"GameSpec JSON:\n{game_spec.model_dump_json(indent=2)}\n\nPlease generate the detailed SceneNodeSpec JSON hierarchy."
        
        response_text = await self.provider.complete(system_prompt, user_prompt)
        
        try:
            json_str = extract_json_from_text(response_text)
            spec_dict = json.loads(json_str)
            return SceneNodeSpec(**spec_dict)
        except Exception as e:
            logger.error(f"SceneAgent failed to parse SceneNodeSpec: {e}\nResponse: {response_text}")
            raise RuntimeError(f"Failed to generate valid SceneNodeSpec: {e}")

    def _get_system_prompt(self) -> str:
        with open("prompts/scene_designer_system.txt", "r") as f:
            return f.read()
