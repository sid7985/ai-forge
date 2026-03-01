# AI GameForge

**"Cursor for Godot: The AI-First Game Engine"**

AI GameForge is a standalone AI-native game development platform forked directly from the Godot 4 C++ source code. 
It replaces the traditional workflow with a multi-agent system (Architect, Scene Designer, Coder, Validator) deeply integrated into the editor UI. 

Go from Natural Language -> Playable 2D/3D Prototype in minutes.

---

## 🚀 Features

- **Embedded AI Dock**: Direct integration into the Godot Editor.
- **5-Agent Generation Pipeline**:
  - `Architect`: Converts prompts to robust JSON schemas.
  - `Scene Designer`: Emits a Godot hierarchy mapped from JSON.
  - `Coder`: Generates targeted GDScripts with full syntax validation.
  - `Validator`: Automatic syntax error loop and patcher using `gdtoolkit`.
  - `Scene Builder`: Deterministically generates `.tscn` files directly to disk.
- **SSE Live Streaming**: Watch the agents think and run operations with live real-time log streaming.
- **Automatic Restore**: Edits failed? GameForge snapshots the project directory before touching anything via the `SnapshotManager`.
- **Multi-Provider Backend**: Use Anthropic Claude 3.5 Sonnet, OpenAI GPT-4o, Google Gemini, or local models via AirLLM.

## 💻 Building from Source

Because AI GameForge is a fork of the engine itself, you build it exactly exactly like you build Godot!

### Requirements
- Python 3.9+ 
- SCons
- A C++ Compiler (GCC, Clang, or MSVC)
- Xcode Command Line tools (macOS only)

### Compiling the Editor
```bash
scons platform=<windows/macos/linux> target=editor
```

The executable will be located in the `bin/` directory.

### Running the Python Backend
The Godot editor will automatically attempt to spawn the Python FastAPI server. However, you can run it manually for debugging:

```bash
cd ai_backend
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python main.py
```

## ⚙️ Configuration & Usage

1. Open the **AI GameForge executable**.
2. Open the **AI GameForge Dock** on the right side of the editor.
3. Switch to the **Settings** tab and enter your preferred **API Key** (e.g., `sk-ant-...`).
4. Select your provider.
5. In the **Generate** tab, select a template (e.g., "2D Platformer") or type your custom game requirements!
6. Click **Generate** and watch the Scene natively auto-open when complete. 
