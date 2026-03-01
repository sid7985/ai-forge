import google.generativeai as genai
from .router import AIProvider
from ..config import config


class GeminiProvider(AIProvider):
    """Google Gemini API provider."""

    def __init__(self):
        genai.configure(api_key=config.get_api_key("gemini"))
        self.model = genai.GenerativeModel('gemini-1.5-pro')

    async def generate(
        self,
        prompt: str,
        max_new_tokens: int = 4096,
        temperature: float = 0.2,
    ) -> str:
        # Gemini 1.5 system instruction override
        self.model = genai.GenerativeModel(
            'gemini-1.5-pro',
            system_instruction="You are an expert Godot 4 AI game developer. Reply ONLY with the requested JSON. No markdown formatting, no explanations."
        )
        
        response = await self.model.generate_content_async(
            prompt,
            generation_config=genai.types.GenerationConfig(
                max_output_tokens=max_new_tokens,
                temperature=temperature,
                response_mime_type="application/json", # Gemini 1.5 native JSON mode
            )
        )
        return response.text
