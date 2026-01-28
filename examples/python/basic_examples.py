"""
Python Examples for Taguchi Array Tool

These examples demonstrate various ways to use the Taguchi Array Tool library
from Python using ctypes to interface with the C library.
"""

import ctypes
import json
import os
import subprocess
from pathlib import Path

# Load the shared library
lib_path = Path(__file__).parent.parent / "libtaguchi.so"
if not lib_path.exists():
    # Try common locations relative to this file
    lib_path = Path(__file__).parent.parent.parent / "libtaguchi.so"
    if not lib_path.exists():
        print("Warning: libtaguchi.so not found. Building library...")
        subprocess.run(["make", "lib"], cwd=Path(__file__).parent.parent.parent)

try:
    libtaguchi = ctypes.CDLL(str(lib_path.absolute()))
except OSError:
    print(f"Could not load {lib_path}. Make sure the library is built.")
    exit(1)

# Define the error buffer size from the header
TAGUCHI_ERROR_SIZE = 256

# Define opaque handle types
class TaguchiExperimentDef(ctypes.Structure):
    pass

class TaguchiExperimentRun(ctypes.Structure):
    pass

class TaguchiResultSet(ctypes.Structure):
    pass

class TaguchiMainEffect(ctypes.Structure):
    pass

# Set up function return and argument types
libtaguchi.taguchi_parse_definition.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
libtaguchi.taguchi_parse_definition.restype = ctypes.POINTER(TaguchiExperimentDef)

libtaguchi.taguchi_free_definition.argtypes = [ctypes.POINTER(TaguchiExperimentDef)]
libtaguchi.taguchi_free_definition.restype = None

libtaguchi.taguchi_add_factor.argtypes = [
    ctypes.POINTER(TaguchiExperimentDef),
    ctypes.c_char_p,
    ctypes.POINTER(ctypes.c_char_p),
    ctypes.c_size_t,
    ctypes.c_char_p
]
libtaguchi.taguchi_add_factor.restype = ctypes.c_int

libtaguchi.taguchi_validate_definition.argtypes = [
    ctypes.POINTER(TaguchiExperimentDef),
    ctypes.c_char_p
]
libtaguchi.taguchi_validate_definition.restype = ctypes.c_bool

libtaguchi.taguchi_generate_runs.argtypes = [
    ctypes.POINTER(TaguchiExperimentDef),
    ctypes.POINTER(ctypes.POINTER(ctypes.POINTER(TaguchiExperimentRun))),
    ctypes.POINTER(ctypes.c_size_t),
    ctypes.c_char_p
]
libtaguchi.taguchi_generate_runs.restype = ctypes.c_int

libtaguchi.taguchi_run_get_value.argtypes = [
    ctypes.POINTER(TaguchiExperimentRun), 
    ctypes.c_char_p
]
libtaguchi.taguchi_run_get_value.restype = ctypes.c_char_p

libtaguchi.taguchi_run_get_id.argtypes = [ctypes.POINTER(TaguchiExperimentRun)]
libtaguchi.taguchi_run_get_id.restype = ctypes.c_size_t

libtaguchi.taguchi_free_runs.argtypes = [
    ctypes.POINTER(ctypes.POINTER(TaguchiExperimentRun)), 
    ctypes.c_size_t
]
libtaguchi.taguchi_free_runs.restype = None

libtaguchi.taguchi_list_arrays.argtypes = []
libtaguchi.taguchi_list_arrays.restype = ctypes.POINTER(ctypes.c_char_p)

libtaguchi.taguchi_run_get_factor_names.argtypes = [ctypes.POINTER(TaguchiExperimentRun)]
libtaguchi.taguchi_run_get_factor_names.restype = ctypes.POINTER(ctypes.c_char_p)

def example_list_arrays():
    """Example showing how to list available arrays"""
    print("=== Example 1: List Available Arrays ===")
    
    # Get array names
    array_names_ptr = libtaguchi.taguchi_list_arrays()
    
    array_names = []
    i = 0
    while True:
        name_ptr = ctypes.cast(array_names_ptr[i], ctypes.c_char_p)
        if not name_ptr.value:
            break
        array_names.append(name_ptr.value.decode('utf-8'))
        i += 1
        
    print("Available orthogonal arrays:")
    for name in array_names:
        print(f"  {name}")
    print()


def example_parse_and_generate():
    """Example showing how to parse .tgu definitions and generate runs"""
    print("=== Example 2: Parse Definition and Generate Runs ===")
    
    # Example .tgu content
    tgu_content = '''factors:
  cache_size: 64M, 128M, 256M
  threads: 2, 4, 8
array: L9'''
    
    # Parse definition
    error_buf = ctypes.create_string_buffer(TAGUCHI_ERROR_SIZE)
    def_ptr = libtaguchi.taguchi_parse_definition(tgu_content.encode('utf-8'), error_buf)
    
    if not def_ptr:
        print(f"Parse error: {error_buf.value.decode('utf-8')}")
        return
    
    print("Successfully parsed experiment definition")
    
    # Generate runs
    runs_ptr = ctypes.POINTER(ctypes.POINTER(TaguchiExperimentRun))()
    count = ctypes.c_size_t()
    
    result = libtaguchi.taguchi_generate_runs(def_ptr, ctypes.byref(runs_ptr), ctypes.byref(count), error_buf)
    
    if result != 0:
        print(f"Generate error: {error_buf.value.decode('utf-8')}")
        libtaguchi.taguchi_free_definition(def_ptr)
        return
    
    print(f"Generated {count.value} experiment runs:")
    
    # Access runs
    for i in range(count.value):
        run_ptr = runs_ptr[i]
        run_id = libtaguchi.taguchi_run_get_id(run_ptr)
        print(f"  Run {run_id}: ", end="")
        
        # Try to get factor values - since we don't have factor-enumeration functions
        # in the current Python bindings, we'll hardcode for this example
        cache_size = libtaguchi.taguchi_run_get_value(run_ptr, b"cache_size")
        threads = libtaguchi.taguchi_run_get_value(run_ptr, b"threads")
        
        if cache_size and threads:
            print(f"cache_size={cache_size.decode('utf-8')}, threads={threads.decode('utf-8')}")
        else:
            print("values available")
    
    # Cleanup
    libtaguchi.taguchi_free_runs(runs_ptr, count.value)
    libtaguchi.taguchi_free_definition(def_ptr)
    print()


def example_simple_usage():
    """A simple end-to-end example"""
    print("=== Example 3: Simple End-to-End Usage ===")
    
    # Define experiment as string
    content = '''
factors:
  processor_speed: 2.0GHz, 2.5GHz, 3.0GHz  
  memory_size: 4GB, 8GB, 16GB
array: L9
'''
    
    # Parse
    error_buf = ctypes.create_string_buffer(TAGUCHI_ERROR_SIZE)
    def_ptr = libtaguchi.taguchi_parse_definition(content.encode('utf-8'), error_buf)
    
    if not def_ptr:
        print(f"Error: {error_buf.value.decode('utf-8')}")
        return
        
    print("Experiment definition created successfully!")
    
    # Validate
    if libtaguchi.taguchi_validate_definition(def_ptr, error_buf):
        print("Experiment definition is valid!")
    else:
        print(f"Validation error: {error_buf.value.decode('utf-8')}")
    
    # Generate and display
    runs_ptr = ctypes.POINTER(ctypes.POINTER(TaguchiExperimentRun))()
    count = ctypes.c_size_t()
    
    if libtaguchi.taguchi_generate_runs(def_ptr, ctypes.byref(runs_ptr), ctypes.byref(count), error_buf) == 0:
        print(f"Generated {count.value} runs for L9 array")
    else:
        print(f"Run generation failed: {error_buf.value.decode('utf-8')}")
    
    # Cleanup
    if 'runs_ptr' in locals():
        libtaguchi.taguchi_free_runs(runs_ptr, count.value)
    libtaguchi.taguchi_free_definition(def_ptr)
    
    print()


if __name__ == "__main__":
    print("Taguchi Array Tool - Python Examples\n")
    
    example_list_arrays()
    example_parse_and_generate()
    example_simple_usage()
    
    print("All Python examples completed!")
    print("\nNote: This example demonstrates ctypes interface to the C library.")
    print("For production use, consider creating a more Pythonic wrapper class.")