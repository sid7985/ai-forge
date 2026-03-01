import json
import logging
from providers.base import AIProvider
from schema.game_spec import GameSpec
from validation.json_validator import extract_json_from_text

logger = logging.getLogger(__name__)

class ArchitectAgent:
    def __init__(self, provider: AIProvider):
        self.provider = provider
    
    async def run(self, prompt: str) -> GameSpec:
        system_prompt = self._get_system_prompt()
        user_prompt = f"User Request: {prompt}\n\nPlease generate the detailed GameSpec JSON for this game."
        
        response_text = await self.provider.complete(system_prompt, user_prompt)
        
        try:
            json_str = extract_json_from_text(response_text)
            spec_dict = json.loads(json_str)
            return GameSpec(**spec_dict)
        except Exception as e:
            logger.error(f"ArchitectAgent failed to parse GameSpec: {e}\nResponse: {response_text}")
            raise RuntimeError(f"Failed to generate valid GameSpec: {e}")

    def _get_system_prompt(self) -> str:
        with open("prompts/architect_system.txt", "r") as f:
            return f.read()
