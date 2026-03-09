"""
Taguchi Array Tool - Python Bindings

A Python interface to the Taguchi orthogonal array library for design of experiments.

Usage:
    from taguchi import Experiment, Analyzer
    
    exp = Experiment()
    exp.add_factor("temp", ["350F", "375F", "400F"])
    runs = exp.generate()
"""

from .core import Taguchi, TaguchiError
from .experiment import Experiment
from .analyzer import Analyzer

__version__ = "1.4.0"
__all__ = ["Taguchi", "TaguchiError", "Experiment", "Analyzer"]
