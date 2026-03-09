# Taguchi Python Bindings

Python bindings for the Taguchi orthogonal array tool.

## Installation

The bindings use the taguchi CLI binary. Make sure it's built:

```bash
cd ~/workspace/taguchi
make
```

## Usage

```python
from taguchi import Experiment, Analyzer

# Create experiment
exp = Experiment()
exp.add_factor("DEPTH", ["4", "6", "8"])
exp.add_factor("MATRIX_LR", ["0.02", "0.04", "0.08"])
exp.add_factor("WEIGHT_DECAY", ["0.1", "0.2", "0.3"])

# Generate runs (auto-selects L9 array: 9 runs)
runs = exp.generate()
print(f"Using {exp.array_type} array with {exp.num_runs} runs")

# Execute experiments and collect results
for run in runs:
    print(f"Run {run['run_id']}: {run['factors']}")
    # ... run your experiment ...

# Analyze results
analyzer = Analyzer(exp, metric_name="val_bpb")
analyzer.add_result(run_id=0, value=1.234)
# ... add more results ...

# Get recommendations
optimal = analyzer.recommend_optimal(higher_is_better=False)
print(f"Best config: {optimal}")
```

## API

### Experiment
- `add_factor(name, levels)` - Add a factor with level values
- `generate()` - Generate experiment runs
- `array_type` - Get selected orthogonal array (e.g., "L9")
- `num_runs` - Number of experimental runs

### Analyzer
- `add_result(run_id, value)` - Add a result
- `main_effects()` - Calculate main effects
- `recommend_optimal(higher_is_better)` - Get optimal configuration
- `summary()` - Get formatted analysis summary

## Demo

```bash
PYTHONPATH=. python3 demo.py
```
