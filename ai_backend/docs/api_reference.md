# API Reference

The AI GameForge backend exposes a standard FastAPI REST interface. All endpoints return JSON.

Base URL: `http://127.0.0.1:8742`

## Endpoints

### `POST /generate`
Converts a natural language game prompt into a `GameSceneSpec`.

**Request Body:**
```json
{
  "prompt": "Create a 2D Platformer...",
  "template": "platformer", 
  "provider": "cloud",
  "cloud_provider": "claude",
  "local_model": "meta-llama/Llama-3.1-70B-Instruct",
  "local_compression": "4bit"
}
```

**Response (200 OK):**
```json
{
  "status": "ok",
  "scene_spec": {
    "scene_name": "main",
    "root_node": { ... },
    "scripts": [ ... ],
    "connections": [ ... ],
    "autoloads": [ ... ]
  }
}
```

---

### `POST /refine`
Applies a surgical conversational patch string to an existing scene JSON spec using the AST Patch engine.

**Request Body:**
```json
{
  "instruction": "Add a double jump mechanic",
  "current_scene_json": { ... },
  "provider": "cloud"
}
```

**Response (200 OK):**
```json
{
  "status": "ok",
  "patch_ops": [
    {
      "op": "modify_function",
      "target_script": "player_controller.gd",
      "function_name": "_physics_process",
      "summary": "Added double jump"
    }
  ]
}
```

---

### `POST /build`
Executes the `GodotSceneBuilder` to transpile a JSON Scene Spec into actual Godot files on disk.

**Request Body:**
```json
{
  "scene_spec": { ... },
  "project_root": "/absolute/path/to/godot/project",
  "create_snapshot": true
}
```

**Response (200 OK):**
```json
{
  "status": "ok",
  "files_written": [
    "/absolute/path/to/godot/project/scenes/main.tscn",
    "/absolute/path/to/godot/project/scripts/player_controller.gd"
  ]
}
```

---

### `GET /health`
Sanity check. Ensure server is alive.

```json
{
  "status": "ok",
  "version": "1.0.0"
}
```

---

### `GET /providers`
Returns currently configured and available AI providers.

```json
{
  "available": ["claude", "local"],
  "primary": "claude",
  "fallback": "local"
}
```
