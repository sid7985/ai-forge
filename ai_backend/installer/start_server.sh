#!/usr/bin/env bash

# AI GameForge Server Start Script (macOS/Linux)
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BACKEND_DIR="$DIR/../ai_backend"
VENV_PYTHON="$BACKEND_DIR/.venv/bin/python"

if [ ! -f "$VENV_PYTHON" ]; then
    echo "Virtual environment not found. Please run installer/setup.py first."
    exit 1
fi

echo "Starting AI GameForge Server..."
cd "$BACKEND_DIR"
"$VENV_PYTHON" main.py
