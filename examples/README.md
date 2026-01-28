# Taguchi Array Tool - Examples

This directory contains examples demonstrating how to use the Taguchi Array Tool from different programming languages.

## Python Examples

### Running Python Examples

1. Make sure the shared library is built:
   ```bash
   cd ..  # From examples/python directory
   make lib
   ```

2. Run the basic examples:
   ```bash
   cd examples/python
   python3 basic_examples.py
   ```

3. Run the advanced examples:
   ```bash
   python3 advanced_examples.py
   ```

### Example Files:
- `basic_examples.py` - Fundamental usage patterns using ctypes
- `advanced_examples.py` - Advanced patterns with a Python wrapper class

## Node.js Examples

### Running Node.js Examples

1. Make sure the shared library is built:
   ```bash
   cd ..  # From examples/nodejs directory
   make lib
   ```

2. Install required dependencies:
   ```bash
   cd examples/nodejs
   npm install ffi-napi ref-napi  # You may need to install these
   ```

3. Run the basic examples:
   ```bash
   node basic_examples.js
   ```

4. Run the advanced examples:
   ```bash
   node advanced_examples.js
   ```

### Example Files:
- `taguchi_binding.js` - Low-level bindings using ffi-napi
- `basic_examples.js` - Fundamental usage patterns
- `advanced_examples.js` - Advanced patterns with workflow classes

## API Usage Patterns

### Python ctypes approach:
- Uses `ctypes` to interface with the C library directly
- Requires careful handling of data types and memory management
- Suitable for performance-critical applications

### Node.js ffi approach:
- Uses `ffi-napi` and `ref-napi` to interface with the C library
- More convenient than manual C++ addons
- Suitable for web services and automation

## Notes

- The examples assume the shared library (`libtaguchi.so`, `libtaguchi.dylib`, or `taguchi.dll`) is available in the parent directory
- Error handling is demonstrated in all examples
- Memory management follows the library's requirements (free functions must be called)
- All examples include proper resource cleanup