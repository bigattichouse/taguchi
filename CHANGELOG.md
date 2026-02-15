# Changelog

All notable changes to the Taguchi Array Tool project.

## [v1.1.0] - 2026-02-15
### Added
- **Large Orthogonal Arrays**: L81 (81 runs, 40 columns) and L243 (243 runs, 121 columns) via GF(3) algorithmic generation
- **Column Pairing**: Multi-level factors (4-27 levels) automatically use paired/tripled OA columns
- **Mixed-Level Support**: Factors with different level counts in the same experiment via modular wrapping
- **Main Effects Analysis**: Full working `calculate_main_effects()` using actual OA design mapping
- **Optimal Recommendations**: `recommend_optimal_levels()` for both maximize and minimize objectives
- **CLI `analyze` command**: Full analysis with main effects table and optimal configuration
- **CLI `effects` command**: Display main effects table from results CSV
- **CSV Results Parsing**: `run_id,response` format with header and comment support
- **Effects JSON Serialization**: `taguchi_effects_to_json()` for language bindings
- **Analyzer Tests**: 10 new tests covering result sets, main effects, recommendations, paired columns
- **Generation Tests**: 16 new tests for column pairing, triple pairing, mixed-level balance, L243

### Changed
- L27 now generated algorithmically via GF(3)^3 (was buggy hardcoded data with 450 orthogonality failures)
- `list-arrays` CLI now shows rows, columns, and levels for each array
- `MAX_LEVELS` increased from 5 to 27 (supports up to 3^3 levels via triple-column pairing)
- `MAX_FACTORS` increased from 16 to 41 (L81 has 40 columns)
- `taguchi_create_result_set()` cleaned up (was double-create/free mess)
- All analysis API stubs in `taguchi.c` replaced with working implementations
- Column ordering in GF(3) generator places unit vectors first for linear independence

### Fixed
- L27 orthogonal array data (replaced with verified GF(3)^3 generation)
- Analyzer now uses actual OA design mapping instead of placeholder `(run_id-1) % level_count`
- Triple column pairing correctness (column reordering ensures linear independence)

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