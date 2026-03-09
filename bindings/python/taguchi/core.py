"""
Core Taguchi library interface using shell commands.
"""

import subprocess
import tempfile
import os
import re
from pathlib import Path
from typing import List, Optional, Dict, Any


class TaguchiError(Exception):
    """Exception raised for Taguchi library errors."""
    pass


class Taguchi:
    """
    Python interface to the Taguchi orthogonal array CLI tool.
    Uses shell commands which is more robust than ctypes for complex structures.
    """
    
    def __init__(self, cli_path: Optional[str] = None):
        self._cli_path = self._find_cli(cli_path)
        self._array_cache: Optional[List[Dict]] = None
        
    def _find_cli(self, cli_path: Optional[str]) -> str:
        """Find the taguchi CLI binary."""
        possible_paths = []
        
        if cli_path:
            possible_paths.append(Path(cli_path))
        
        # Search relative to this file (taguchi_bindings -> ../../../build/taguchi)
        current_dir = Path(__file__).parent
        possible_paths.extend([
            current_dir.parent.parent.parent / "build" / "taguchi",
            current_dir.parent.parent / "build" / "taguchi",
        ])
        
        # System paths
        possible_paths.extend([
            Path("/usr/local/bin/taguchi"),
            Path("/usr/bin/taguchi"),
        ])
        
        for path in possible_paths:
            if path.exists() and os.access(path, os.X_OK):
                return str(path.absolute())
        
        # Try PATH
        result = subprocess.run(["which", "taguchi"], capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.strip()
        
        raise TaguchiError("Could not find taguchi CLI. Build with 'make' first.")
    
    def _run_command(self, args: List[str]) -> str:
        """Run a taguchi command and return output."""
        cmd = [self._cli_path] + args
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            error_msg = result.stderr.strip() or "Unknown error"
            raise TaguchiError(f"Taguchi command failed: {error_msg}")
        return result.stdout
    
    def _get_arrays_info(self) -> List[Dict]:
        """Get cached array information."""
        if self._array_cache is not None:
            return self._array_cache
        
        output = self._run_command(["list-arrays"])
        arrays = []
        
        for line in output.strip().split('\n'):
            match = re.match(r'\s+(L\d+)\s+\(\s*(\d+)\s+runs,\s*(\d+)\s+cols,\s*(\d+)\s+levels\)', line)
            if match:
                arrays.append({
                    'name': match.group(1),
                    'rows': int(match.group(2)),
                    'cols': int(match.group(3)),
                    'levels': int(match.group(4)),
                })
        
        self._array_cache = arrays
        return arrays
    
    def list_arrays(self) -> List[str]:
        """List all available orthogonal arrays."""
        return [a['name'] for a in self._get_arrays_info()]
    
    def get_array_info(self, name: str) -> dict:
        """Get information about an orthogonal array."""
        for array in self._get_arrays_info():
            if array['name'] == name:
                return {'rows': array['rows'], 'cols': array['cols'], 'levels': array['levels']}
        raise TaguchiError(f"Array '{name}' not found")
    
    def suggest_array(self, num_factors: int, max_levels: int) -> str:
        """Suggest optimal array for given factors."""
        arrays = self._get_arrays_info()
        candidates = [a for a in arrays if a['levels'] >= max_levels]
        if not candidates:
            candidates = arrays
        sufficient = [a for a in candidates if a['cols'] >= num_factors]
        if not sufficient:
            sufficient = candidates
        for array in sorted(sufficient, key=lambda a: a['rows']):
            if array['cols'] >= num_factors:
                return array['name']
        return min(sufficient, key=lambda a: a['rows'])['name']
    
    def generate_runs(self, tgu_path: str) -> List[Dict[str, Any]]:
        """
        Generate experiment runs from .tgu file.
        
        Args:
            tgu_path: Path to .tgu file or content string
            
        Returns:
            List of dicts with run_id and factors.
        """
        if os.path.exists(tgu_path):
            output = self._run_command(["generate", tgu_path])
        else:
            with tempfile.NamedTemporaryFile(mode='w', suffix='.tgu', delete=False) as f:
                f.write(tgu_path)
                temp_path = f.name
            try:
                output = self._run_command(["generate", temp_path])
            finally:
                os.unlink(temp_path)
        
        runs = []
        for line in output.strip().split("\n"):
            if line.startswith("Run "):
                parts = line.split(": ", 1)
                if len(parts) < 2:
                    continue
                run_id = int(parts[0].replace("Run ", "").strip())
                factors_str = parts[1]
                factors = {}
                for factor_pair in factors_str.split(", "):
                    if "=" in factor_pair:
                        key, value = factor_pair.split("=", 1)
                        factors[key.strip()] = value.strip()
                runs.append({"run_id": run_id, "factors": factors})
        
        return runs
    
    def analyze(self, tgu_path: str, results_csv: str, metric: str = "response") -> str:
        """Analyze experiment results."""
        return self._run_command(["analyze", tgu_path, results_csv, "--metric", metric])
    
    def effects(self, tgu_path: str, results_csv: str) -> str:
        """Calculate main effects from results."""
        return self._run_command(["effects", tgu_path, results_csv])
