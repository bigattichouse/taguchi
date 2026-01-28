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
- ⏳ **CLI Tool** (`taguchi`) - Coming Soon
- ✅ **Build System**: Makefile with comprehensive targets
- ✅ **Tests**: Comprehensive unit and integration tests
- ✅ **Memory Safety**: Valgrind clean

## Project Status

### ✅ Complete Modules
- **Infrastructure**: Memory management, error handling, utilities
- **Arrays**: Predefined orthogonal arrays (L4, L8, L9, L16, L27*)
- **Parser**: `.tgu` file parsing and validation
- **Generator**: Mapping factor definitions to experiment runs
- **Serializer**: JSON serialization for bindings
- **Public API**: Complete facade connecting all modules

*Note: L27 orthogonality verification in progress

### 🔄 Active Development
- **Analyzer**: Statistical analysis and main effects calculation

### 📋 Planned Modules
- **CLI**: Command-line interface
- **Language Bindings**: Python, Node.js, etc.

## Quick Start

### Building the Library
```bash
# Build shared and static libraries
make lib

# Run tests
make test

# Build and run static analysis
make check
```

### Basic Usage Example
```c
#include <taguchi.h>
#include <stdio.h>

int main(void) {
    char error[TAGUCHI_ERROR_SIZE];

    // Parse definition
    const char *tgu_content =
        "factors:\n"
        "  cache_size: 64M, 128M, 256M\n"
        "  threads: 2, 4, 8\n"
        "array: L9\n";

    taguchi_experiment_def_t *def = taguchi_parse_definition(tgu_content, error);
    if (!def) {
        fprintf(stderr, "Parse error: %s\n", error);
        return 1;
    }

    // Generate runs
    taguchi_experiment_run_t **runs = NULL;
    size_t count = 0;

    if (taguchi_generate_runs(def, &runs, &count, error) != 0) {
        fprintf(stderr, "Generate error: %s\n", error);
        taguchi_free_definition(def);
        return 1;
    }

    // Print runs
    printf("Generated %zu runs:\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("Run %zu: cache_size=%s, threads=%s\n",
               taguchi_run_get_id(runs[i]),
               taguchi_run_get_value(runs[i], "cache_size"),
               taguchi_run_get_value(runs[i], "threads"));
    }

    // Cleanup
    taguchi_free_runs(runs, count);
    taguchi_free_definition(def);

    return 0;
}
```

### .tgu File Format
```yaml
factors:
  cache_size: 64M, 128M, 256M
  threads: 2, 4, 8
  timeout: 30, 60, 120
array: L9
```

## Architecture

The project follows a modular design with clear separation of concerns:

- **lib/**: Core library modules (no I/O dependencies)
- **cli/**: Command-line specific code (future)
- **include/**: Public API header
- **tests/**: Unit and integration tests
- **bindings/**: Language bindings (future)

## Development

### Contributing
1. Fork the repository
2. Create a feature branch
3. Make changes and add tests
4. Ensure all tests pass: `make test`
5. Submit a pull request

### Testing
Run the complete test suite:
```bash
make test
```

Run static analysis:
```bash
make check
```

## License
MIT License - see LICENSE file for details.

## Current Version
v1.0.0-pre (development in progress)