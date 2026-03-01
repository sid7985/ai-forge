from pydantic import BaseModel
from typing import Literal

class GenerateRequest(BaseModel):
    prompt: str
    project_root: str
    provider: str
    api_key: str | None = None
    mode: Literal["generate", "refine"] = "generate"

class RefineRequest(BaseModel):
    prompt: str
    project_root: str
    provider: str
    api_key: str | None = None

class RestoreRequest(BaseModel):
    project_root: str
    timestamp: str
