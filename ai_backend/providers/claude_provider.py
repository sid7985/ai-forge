import anthropic
from .router import AIProvider
from ..config import config


class ClaudeProvider(AIProvider):
    """Anthropic Claude API provider."""

    def __init__(self):
        self.client = anthropic.AsyncAnthropic(
            api_key=config.get_api_key("claude"),
        )
        self.model = "claude-3-5-sonnet-20240620"

    async def generate(
        self,
        prompt: str,
        max_new_tokens: int = 4096,
        temperature: float = 0.2,
    ) -> str:
        response = await self.client.messages.create(
            model=self.model,
            max_tokens=max_new_tokens,
            temperature=temperature,
            messages=[
                {"role": "user", "content": prompt}
            ],
            # Claude specific optimization for structured JSON code gen
            system="You are an expert Godot 4 AI game developer. Reply ONLY with the requested JSON. No markdown formatting, no explanations.",
        )
        return response.content[0].text
