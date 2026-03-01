# Setup Guide

## 1. System Requirements
- Python 3.10+
- Godot 4.3+ (earlier Godot 4 versions may have `gdtoolkit` incompatibilities)
- To run Local AI: NVIDIA GPU with 4GB+ VRAM or Unified Memory Apple Silicon (M1/M2/M3)

## 2. Server Installation

Run the automated installer:
```bash
cd ai_gameforge/installer
python setup.py
```
This will create a virtual environment in `ai_gameforge/ai_backend/.venv` and ask if you want to install the Local AI stack.

## 3. Configuration

Enter the `ai_backend` folder:
```bash
cd ../ai_backend
cp .env.example .env
```

Edit your `.env` file to contain your chosen Cloud API Keys. For Cloud mode, Claude 3.5 Sonnet (`ANTHROPIC_API_KEY`) is the recommended primary provider.

## 4. Run the Server
Use the launcher scripts:
```bash
cd ../installer
./start_server.sh  # macOS/Linux
# OR
start_server.bat   # Windows
```

## 5. Godot Plugin Setup
1. Copy the folder `ai_gameforge/godot_plugin/addons/ai_gameforge` from this repo into your Godot project's `res://addons/ai_gameforge` directory.
2. Open your Godot project.
3. Go to **Project -> Project Settings -> Plugins**.
4. Check "Enable" next to **AI GameForge**.
5. The dock will appear in the bottom-left of the Godot Editor.
