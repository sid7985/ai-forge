from openai import AsyncOpenAI
from .router import AIProvider
from ..config import config


class OpenAIProvider(AIProvider):
    """OpenAI GPT-4o API provider."""

    def __init__(self):
        self.client = AsyncOpenAI(
            api_key=config.get_api_key("openai"),
        )
        self.model = "gpt-4o"

    async def generate(
        self,
        prompt: str,
        max_new_tokens: int = 4096,
        temperature: float = 0.2,
    ) -> str:
        response = await self.client.chat.completions.create(
            model=self.model,
            max_tokens=max_new_tokens,
            temperature=temperature,
            messages=[
                {
                    "role": "system",
                    "content": "You are an expert Godot 4 AI game developer. Reply ONLY with the requested JSON. No markdown formatting, no explanations."
                },
                {"role": "user", "content": prompt}
            ],
            response_format={"type": "json_object"}  # GPT-4o native JSON mode
        )
        return response.choices[0].message.content
