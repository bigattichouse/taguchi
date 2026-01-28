# Taguchi Array Tool

A robust, Unix-philosophy command-line tool for designing and analyzing experiments using Taguchi orthogonal arrays. The tool is small, fast, composable, and follows strict POSIX conventions. The core is built as a shared library to enable bindings for Python, Node.js, and other languages.

## Features

- **Orthogonal Array Support**: L4, L8, L9, L16, L27
- **YAML-like Configuration**: Human-readable `.tgu` experiment definition files
- **JSON Serialization**: Output for language bindings and external tools
- **Memory Safe**: No leaks, bounds checking, safe string handling
- **Cross-Platform**: Pure C99 with POSIX libc
- **Extensible**: Library-first design for maximum reusability

## Build Status

- ✅ **Shared Library** (`libtaguchi.so` / `libtaguchi.dylib` / `taguchi.dll`)
- ✅ **Static Library** (`libtaguchi.a`)
- ✅ **CLI Tool** (`taguchi`) - Complete
- ✅ **Build System**: Makefile with comprehensive targets
- ✅ **Tests**: Comprehensive unit and integration tests
- ✅ **Memory Safety**: Valgrind clean

## 🎯 **Project Status: COMPLETE** 

The Taguchi Array Tool has been successfully implemented as per the original specifications. All core functionality is complete and working with production-quality code.

### ✅ **Core Modules Delivered**
- **Infrastructure**: Memory management, error handling, utilities
- **Arrays**: All orthogonal arrays (L4, L8, L9, L16, L27) with verified orthogonality
- **Parser**: `.tgu` file parsing and validation
- **Generator**: Mapping factor definitions to experiment runs  
- **Serializer**: JSON serialization for language bindings
- **Analyzer**: Statistical analysis and main effects calculation
- **Public API**: Complete facade connecting all modules
- **CLI**: Command-line interface with run, generate, validate, list-arrays commands
- **Examples**: Python and Node.js API client examples with comprehensive documentation

*Note: L27 orthogonality is fully implemented and verified

## 🏗️ **Architecture Overview**

The system consists of:
- **Core C Library**: High-performance, memory-safe implementation
- **Command-Line Interface**: Unix-style tool for direct usage
- **Foreign Function Interfaces**: Examples for Python (ctypes) and Node.js (ffi-napi)
- **Comprehensive Testing**: All modules validated with memory leak detection

### 🚀 **Getting Started**

#### **Build and Install**
```bash
make lib          # Build shared and static libraries
make cli          # Build command-line tool
make test         # Run comprehensive test suite
make check        # Run static analysis and valgrind
```

#### **Command-Line Usage**
```bash
# Create experiment definition file (experiment.tgu)
factors:
  cache_size: 64M, 128M, 256M
  threads: 2, 4, 8
  timeout: 30, 60, 120
array: L9

# Generate experiment runs
./taguchi generate experiment.tgu

# Execute experiments with external script
./taguchi run experiment.tgu "./my_test.sh"

# Validate experiment definition
./taguchi validate experiment.tgu

# List available arrays
./taguchi list-arrays
```

#### **API Client Usage**
See the [examples/](examples/) directory for Python and Node.js integration examples with comprehensive documentation.

### 🧪 **Quality Assurance**
- ✅ All tests pass including valgrind memory leak checks
- ✅ Clean static analysis with -Wall -Wextra -Werror -pedantic
- ✅ Memory-safe implementation with proper resource management
- ✅ Comprehensive error handling with contextual messages
- ✅ API consistency across languages
- ✅ Unix philosophy compliance (small, fast, composable)

### 📚 **Documentation**
- [API Reference](docs/API.md) - Public API documentation
- [Design Document](docs/DESIGN.md) - Architecture and implementation details  
- [Examples](examples/) - Language-specific usage examples
- [Project Summary](PROJECT_SUMMARY.md) - Complete project overview

### 🔧 **Advanced Usage Examples**

#### **Python Integration**
```python
import ctypes
# See examples/python/ for complete usage patterns
```

#### **Node.js Integration**
```javascript
const ffi = require('ffi-napi');
// See examples/nodejs/ for complete usage patterns
```

The examples demonstrate proper FFI usage, error handling, and resource management for both languages.

### 🔄 **Active Development**
- **Analysis**: Advanced statistical features (ANOVA, confidence intervals, prediction models)
- **Language Bindings**: Production-quality Python/Node.js packages

### 📋 **Planned Enhancements**
- **Additional Arrays**: Larger orthogonal arrays and custom construction algorithms
- **Result Analysis**: Advanced statistical analysis and visualization tools
- **Optimization**: Advanced parameter optimization and machine learning integration
- **Web Interface**: Browser-based experiment designer (separate project)

### 🎯 **Original Specifications Fulfilled**
1. ✅ Complete C library with safe memory management
2. ✅ All specified orthogonal arrays (L4-L27)
3. ✅ YAML-like .tgu file format support
4. ✅ Complete CLI with all specified commands
5. ✅ Language binding examples for Python and Node.js
6. ✅ Comprehensive test suite with memory safety
7. ✅ Unix philosophy compliance
8. ✅ Production-quality code with documentation

### 🤝 **Contributing**
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass (`make test`)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

### 📄 **License**
MIT License - see the [LICENSE](LICENSE) file for details.

### 🙏 **Acknowledgments**
- Based on Genichi Taguchi's robust parameter design methodology
- Inspired by Unix philosophy and modern C engineering practices
- Built with attention to memory safety and performance best practices