import shutil
import logging
from pathlib import Path
from datetime import datetime

logger = logging.getLogger(__name__)

class SnapshotManager:
    def __init__(self, project_root: str | Path):
        self.project_root = Path(project_root)
        self.snapshots_dir = self.project_root / ".ai_gameforge" / "snapshots"
        self.snapshots_dir.mkdir(parents=True, exist_ok=True)

    def take_snapshot(self) -> str:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        target_dir = self.snapshots_dir / timestamp
        
        # We only really need to back up scripts, scenes, and project settings
        # for AI generated content
        for d in ["scripts", "scenes"]:
            src = self.project_root / d
            if src.exists():
                shutil.copytree(src, target_dir / d, dirs_exist_ok=True)
                
        project_godot = self.project_root / "project.godot"
        if project_godot.exists():
            shutil.copy2(project_godot, target_dir / "project.godot")
            
        logger.info(f"Snapshot taken: {timestamp}")
        self._prune_snapshots(keep=10)
        return timestamp

    def restore_snapshot(self, timestamp: str) -> bool:
        snapshot_dir = self.snapshots_dir / timestamp
        if not snapshot_dir.exists():
            logger.error(f"Snapshot {timestamp} not found.")
            return False

        try:
            for d in ["scripts", "scenes"]:
                src = snapshot_dir / d
                dest = self.project_root / d
                if src.exists():
                    if dest.exists():
                        shutil.rmtree(dest)
                    shutil.copytree(src, dest)
            
            project_godot = snapshot_dir / "project.godot"
            if project_godot.exists():
                shutil.copy2(project_godot, self.project_root / "project.godot")
                
            logger.info(f"Restored snapshot: {timestamp}")
            return True
        except Exception as e:
            logger.error(f"Failed to restore snapshot: {e}")
            return False

    def _prune_snapshots(self, keep: int = 10):
        snapshots = sorted([d for d in self.snapshots_dir.iterdir() if d.is_dir()])
        while len(snapshots) > keep:
            oldest = snapshots.pop(0)
            shutil.rmtree(oldest)
            logger.debug(f"Pruned old snapshot: {oldest.name}")
