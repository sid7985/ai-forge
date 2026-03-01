# 🤖 AI GameForge 

AI GameForge is an AI-first game generation layer built on top of Godot 4. It enables developers to go from natural language description → playable 2D Godot prototype in under 30 minutes, using either cloud AI (Claude/GPT-4o/Gemini) or full local AI (running 70B models via AirLLM on a 4GB GPU).

## ⚡ What it is
Instead of replacing the game engine, GameForge works as a Godot 4 editor plugin communicating with a local Python FastAPI backend. It generates deterministic `.tscn` (scene) and `.gd` (GDScript) files directly into your project.

## 🚀 Quick Start

### 1. Start the Server
Requires Python 3.10+.

```bash
cd installer
python setup.py
./start_server.sh  # or start_server.bat on Windows
```

*Note: The setup script will ask if you want to install Cloud-only or Local AI (AirLLM/PyTorch) dependencies. The Local AI stack is ~2GB+.*

### 2. Configure API Keys
Copy `.env.example` to `ai_backend/.env` and add your keys:
```env
ANTHROPIC_API_KEY=sk-ant-xxxxxxxxxxxxx
```

### 3. Install the Godot Plugin
Copy the `godot_plugin/addons/ai_gameforge/` folder into your Godot 4 project's `res://addons/` directory. Enable the plugin in `Project -> Project Settings -> Plugins`.

The dock will appear on the left. Type: *"Create a 2D Platformer where a character collects coins"* and click Generate.

## 📁 Architecture Overview
* `ai_backend/` - Python FastAPI server, AST Patcher, Deterministic Scene Builder, gdtoolkit validation.
* `godot_plugin/` - GDScript Godot 4 editor plugin containing the chat interface and HTTP client.
* `tests/` - Pytest suite.

See `docs/` for API Reference and Local AI (AirLLM) Setup guidelines.
