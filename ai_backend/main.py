import json
import uuid
import asyncio
from fastapi import FastAPI, HTTPException, BackgroundTasks
from fastapi.responses import StreamingResponse
from fastapi.middleware.cors import CORSMiddleware
import structlog
import uvicorn
from contextlib import asynccontextmanager
from typing import Dict, Any

from config import config
from schema.requests import GenerateRequest, RefineRequest
from providers.router import ProviderRouter, ProviderConfig, AIProviderType
from agents.pipeline import AgentPipeline

logger = structlog.get_logger()

# Global state
router = None
sessions: Dict[str, asyncio.Queue] = {}

@asynccontextmanager
async def lifespan(app: FastAPI):
    global router
    logger.info("Initializing AI GameForge Server...")
    try:
        primary = AIProviderType(config.primary_provider)
        fallback = AIProviderType(config.fallback_provider) if config.fallback_provider else None
        p_config = ProviderConfig(primary=primary, fallback=fallback)
        router = ProviderRouter(p_config)
    except ValueError:
        logger.warning(f"Invalid primary provider configured: {config.primary_provider}. Using Anthropic.")
        p_config = ProviderConfig(primary=AIProviderType.ANTHROPIC)
        router = ProviderRouter(p_config)
    yield

app = FastAPI(title="AI GameForge Backend", version="1.0.0", lifespan=lifespan)
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_credentials=True, allow_methods=["*"], allow_headers=["*"])

async def run_generation_task(session_id: str, prompt: str, project_root: str):
    queue = sessions[session_id]
    try:
        pipeline = AgentPipeline(router.get_provider(None), project_root)
        async for chunk in pipeline.generate_game(prompt):
            await queue.put(chunk)
            
        await queue.put(None) # EOF
    except Exception as e:
        logger.error(f"Generation task failed: {e}")
        await queue.put(f"data: {json.dumps({'step': 'Error', 'message': str(e), 'status': 3})}\n\n")
        await queue.put(None)

@app.post("/generate")
async def generate_game(req: GenerateRequest, background_tasks: BackgroundTasks):
    session_id = str(uuid.uuid4())
    sessions[session_id] = asyncio.Queue()
    
    background_tasks.add_task(run_generation_task, session_id, req.prompt, req.project_root)
    return {"status": "started", "session_id": session_id}

@app.get("/stream/{session_id}")
async def stream_progress(session_id: str):
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
        
    async def sse_generator():
        queue = sessions[session_id]
        while True:
            data = await queue.get()
            if data is None:
                break
            yield data
        if session_id in sessions:
            del sessions[session_id]

    return StreamingResponse(sse_generator(), media_type="text/event-stream")

from schema.requests import RestoreRequest
from snapshot.snapshot_manager import SnapshotManager

@app.post("/restore")
async def restore_snapshot(req: RestoreRequest):
    manager = SnapshotManager(req.project_root)
    success = manager.restore_snapshot(req.timestamp)
    if success:
        return {"status": "ok", "message": f"Restored {req.timestamp}"}
    else:
        raise HTTPException(status_code=404, detail="Restore failed")

@app.get("/health")
async def health():
    return {"status": "ok", "version": "1.0.0"}

if __name__ == "__main__":
    uvicorn.run("main:app", host=config.host, port=config.port, log_level=config.log_level)
