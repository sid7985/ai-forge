import json
import re
import structlog
from typing import Dict, Any

from pydantic import ValidationError
from ..builder.schema import GameSceneSpec, PatchOperation

logger = structlog.get_logger()


def extract_and_validate_json(raw_text: str) -> GameSceneSpec:
    """
    Extracts JSON block from raw LLM output (bypassing conversational wrapper)
    and validates it against the Pydantic GameSceneSpec model.
    """
    json_dict = _extract_json_dict(raw_text)
    
    try:
        # Let Pydantic validate and enforce nested schemas
        spec = GameSceneSpec.model_validate(json_dict)
        return spec
    except ValidationError as e:
        logger.error(f"JSON validation failed: {e}")
        raise ValueError(f"AI returned invalid GameSceneSpec JSON structure: {str(e)}")


def extract_patch_operations(raw_text: str) -> list[PatchOperation]:
    """
    Extracts JSON patch arrays from raw LLM output for conversational refinement.
    """
    json_list = _extract_json_list(raw_text)
    
    ops = []
    try:
        for item in json_list:
            ops.append(PatchOperation.model_validate(item))
        return ops
    except ValidationError as e:
        logger.error(f"Patch operation validation failed: {e}")
        raise ValueError(f"AI returned invalid PatchOperation format: {str(e)}")


def _extract_json_dict(text: str) -> Dict[str, Any]:
    # Try finding markdown JSON block
    match = re.search(r'```(?:json)?\s*(\{.*?\})\s*```', text, re.DOTALL)
    if match:
        try:
            return json.loads(match.group(1))
        except json.JSONDecodeError:
            pass
            
    # Try matching first { to last }
    match = re.search(r'(\{.*\})', text, re.DOTALL)
    if match:
        try:
            return json.loads(match.group(1))
        except json.JSONDecodeError:
            pass
            
    raise ValueError("Could not locate a valid JSON object in the AI response.")


def _extract_json_list(text: str) -> list[Dict[str, Any]]:
    # Try finding markdown JSON block
    match = re.search(r'```(?:json)?\s*(\[.*?\])\s*```', text, re.DOTALL)
    if match:
        try:
            return json.loads(match.group(1))
        except json.JSONDecodeError:
            pass
            
    # Try matching first [ to last ]
    match = re.search(r'(\[.*\])', text, re.DOTALL)
    if match:
        try:
            return json.loads(match.group(1))
        except json.JSONDecodeError:
            pass
            
    raise ValueError("Could not locate a valid JSON array in the AI response.")
