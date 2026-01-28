# Python Examples - Taguchi Array Tool

Examples for integrating with the Taguchi Array Tool from Python using ctypes.

## Available Examples

### `basic_examples.py`
- Direct ctypes interface to C library functions
- Demonstrates basic operations: list arrays, parse definitions, generate runs
- Shows proper error handling and memory management
- Minimal abstraction layer, direct API calls

### `advanced_examples.py` 
- Higher-level Pythonic wrapper class around C library
- Context managers for automatic resource management
- More intuitive interface with Python conventions
- Error handling with proper exception propagation

## Requirements

- Python 3.x
- C library built (libtaguchi.so, libtaguchi.dylib, or taguchi.dll)
- Access to ctypes library (part of standard library)

## Running Examples

```bash
# Ensure you're in the examples/python directory
cd examples/python

# Run basic examples
python3 basic_examples.py

# Run advanced examples  
python3 advanced_examples.py
```

## Python Integration Patterns

### Direct ctypes Approach
```python
import ctypes

# Load library
lib = ctypes.CDLL('./libtaguchi.so')

# Setup function signatures
lib.taguchi_list_arrays.restype = ctypes.POINTER(ctypes.c_char_p)

# Call functions directly
arrays_ptr = lib.taguchi_list_arrays()
```

### Wrapper Class Approach
```python
from taguchi_python import TaguchiExperiment

# Clean, Pythonic interface
with TaguchiExperiment('experiment.tgu') as exp:
    runs = exp.generate_runs()
    results = analyze_runs(runs)
```

## Best Practices

1. **Error Handling**: Always check for null pointers and error codes
2. **Memory Management**: Use context managers or try/finally for cleanup
3. **Type Safety**: Define proper function signatures to prevent crashes
4. **String Handling**: Properly encode/decode strings between Python and C

## Development Notes

- The ctypes interface enables direct access to all C functions
- Wrapper classes provide more Pythonic interfaces
- Consider using `cffi` for improved maintainability
- Error conditions should raise Python exceptions, not return error codes