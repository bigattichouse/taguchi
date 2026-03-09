# Taguchi Array Integration for Autoresearch

## Overview

The Taguchi library provides **orthogonal array experimental design** that can significantly improve autoresearch hyperparameter exploration efficiency.

## Quick Start

```bash
# Build the taguchi CLI
cd ~/workspace/taguchi
make

# Copy bindings to autoresearch
cp -r bindings/python/taguchi /path/to/autoresearch/
```

## Usage Example

```python
from taguchi import Experiment, Analyzer

# Define hyperparameters to explore
exp = Experiment()
exp.add_factor("DEPTH", ["6", "8", "10"])
exp.add_factor("MATRIX_LR", ["0.02", "0.04", "0.08"])
exp.add_factor("WEIGHT_DECAY", ["0.1", "0.2", "0.3"])

# Generate runs (auto-selects L9: 9 runs vs 27 full factorial)
runs = exp.generate()
print(f"Using {exp.array_type} array with {exp.num_runs} runs")

# Execute experiments
for run in runs:
    # Modify train.py with run['factors']
    # Run training, collect val_bpb
    pass

# Analyze results
analyzer = Analyzer(exp, metric_name="val_bpb")
# ... add results ...
optimal = analyzer.recommend_optimal(higher_is_better=False)
```

## Efficiency Gains

| Factors | Levels | Full Factorial | Taguchi | Savings |
|---------|--------|---------------|---------|---------|
| 3 | 3 | 27 runs | 9 runs (L9) | 67% |
| 4 | 3 | 81 runs | 9 runs (L9) | 89% |
| 7 | 2 | 128 runs | 8 runs (L8) | 94% |

## Integration Pattern for Autoresearch

See `bindings/python/demo.py` for a working example.
