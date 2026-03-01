import dataclasses
from validation.gdscript_validator import validate_gdscript

@dataclasses.dataclass
class ValidationResult:
    is_valid: bool
    errors: list[str]

class ValidatorAgent:
    def __init__(self):
        pass

    def validate(self, scripts: dict[str, str]) -> ValidationResult:
        all_errors = []
        for filename, code in scripts.items():
            is_valid, error = validate_gdscript(code)
            if not is_valid:
                all_errors.append(f"In {filename}:\n{error}")
        
        return ValidationResult(
            is_valid=len(all_errors) == 0,
            errors=all_errors
        )
