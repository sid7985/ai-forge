import os
from pydantic_settings import BaseSettings, SettingsConfigDict
from typing import Literal, Optional


class AppConfig(BaseSettings):
    """
    Central configuration for the AI GameForge server.
    Reads from .env file or environment variables.
    """
    # Server configuration
    host: str = "127.0.0.1"
    port: int = 8742
    log_level: Literal["debug", "info", "warning", "error", "critical"] = "info"

    # Provider Selection
    primary_provider: Literal["claude", "openai", "gemini", "local"] = "claude"
    fallback_provider: Literal["claude", "openai", "gemini", "local"] = "local"

    # API Keys (Cloud)
    anthropic_api_key: Optional[str] = None
    openai_api_key: Optional[str] = None
    google_api_key: Optional[str] = None

    # Local AI Configuration (AirLLM)
    local_model_id: str = "meta-llama/Llama-3.1-70B-Instruct"
    local_compression: Optional[Literal["4bit", "8bit"]] = "4bit"
    local_layer_shards_path: Optional[str] = None
    hf_token: Optional[str] = None
    cpu_only: bool = False

    # Generation Defaults
    max_new_tokens: int = 4096
    temperature: float = 0.2

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra="ignore"
    )

    def get_api_key(self, provider: str) -> str:
        """Helper to get the appropriate API key."""
        if provider == "claude":
            if not self.anthropic_api_key:
                raise ValueError("ANTHROPIC_API_KEY is not set.")
            return self.anthropic_api_key
        elif provider == "openai":
            if not self.openai_api_key:
                raise ValueError("OPENAI_API_KEY is not set.")
            return self.openai_api_key
        elif provider == "gemini":
            if not self.google_api_key:
                raise ValueError("GOOGLE_API_KEY is not set.")
            return self.google_api_key
        return ""


# Global configuration instance
config = AppConfig()
