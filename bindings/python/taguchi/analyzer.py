"""
Analyzer class for Taguchi experimental results.
"""

import tempfile
import os
import csv
import re
from typing import Dict, List, Optional, Any
from .core import Taguchi, TaguchiError


class Analyzer:
    """Analyze results from Taguchi orthogonal array experiments."""
    
    def __init__(self, experiment: Any, metric_name: str = "response"):
        self._taguchi = Taguchi()
        self._experiment = experiment
        self._metric_name = metric_name
        self._results: Dict[int, float] = {}
        self._effects: Optional[List[Dict]] = None
        self._csv_path: Optional[str] = None
        self._tgu_path: Optional[str] = None
        
    def _ensure_files(self) -> tuple:
        """Ensure results CSV and TGU files exist."""
        if self._csv_path is None:
            fd, path = tempfile.mkstemp(suffix='.csv')
            with os.fdopen(fd, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(['run_id', self._metric_name])
                for run_id, value in sorted(self._results.items()):
                    writer.writerow([run_id, value])
            self._csv_path = path
        
        if self._tgu_path is None:
            self._tgu_path = self._experiment._ensure_tgu_file()
        
        return self._tgu_path, self._csv_path
    
    def add_result(self, run_id: int, value: float) -> "Analyzer":
        """Add a result for a run."""
        self._results[run_id] = value
        self._effects = None
        self._csv_path = None
        return self
    
    def add_results_from_dict(self, results: Dict[int, float]) -> "Analyzer":
        """Add multiple results from a dictionary."""
        for run_id, value in results.items():
            self.add_result(run_id, value)
        return self
    
    def _parse_effects(self, output: str) -> List[Dict]:
        """Parse effects table from CLI output."""
        effects = []
        lines = output.strip().split('\n')
        
        for line in lines:
            # Parse lines like "depth                   0.026   L1=1.050, L2=1.024, L3=1.037"
            # Factor name, range, and level means
            match = re.match(r'\s*(\w+)\s+([\d.]+)\s+(.+)', line)
            if match:
                factor = match.group(1)
                range_val = float(match.group(2))
                means_str = match.group(3)
                
                # Parse level means: "L1=1.050, L2=1.024, L3=1.037"
                level_matches = re.findall(r'L\d+=([\d.]+)', means_str)
                means = [float(m) for m in level_matches]
                
                if means:
                    effects.append({
                        'factor': factor,
                        'range': range_val,
                        'level_means': means,
                    })
        
        return effects
    
    def main_effects(self) -> List[Dict[str, Any]]:
        """Calculate main effects for all factors."""
        if self._effects is None:
            tgu_path, csv_path = self._ensure_files()
            output = self._taguchi.effects(tgu_path, csv_path)
            self._effects = self._parse_effects(output)
        return self._effects
    
    def recommend_optimal(self, higher_is_better: bool = True) -> Dict[str, str]:
        """Recommend optimal factor levels based on main effects."""
        effects = self.main_effects()

        # Use definition order from the experiment — the OA level indices (L1, L2, L3)
        # correspond to the order levels were defined, not alphabetical order.
        factor_levels: Dict[str, List[str]] = self._experiment.factors
        
        optimal = {}
        for effect in effects:
            factor = effect["factor"]
            level_means = effect["level_means"]
            
            if not level_means:
                continue
                
            if higher_is_better:
                best_idx = level_means.index(max(level_means))
            else:
                best_idx = level_means.index(min(level_means))
            
            if factor in factor_levels and best_idx < len(factor_levels[factor]):
                optimal[factor] = factor_levels[factor][best_idx]
        
        return optimal
    
    def get_significant_factors(self, threshold: float = 0.1) -> List[str]:
        """Get factors with significant effects."""
        effects = self.main_effects()
        if not effects:
            return []
        
        max_range = max(e["range"] for e in effects)
        if max_range == 0:
            return []
        
        return [e["factor"] for e in effects if e["range"] >= threshold * max_range]
    
    def summary(self) -> str:
        """Generate a summary of the analysis."""
        effects = self.main_effects()
        optimal = self.recommend_optimal()
        
        lines = [
            "=" * 60,
            f"Taguchi Experiment Analysis: {self._metric_name}",
            "=" * 60,
            "",
            "Main Effects:",
            "-" * 40,
        ]
        
        sorted_effects = sorted(effects, key=lambda e: e["range"], reverse=True)
        
        for effect in sorted_effects:
            factor = effect["factor"]
            range_val = effect["range"]
            level_means = effect["level_means"]
            means_str = ", ".join(f"{m:.4f}" for m in level_means)
            lines.append(f"  {factor:20s} range={range_val:8.4f}  means=[{means_str}]")
        
        lines.extend([
            "",
            "Recommended Optimal Settings:",
            "-" * 40,
        ])
        
        for factor, level in optimal.items():
            lines.append(f"  {factor:20s} -> {level}")
        
        lines.extend(["", "=" * 60])
        
        return "\n".join(lines)
    
    def __enter__(self) -> "Analyzer":
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        if self._csv_path and os.path.exists(self._csv_path):
            os.unlink(self._csv_path)
            self._csv_path = None
