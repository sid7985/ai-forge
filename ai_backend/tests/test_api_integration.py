import pytest
from httpx import AsyncClient, ASGITransport
import json
from unittest.mock import AsyncMock, patch
from ai_backend.main import app

MOCK_SCENE_SPEC = {
    "scene_name": "main",
    "root_node": {"name": "Main", "type": "Node2D"},
    "scripts": [],
    "connections": [],
    "autoloads": []
}

@pytest.mark.asyncio
async def test_generate_endpoint_returns_scene_spec():
    mock_provider = AsyncMock()
    mock_provider.generate.return_value = json.dumps(MOCK_SCENE_SPEC)

    with patch("ai_backend.main.router") as mock_router:
        mock_router.generate = mock_provider.generate
        
        async with AsyncClient(
            transport=ASGITransport(app=app), base_url="http://test"
        ) as client:
            response = await client.post("/generate", json={
                "prompt": "Create a simple 2D platformer",
                "provider": "cloud",
                "cloud_provider": "claude"
            })
            
    assert response.status_code == 200
    assert "scene_spec" in response.json()
    assert response.json()["scene_spec"]["scene_name"] == "main"

@pytest.mark.asyncio
async def test_health_endpoint():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        response = await client.get("/health")
        
    assert response.status_code == 200
    assert response.json()["status"] == "ok"

@pytest.mark.asyncio
async def test_generate_with_invalid_prompt_returns_422():
    async with AsyncClient(
        transport=ASGITransport(app=app), base_url="http://test"
    ) as client:
        # Missing required 'prompt' field
        response = await client.post("/generate", json={})
        
    assert response.status_code == 422
