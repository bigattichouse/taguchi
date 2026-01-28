# Taguchi Array Tool - Design Document

## Current Architecture State

### Completed Components

#### 1. Core Infrastructure
- ✅ **Build System**: Makefile with support for shared/static libraries and CLI
- ✅ **Configuration**: Centralized constants in `config.h`
- ✅ **Utilities**: Memory management, string handling in `utils.c/h`
- ✅ **Test Framework**: Comprehensive unit testing infrastructure

#### 2. Array Module
- ✅ **L4 (2^3)**: Verified orthogonal array
- ✅ **L8 (2^7)**: Verified orthogonal array  
- ✅ **L9 (3^4)**: Verified orthogonal array
- ✅ **L16 (2^15)**: Verified orthogonal array
- ⚠️ **L27 (3^13)**: Exists but orthogonality verification pending mathematical construction
- ✅ **API Functions**: Array lookup, listing, info retrieval

#### 3. Parser Module
- ✅ **File Format**: YAML-like parsing for `.tgu` files
- ✅ **Factor Definitions**: Parse named factors with multiple levels
- ✅ **Validation**: Check factor counts and array compatibility
- ✅ **Error Handling**: Comprehensive error reporting with context

#### 4. Generator Module  
- ✅ **Run Generation**: Map factor definitions to orthogonal array runs
- ✅ **Compatibility Checks**: Verify factor counts vs array capacity
- ✅ **Value Mapping**: Properly map array indices to factor level values
- ✅ **Memory Management**: Proper allocation/deallocation of run structures

#### 5. Serializer Module
- ✅ **JSON Serialization**: Convert experiment runs to JSON format
- ✅ **String Escaping**: Proper JSON string escaping
- ✅ **Memory Management**: Safe allocation/deallocation of JSON strings

#### 6. Public API Facade
- ✅ **Opaque Handles**: Proper encapsulation with opaque types
- ✅ **Function Bridge**: Connect public API to internal implementations
- ✅ **Memory Management**: Proper resource management across API boundaries
- ✅ **Error Propagation**: Consistent error handling pattern

#### 7. Analysis Module (Complete)
- **Status**: ✅ Complete
- **Files**: `analyzer.c/h`
- **Functions**:
  - `taguchi_calculate_main_effects()`: Calculate main effects from experimental results
  - `taguchi_effect_get_range()`: Calculate effect ranges (max-min)
  - `taguchi_recommend_optimal()`: Recommend optimal factor configurations
  - Statistical analysis algorithms for variance, significance testing
  - Result set management and experimental data correlation

### Upcoming Components

#### 8. Results Module (Integrated)
- **Status**: ✅ Integrated into Analysis Module
- **Functions**:
  - `taguchi_create_result_set()`: Manage experimental results
  - `taguchi_add_result()`: Add individual result measurements
  - Result validation and cross-referencing with runs

#### 9. CLI Module (Complete)
- **Status**: ✅ Complete
- **Files**: `src/cli/main.c`
- **Commands**: `generate`, `run`, `validate`, `list-arrays`, `help`, `--version`
- **Features**:
  - Process execution for each experimental run using fork/wait
  - Environment variable setting for factor values (`TAGUCHI_<factor_name>=<value>`)
  - Run ID available as `TAGUCHI_RUN_ID` environment variable
  - External command execution with proper argument passing
  - Error handling and validation

#### 10. API Client Examples (Complete)
- **Status**: ✅ Complete
- **Files**: `examples/python/`, `examples/nodejs/`, `examples/DOCUMENTATION.md`
- **Languages**: Python, Node.js
- **Features**:
  - Python ctypes interface to C library with basic and advanced examples
  - Node.js ffi-napi interface to C library with basic and advanced examples
  - Proper error handling and resource management in both languages
  - File-based and programmatic workflow examples
  - Async execution patterns for Node.js
  - Complete usage demonstrations and comprehensive documentation
  - Workflow classes for complex experiment management
  - Batch processing capabilities for multiple experiments

## Internal Module Interactions

```
Public API (taguchi.h)
    ↓
taguchi.c (facade)
    ↓
┌─────────────────┐    ┌─────────────────┐
│   parser.c/h    │ ←→ │  generator.c/h  │
│  (.tgu parsing) │    │ (run generation)│
└─────────────────┘    └─────────────────┘
    ↑                       ↑
    └───────────────────────┘
             ↓
┌─────────────────┐    ┌─────────────────┐
│   arrays.c/h    │    │serializer.c/h   │
│ (orthogonal     │    │ (JSON output)   │
│  arrays)       │    │                 │
└─────────────────┘    └─────────────────┘
```

## Data Flow Architecture

### 1. Parse Pipeline
```c
input: ".tgu file content" 
    ↓
taguchi_parse_definition() 
    ↓
parser.c::parse_experiment_def_from_string() 
    ↓
ExperimentDef (internal structure)
```

### 2. Generate Pipeline  
```c
input: ExperimentDef (parsed from .tgu)
    ↓
taguchi_generate_runs() 
    ↓
generator.c::generate_experiments() 
    ↓
arrays.c::get_array() + mapping logic
    ↓
ExperimentRun (array of configurations)
```

### 3. Serialize Pipeline
```c
input: ExperimentRun (generated runs)  
    ↓
taguchi_runs_to_json()
    ↓
serializer.c::serialize_runs_to_json()
    ↓
JSON string output
```

## Memory Management Strategy

### Internal Modules
- **Allocation**: `xmalloc()`, `xcalloc()` for safe allocation
- **Deallocation**: Explicit free functions: `free_experiments()`, `free_serialized_string()`
- **Safety**: All allocations check for failure and exit gracefully

### Public API  
- **Resource Ownership**: Clear ownership boundaries between caller/library
- **Allocation Patterns**: Library allocates, client frees (for return values)
- **Cleanup Functions**: Explicit free functions for all allocated resources

## Error Handling Strategy

### Error Propagation Pattern
```c
// Functions return int: 0=success, -1=error
// Error details in dedicated buffer parameter
int function_name(params..., char *error_buf) {
    if (error_condition) {
        set_error(error_buf, "Descriptive error message");
        return -1;
    }
    return 0; // Success
}
```

### Error Context Preservation
- **Function Names**: Included in error messages
- **Parameter Details**: Specific problematic values
- **Validation Errors**: Clear indication of what went wrong

## Testing Strategy Status

### Completed Test Coverage
- ✅ **Unit Tests**: Individual modules tested in isolation
- ✅ **Integration Tests**: Module interactions validated
- ✅ **Memory Tests**: Valgrind-clean operation verified  
- ✅ **Error Conditions**: All error paths tested
- ✅ **Edge Cases**: Boundary conditions validated
- ✅ **Parser Tests**: Various .tgu formats validated

### Testing Framework
- **Test Runner**: Comprehensive test orchestration
- **Assertions**: Rich set of assertion macros
- **Error Reporting**: Clear failure diagnostics
- **Memory Safety**: Automatic leak detection with valgrind

## Development Progress

### Phase Completion Status
- ✅ **Phase 1**: Core Library Infrastructure (utilities, config)
- ✅ **Phase 2**: Parsing and Arrays (partial - L27 verification pending)  
- ✅ **Phase 3**: Generation and Serialization
- 🔄 **Phase 4**: Analysis Module (next priority)
- 📋 **Phase 5**: CLI Implementation (future)
- 📋 **Phase 6**: Language Bindings (future) 
- 📋 **Phase 7**: Polish and Release (future)

### Current Focus: Analysis Module Implementation
- **Priority**: Calculate main effects from experimental results
- **Scope**: Statistical analysis algorithms
- **API**: Complete the results and analysis functions in public header

## Next Development Steps

### Immediate Priorities
1. **Analysis Module** (`analyzer.c/h`) - Statistical calculations
2. **Results Module** - Experimental result management  
3. **Complete API** - Implement remaining public functions

### Architectural Decisions Made
- **Modularity**: Clear separation of concerns between modules
- **Memory Safety**: Comprehensive safe allocation patterns
- **Error Handling**: Consistent error propagation strategy
- **API Design**: Opaque handles for proper encapsulation
- **Testing**: Comprehensive unit and integration testing
- **Portability**: Strict C99/POSIX compliance

### Future Scalability
- **New Arrays**: Easy addition of orthogonal arrays
- **Statistics**: Extensible analysis functionality  
- **I/O Formats**: Multiple serialization options
- **Language Bindings**: Clean FFI boundaries