import structlog
from typing import Optional
import asyncio
from concurrent.futures import ThreadPoolExecutor
from .router import AIProvider
from ..config import config

logger = structlog.get_logger()

try:
    from airllm import AutoModel
    import torch
    AIRLLM_AVAILABLE = True
except ImportError:
    AIRLLM_AVAILABLE = False


class AirLLMProvider(AIProvider):
    """
    AirLLM local provider - runs 70B models on 4GB GPU
    via layer-wise disk offloading.
    """

    def __init__(self):
        if not AIRLLM_AVAILABLE:
            raise ImportError(
                "AirLLM is not installed. Run `pip install airllm torch transformers`."
            )
            
        self.model_id = config.local_model_id
        self.compression = config.local_compression
        self.layer_shards_path = config.local_layer_shards_path
        self.hf_token = config.hf_token
        self.cpu_only = config.cpu_only
        
        # Load model lazily on first request to prevent slow server startup
        self.model = None
        self._executor = ThreadPoolExecutor(max_workers=1)

    def _load_model(self):
        if self.model is not None:
            return

        logger.info(f"Loading local model {self.model_id} via AirLLM...")
        logger.info(f"Compression: {self.compression}")
        
        device = "cpu" if self.cpu_only or not torch.cuda.is_available() else "cuda"
        logger.info(f"Using device: {device}")

        # AirLLM handles device mapping internally, but handles compression natively
        self.model = AutoModel.from_pretrained(
            self.model_id,
            compression=self.compression,
            layer_shards_saving_path=self.layer_shards_path,
            hf_token=self.hf_token,
            prefetching=True,       # Overlap disk load + compute
            delete_original=False,  # Keep original weights safe
        )
        logger.info("Local model loaded successfully.")

    def _generate_sync(self, prompt: str, max_new_tokens: int, temperature: float) -> str:
        """Synchronous generation running in a thread pool to avoid blocking FastAPI."""
        self._load_model()
        
        MAX_LENGTH = 4096
        # Prepend system instruction to the prompt based on what model expects
        # For Llama-3 standard prompt format
        full_prompt = f"<|begin_of_text|><|start_header_id|>system<|end_header_id|>\nYou are an expert Godot 4 AI game developer. Reply ONLY with the requested JSON. No markdown formatting, no explanations.<|eot_id|><|start_header_id|>user<|end_header_id|>\n{prompt}<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n"

        input_tokens = self.model.tokenizer(
            [full_prompt],
            return_tensors="pt",
            return_attention_mask=False,
            truncation=True,
            max_length=MAX_LENGTH,
            padding=False,
        )

        device = "cuda" if torch.cuda.is_available() and not self.cpu_only else "cpu"
        input_ids = input_tokens["input_ids"].to(device)

        logger.info(f"Starting local inference for {input_ids.shape[1]} input tokens...")
        
        output = self.model.generate(
            input_ids,
            max_new_tokens=max_new_tokens,
            use_cache=True,
            return_dict_in_generate=True,
            temperature=temperature,
            do_sample=temperature > 0,
        )

        result = self.model.tokenizer.decode(
            output.sequences[0][input_ids.shape[1]:], # Only return new tokens
            skip_special_tokens=True
        )
        return result

    async def generate(
        self,
        prompt: str,
        max_new_tokens: int = 4096,
        temperature: float = 0.2,
    ) -> str:
        # AirLLM is synchronous blocking, so run it in a thread pool
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(
            self._executor, 
            self._generate_sync, 
            prompt, 
            max_new_tokens, 
            temperature
        )
