from pydantic import BaseModel, Field
from typing import List, Literal
from schema.scene_spec import SceneNodeSpec

class GameSpec(BaseModel):
    title: str = Field(..., description="The title of the game")
    genre: Literal["platformer","top_down","rpg","puzzle","endless_runner","shooter","tower_defense","match3","racing","strategy"]
    description: str = Field(..., description="A short description of the core gameplay loop")
    physics: Literal["2d","3d","none"]
    systems: List[str] = Field(..., description="List of required gameplay systems, e.g. ['player_controller', 'enemy_ai', 'inventory']")
    art_style: Literal["pixel","cartoon","minimalist","realistic"]
    has_enemies: bool
    has_ui: bool
    complexity: int = Field(..., ge=1, le=10, description="Estimated complexity from 1 to 10")
    main_scene: SceneNodeSpec = Field(..., description="The root scene tree representing the primary game view/level")
