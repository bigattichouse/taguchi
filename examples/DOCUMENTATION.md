# Taguchi Array Tool - API Client Examples Documentation

## Overview

This document describes the Python and Node.js API client examples for the Taguchi Array Tool. These examples demonstrate how to interface with the core C library using Foreign Function Interface (FFI) technologies in higher-level languages.

## Directory Structure

```
examples/
├── README.md                 # Top-level examples documentation
├── python/                   # Python examples
│   ├── basic_examples.py     # Basic usage patterns with ctypes
│   └── advanced_examples.py  # Advanced patterns with wrapper class
└── nodejs/                   # Node.js examples
    ├── taguchi_binding.js    # FFI bindings using ffi-napi
    ├── basic_examples.js     # Basic usage patterns
    └── advanced_examples.js  # Advanced patterns with workflow classes
```

## Python Examples

### Basic Examples (`basic_examples.py`)

Demonstrates fundamental usage using Python's `ctypes` module to interface directly with the C library:

- **Direct Library Calls**: Shows how to load the shared library and call functions directly
- **Data Type Mapping**: Demonstrates mapping C data types to Python equivalents
- **Error Handling**: Shows how to properly handle error buffers from the C library
- **Memory Management**: Demonstrates proper resource cleanup

### Advanced Examples (`advanced_examples.py`)

Provides more sophisticated usage with a higher-level wrapper class:

- **Wrapper Class**: `TaguchiAPI` class encapsulates FFI complexity
- **Resource Management**: Automatic cleanup of library resources
- **Exception Handling**: Proper error propagation from C to Python
- **Workflow Classes**: Advanced patterns for complex workflows

## Node.js Examples

### FFI Bindings (`taguchi_binding.js`)

Provides the low-level interface between JavaScript and the C library:

- **Function Signatures**: Defines C function signatures for FFI
- **Type Definitions**: Specifies parameter and return types for each function
- **Memory Management**: Handles pointer management and buffer allocation
- **Error Handling**: Proper error propagation patterns

### Basic Examples (`basic_examples.js`)

Shows fundamental usage patterns:

- **Library Integration**: Loading and interfacing with the C library
- **Asynchronous Patterns**: Demonstrates async execution of experiments
- **Error Handling**: Proper error handling in JavaScript environment
- **Resource Management**: Cleanup and resource management

### Advanced Examples (`advanced_examples.js`)

Provides sophisticated usage patterns:

- **Workflow Classes**: `TaguchiWorkflow` class abstracts common patterns
- **Batch Processing**: Demonstrates processing multiple experiments
- **Result Analysis**: Basic result analysis and optimization
- **File Integration**: Loading experiments from .tgu files

## API Usage Patterns

### Python ctypes Pattern

```python
import ctypes

# Load library
libtaguchi = ctypes.CDLL("libtaguchi.so")

# Setup function signatures
libtaguchi.taguchi_parse_definition.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
libtaguchi.taguchi_parse_definition.restype = ctypes.POINTER(TaguchiExperimentDef)

# Call function
error_buf = ctypes.create_string_buffer(TAGUCHI_ERROR_SIZE)
def_ptr = libtaguchi.taguchi_parse_definition(content.encode('utf-8'), error_buf)
```

### Node.js ffi-napi Pattern

```javascript
const ffi = require('ffi-napi');

const taguchiLib = ffi.Library('libtaguchi.so', {
  'taguchi_parse_definition': ['pointer', ['string', 'pointer']],
  'taguchi_free_definition': ['void', ['pointer']]
});

const defHandle = taguchiLib.taguchi_parse_definition(content, errorBuf);
```

## Best Practices Demonstrated

### Error Handling
- Always check return values from library functions
- Use error buffers to get detailed error messages
- Proper exception handling in high-level languages

### Memory Management
- Always call the corresponding free function for each allocation
- Use try/finally or similar patterns for cleanup
- Proper buffer sizing and null pointer checks

### Resource Management
- Encapsulate resource lifecycle in wrapper classes when appropriate
- Use RAII-style patterns for automatic cleanup
- Handle multiple resources properly

### Type Safety
- Define proper function signatures
- Use appropriate data type conversions
- Handle string encoding/decoding properly

## Integration Notes

### Building Requirements
- The shared library (`libtaguchi.so`, `libtaguchi.dylib`, or `taguchi.dll`) must be available
- Python requires `ctypes` (standard library)
- Node.js requires `ffi-napi` and `ref-napi` packages

### Platform Compatibility
- Linux: `libtaguchi.so`
- macOS: `libtaguchi.dylib` 
- Windows: `taguchi.dll`

### Performance Considerations
- FFI calls have overhead, minimize frequent calls
- Batch operations where possible
- Cache library function references

## Use Cases

These examples are designed to support:

1. **Scientific Computing**: Integration with Python scientific stacks
2. **Automation**: Automated experiment execution via scripts
3. **Web Services**: Integration with Node.js-based services
4. **Data Analysis**: Post-processing of experimental results
5. **CI/CD Pipelines**: Automated experimentation workflows

## Next Steps

### Production Ready Bindings
- Create formal packages (PyPI, npm) for easy installation
- Add comprehensive unit testing for bindings
- Implement additional language bindings (Go, Rust, etc.)
- Create formal documentation and tutorials

### Advanced Features
- Real-time experiment execution with progress tracking
- Advanced statistical analysis functions
- Parallel execution of experiments
- Integration with popular ML frameworks

## Testing the Examples

### Python Examples
```bash
cd examples/python
python3 basic_examples.py
python3 advanced_examples.py
```

### Node.js Examples
```bash
cd examples/nodejs
npm install ffi-napi ref-napi  # if not already installed
node basic_examples.js
node advanced_examples.js
```

## Troubleshooting

### Common Issues

1. **Library Not Found**: Ensure `libtaguchi.so` is in the same directory or in the library path
2. **Missing Dependencies**: Install required FFI packages for your platform
3. **Permission Issues**: Ensure you have execute permissions on the shared library
4. **Architecture Mismatch**: Ensure library architecture matches your Python/Node.js installation

## Security Considerations

- These are FFI bindings to a C library, so follow security best practices for both environments
- Validate all input before passing to the C library
- Be cautious with file I/O and external command execution
- Review the underlying C library for security considerations