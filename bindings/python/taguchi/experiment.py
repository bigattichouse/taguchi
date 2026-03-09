"""
High-level Experiment class for Taguchi orthogonal arrays.
"""

import tempfile
import os
from typing import Dict, List, Optional, Any
from .core import Taguchi, TaguchiError


class Experiment:
    """
    High-level interface for designing and running Taguchi experiments.
    """
    
    def __init__(self, array_type: Optional[str] = None):
        self._taguchi = Taguchi()
        self._array_type = array_type
        self._factors: Dict[str, List[str]] = {}
        self._runs: Optional[List[Dict]] = None
        self._tgu_path: Optional[str] = None
        
    def add_factor(self, name: str, levels: List[str]) -> "Experiment":
        """Add a factor with its levels."""
        if self._runs is not None:
            raise TaguchiError("Cannot add factors after runs are generated")
        self._factors[name] = list(levels)
        self._tgu_path = None
        return self
    
    def _generate_tgu(self) -> str:
        """Generate .tgu content string."""
        if not self._factors:
            raise TaguchiError("No factors defined")
        
        lines = ["factors:"]
        for name, levels in self._factors.items():
            levels_str = ", ".join(levels)
            lines.append(f"  {name}: {levels_str}")
        
        if self._array_type:
            lines.append(f"array: {self._array_type}")
        
        return "\n".join(lines)
    
    def _ensure_tgu_file(self) -> str:
        """Ensure .tgu file exists and return its path."""
        if self._tgu_path is None:
            self._initialize()
            fd, path = tempfile.mkstemp(suffix='.tgu')
            with os.fdopen(fd, 'w') as f:
                f.write(self._generate_tgu())
            self._tgu_path = path
        return self._tgu_path
    
    def _initialize(self) -> None:
        """Initialize experiment and determine array type."""
        if self._array_type is None:
            num_factors = len(self._factors)
            max_levels = max(len(levels) for levels in self._factors.values())
            self._array_type = self._taguchi.suggest_array(num_factors, max_levels)
    
    def generate(self) -> List[Dict[str, Any]]:
        """Generate experiment runs."""
        if self._runs is None:
            tgu_path = self._ensure_tgu_file()
            self._runs = self._taguchi.generate_runs(tgu_path)
        return self._runs
    
    @property
    def array_type(self) -> Optional[str]:
        """Get the selected orthogonal array type."""
        if self._array_type is None:
            self._initialize()
        return self._array_type
    
    @property
    def num_runs(self) -> int:
        """Get number of experimental runs."""
        return len(self.generate())
    
    @property
    def factors(self) -> Dict[str, List[str]]:
        """Get defined factors and their levels."""
        return self._factors.copy()
    
    def get_array_info(self) -> Optional[Dict]:
        """Get information about the selected orthogonal array."""
        if self.array_type:
            return self._taguchi.get_array_info(self.array_type)
        return None
    
    def to_tgu(self) -> str:
        """Export experiment definition as .tgu format string."""
        self._initialize()
        return self._generate_tgu()
    
    def save(self, path: str) -> None:
        """Save experiment definition to a .tgu file."""
        self._initialize()
        with open(path, 'w') as f:
            f.write(self._generate_tgu())
    
    @classmethod
    def from_tgu(cls, path: str) -> "Experiment":
        """Create experiment from .tgu file."""
        with open(path, 'r') as f:
            content = f.read()
        
        exp = cls()
        exp._tgu_path = path
        
        factors = {}
        in_factors = False
        for line in content.split('\n'):
            line = line.strip()
            if line.startswith('factors:'):
                in_factors = True
                continue
            if line.startswith('array:'):
                exp._array_type = line.split(':')[1].strip()
                in_factors = False
                continue
            if in_factors and ':' in line:
                name, levels_str = line.split(':', 1)
                levels = [l.strip() for l in levels_str.split(',')]
                factors[name.strip()] = levels
        
        exp._factors = factors
        return exp
    
    def __enter__(self) -> "Experiment":
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        if self._tgu_path and os.path.exists(self._tgu_path):
            os.unlink(self._tgu_path)
            self._tgu_path = None
