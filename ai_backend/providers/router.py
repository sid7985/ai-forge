from abc import ABC, abstractmethod
import structlog
from typing import Optional
from enum import Enum

logger = structlog.get_logger()

class AIProviderType(str, Enum):
    CLAUDE = "claude"
    OPENAI = "openai"
    GEMINI = "gemini"
    LOCAL = "local"


class AIProvider(ABC):
    """Abstract base class for all AI model providers."""

    @abstractmethod
    async def generate(
        self,
        prompt: str,
        max_new_tokens: int = 4096,
        temperature: float = 0.2,
    ) -> str:
        """Generate a response from the AI model."""
        pass


class ProviderConfig:
    def __init__(
        self,
        primary: AIProviderType,
        fallback: Optional[AIProviderType] = None,
    ):
        self.primary = primary
        self.fallback = fallback


class ProviderRouter:
    """
    Selects AI provider based on configuration.
    Falls back if the primary provider fails.
    """

    def __init__(self, config: ProviderConfig):
        self._config = config
        self._primary_instance = self._init_provider(config.primary)
        self._fallback_instance = (
            self._init_provider(config.fallback) if config.fallback else None
        )

    async def generate(self, prompt: str, **kwargs) -> str:
        """
        Attempts generation with primary provider.
        If it fails, automatically falls back to secondary if configured.
        """
        try:
            logger.info(f"Generating with primary provider: {self._config.primary}")
            return await self._primary_instance.generate(prompt, **kwargs)
        except Exception as e:
            logger.error(f"Primary provider failed: {e}")
            if self._fallback_instance:
                logger.info(f"Falling back to: {self._config.fallback}")
                return await self._fallback_instance.generate(prompt, **kwargs)
            raise e

    def get_provider(self, provider_id: Optional[str] = None) -> AIProvider:
        if provider_id:
            return self._init_provider(AIProviderType(provider_id))
        return self._primary_instance

    def _init_provider(self, ptype: AIProviderType) -> AIProvider:
        if ptype == AIProviderType.CLAUDE:
            from .claude_provider import ClaudeProvider
            return ClaudeProvider()
        elif ptype == AIProviderType.OPENAI:
            from .openai_provider import OpenAIProvider
            return OpenAIProvider()
        elif ptype == AIProviderType.GEMINI:
            from .gemini_provider import GeminiProvider
            return GeminiProvider()
        elif ptype == AIProviderType.LOCAL:
            from .airllm_provider import AirLLMProvider
            return AirLLMProvider()
        else:
            raise ValueError(f"Unknown provider type: {ptype}")

    def list_available_providers(self) -> list[str]:
        # Returns all supported providers for this installation
        from ..config import config as app_config
        available = ["local"]  # Always supported (if deps installed)
        if app_config.anthropic_api_key:
            available.append("claude")
        if app_config.openai_api_key:
            available.append("openai")
        if app_config.google_api_key:
            available.append("gemini")
        return available
