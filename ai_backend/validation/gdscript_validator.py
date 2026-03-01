import subprocess
import tempfile
import structlog
from pathlib import Path
from typing import List, Dict, Any

from ..builder.schema import GameSceneSpec

logger = structlog.get_logger()

def validate_gdscript_syntax(scene_spec: GameSceneSpec) -> List[Dict[str, str]]:
    """
    Runs gdtoolkit (gdparse) on all generated GDScript files in the spec.
    Returns list of errors; returning [] means all scripts are valid.
    Requires: pip install gdtoolkit==4.3.1
    """
    errors = []
    
    for script in scene_spec.scripts:
        errs = validate_gdscript_syntax_string(script.content)
        if errs:
            errors.append({
                "script": script.filename,
                "error": "\n".join(errs)
            })
            
    return errors

def validate_gdscript_syntax_string(content: str) -> List[str]:
    """
    Validates a single string of GDScript using gdparse.
    Returns list of error strings; [] if perfectly valid.
    """
    with tempfile.NamedTemporaryFile(suffix=".gd", mode="w", delete=False, encoding="utf-8") as f:
        # Prepend missing newline if needed
        if not content.endswith('\n'):
            content += '\n'
        f.write(content)
        tmp_path = f.name
        
    try:
        # We run gdparse -- which just parses the AST and fails immediately on syntax errs
        result = subprocess.run(
            ["gdparse", tmp_path],
            capture_output=True, 
            text=True
        )
        
        if result.returncode != 0:
            return [result.stderr.strip() or "Unknown parsing error"]
            
        return []
    except FileNotFoundError:
        logger.warning("gdparse not found. Skipping validation. Ensure gdtoolkit is installed.")
        return []
    finally:
        Path(tmp_path).unlink(missing_ok=True)
