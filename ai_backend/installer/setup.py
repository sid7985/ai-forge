#!/usr/bin/env python3
import sys
import subprocess
import os
from pathlib import Path

def main():
    print("=======================================")
    print(" AI GameForge Installer")
    print("=======================================")
    
    backend_dir = Path(__file__).parent.parent / "ai_backend"
    
    # 1. Check Python version
    if sys.version_info < (3, 10):
        print("❌ Error: Python 3.10+ required.")
        sys.exit(1)
        
    print("✅ Python version OK")

    # 2. Virtual Environment
    venv_dir = backend_dir / ".venv"
    if not venv_dir.exists():
        print("📦 Creating virtual environment...")
        subprocess.run([sys.executable, "-m", "venv", str(venv_dir)], check=True)
        
    # Get venv python path
    if os.name == 'nt':
        venv_python = venv_dir / "Scripts" / "python.exe"
    else:
        venv_python = venv_dir / "bin" / "python"

    # 3. Ask about Local AI
    use_local = input("Do you want to install heavy Local AI (AirLLM/PyTorch) packages? (~2GB+ download) [y/N]: ").lower() == 'y'
    req_file = "requirements.txt" if use_local else "requirements-minimal.txt"
    req_path = backend_dir / req_file

    print(f"📥 Installing dependencies from {req_file}...")
    subprocess.run([str(venv_python), "-m", "pip", "install", "-U", "pip"], check=True)
    subprocess.run([str(venv_python), "-m", "pip", "install", "-r", str(req_path)], check=True)

    print("\n✅ Install Complete!")
    print("\nTo start the AI server:")
    if os.name == 'nt':
        print("  cd ai_gameforge\\installer")
        print("  start_server.bat")
    else:
        print("  cd ai_gameforge/installer")
        print("  ./start_server.sh")

if __name__ == "__main__":
    main()
