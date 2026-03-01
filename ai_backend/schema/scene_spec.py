from pydantic import BaseModel, Field
from typing import List, Literal

class SceneNodeSpec(BaseModel):
    name: str = Field(description="Name of the node")
    type: str = Field(description="Godot node type e.g. 'CharacterBody2D', 'Node2D', 'CollisionShape2D'")
    script: str | None = Field(default=None, description="Filename of the GDScript to attach, e.g. 'player.gd'")
    properties: dict = Field(default_factory=dict, description="Dictionary of Godot properties to set")
    children: List["SceneNodeSpec"] = Field(default_factory=list, description="Child nodes")

SceneNodeSpec.model_rebuild()
