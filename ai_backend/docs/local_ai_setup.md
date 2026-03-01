# Local AI Setup using AirLLM

AI GameForge allows complete offline generation using AirLLM. AirLLM uses layer-wise sharding to run massive models (like a 70B parameter Llama3) on average consumer hardware.

## How it works
Normally, a 70B model requires 140GB+ of VRAM to load. AirLLM stores the model on disk (prefer NVMe SSD) and loads individual transformer layers into VRAM, computes, and discards them. Peak VRAM never exceeds ~4GB.

## System Requirements
- **VRAM:** 4GB minimum. 
- **Storage:** NVMe SSD highly recommended. Since weights are transferred from disk to VRAM during inference, HDD will result in very slow generation (2-3x penalty).
- **RAM:** 16GB minimum (32GB recommended for caching).

## 1. Install Dependencies
Run the install script (`setup.py`) and select `Y` when asked to install heavy Local AI packages. This will pull PyTorch, Accelerate, BitsAndBytes, and AirLLM.

## 2. Download Model
In Godot, open the AI GameForge Dock settings and select **Local AirLLM**.
Select your model ID. 
- Fast/Lightweight: `mistralai/Mistral-7B-Instruct-v0.1` (~10 seconds/gen)
- Deep/Slower: `meta-llama/Llama-3.1-70B-Instruct` (~60 seconds/gen)

*Note: For gated models like Llama3, you must create a HuggingFace account, accept the terms on the model page, and generate a User Access Token. Provide this token in the settings.*

## 3. Choose Compression
4-bit compression is strongly recommended. It yields a 3x speedup with virtually indistinguishable quality difference for code generation tasks.
