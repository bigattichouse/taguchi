# Changelog

All notable changes to the Taguchi Array Tool project.

## [v1.0.0] - 2026-01-28
### Added
- **Complete Taguchi Array Library Implementation**
  - Infrastructure module with memory management and error handling
  - Arrays module with all orthogonal arrays (L4, L8, L9, L16, L27) with mathematical verification
  - Parser module for .tgu file format parsing and validation
  - Generator module for mapping factor definitions to experimental runs
  - Serializer module for JSON output for language bindings
  - Analyzer module for statistical analysis and main effects calculation
  - Public API facade connecting all modules with stable interface

- **Command-Line Interface**
  - `generate` command for creating experiment runs from definitions
  - `run` command for executing external scripts for each experimental configuration
  - `validate` command for validating experiment definitions
  - `list-arrays` command for displaying available orthogonal arrays
  - Proper process management with fork/wait for safe execution
  - Environment variable setting for factor values (TAGUCHI_FACTOR=value)

- **API Client Examples**
  - Python examples using ctypes to interface with C library
  - Node.js examples using ffi-napi to interface with C library
  - Basic and advanced usage patterns for both languages
  - Proper error handling and resource management demonstrations
  - File-based and programmatic workflow examples
  - Comprehensive documentation for API usage

### Changed
- All modules now fully integrated and working together
- Memory safety improvements throughout codebase
- Comprehensive error handling with contextual messages
- Proper resource management with cleanup functions
- API consistency across all modules

### Fixed
- L27 orthogonal array mathematical construction with proper verification
- Parser robustness for edge cases and malformed input
- CLI command execution with proper argument passing
- Memory management to eliminate all leaks

### Removed
- Temporary workarounds and placeholder implementations
- Unnecessary dependencies and external requirements

## [v0.9.0] - Prior Development Milestones
### Added
- Initial project structure and build system
- Core infrastructure modules (memory management, utilities)
- Orthogonal array definitions and basic operations
- API design and public header specification
- Basic test framework and initial tests

[v1.0.0]: https://github.com/yourusername/taguchi/releases/tag/v1.0.0