@echo off
REM AI GameForge Server Start Script (Windows)

set BACKEND_DIR=%~dp0..\ai_backend
set VENV_PYTHON=%BACKEND_DIR%\.venv\Scripts\python.exe

if not exist "%VENV_PYTHON%" (
    echo Virtual environment not found. Please run installer/setup.py first.
    pause
    exit /b 1
)

echo Starting AI GameForge Server...
cd "%BACKEND_DIR%"
"%VENV_PYTHON%" main.py
pause
