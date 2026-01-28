"""
Advanced Python Examples for Taguchi Array Tool

This demonstrates more advanced features of the Taguchi Array Tool API.
"""

import ctypes
import json
import os
import tempfile
import subprocess
from pathlib import Path


class TaguchiAPI:
    """Python wrapper class for the Taguchi C API"""
    
    def __init__(self, lib_path=None):
        """Initialize the Taguchi API wrapper"""
        if lib_path is None:
            lib_path = Path(__file__).parent.parent.parent / "libtaguchi.so"
        
        if not lib_path.exists():
            # Try common locations
            lib_path = Path(__file__).parent.parent.parent / "libtaguchi.so"
            if not lib_path.exists():
                raise FileNotFoundError(f"Could not find libtaguchi.so at {lib_path}")
        
        self.lib = ctypes.CDLL(str(lib_path))
        self.TAGUCHI_ERROR_SIZE = 256
        
        # Define handle types
        class ExperimentDefStruct(ctypes.Structure):
            pass
        
        class ExperimentRunStruct(ctypes.Structure):
            pass
        
        class ResultSetStruct(ctypes.Structure):
            pass
        
        class MainEffectStruct(ctypes.Structure):
            pass
        
        self.ExperimentDef = ExperimentDefStruct
        self.ExperimentRun = ExperimentRunStruct
        self.ResultSet = ResultSetStruct
        self.MainEffect = MainEffectStruct
        
        # Setup function signatures
        self._setup_function_signatures()
    
    def _setup_function_signatures(self):
        """Setup ctypes function signatures"""
        # Parse definition
        self.lib.taguchi_parse_definition.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self.lib.taguchi_parse_definition.restype = ctypes.POINTER(self.ExperimentDef)
        
        # Free definition
        self.lib.taguchi_free_definition.argtypes = [ctypes.POINTER(self.ExperimentDef)]
        self.lib.taguchi_free_definition.restype = None
        
        # Validate definition
        self.lib.taguchi_validate_definition.argtypes = [ctypes.POINTER(self.ExperimentDef), ctypes.c_char_p]
        self.lib.taguchi_validate_definition.restype = ctypes.c_bool
        
        # Generate runs
        self.lib.taguchi_generate_runs.argtypes = [
            ctypes.POINTER(self.ExperimentDef),
            ctypes.POINTER(ctypes.POINTER(ctypes.POINTER(self.ExperimentRun))),
            ctypes.POINTER(ctypes.c_size_t),
            ctypes.c_char_p
        ]
        self.lib.taguchi_generate_runs.restype = ctypes.c_int
        
        # Run functions
        self.lib.taguchi_run_get_value.argtypes = [ctypes.POINTER(self.ExperimentRun), ctypes.c_char_p]
        self.lib.taguchi_run_get_value.restype = ctypes.c_char_p
        
        self.lib.taguchi_run_get_id.argtypes = [ctypes.POINTER(self.ExperimentRun)]
        self.lib.taguchi_run_get_id.restype = ctypes.c_size_t
        
        self.lib.taguchi_free_runs.argtypes = [ctypes.POINTER(ctypes.POINTER(self.ExperimentRun)), ctypes.c_size_t]
        self.lib.taguchi_free_runs.restype = None
        
        # Array functions
        self.lib.taguchi_list_arrays.argtypes = []
        self.lib.taguchi_list_arrays.restype = ctypes.POINTER(ctypes.c_char_p)
    
    def list_arrays(self):
        """Get list of available orthogonal arrays"""
        array_names_ptr = self.lib.taguchi_list_arrays()
        
        array_names = []
        i = 0
        while True:
            name_ptr = ctypes.cast(array_names_ptr[i], ctypes.c_char_p)
            if not name_ptr.value:
                break
            array_names.append(name_ptr.value.decode('utf-8'))
            i += 1
            
        return array_names
    
    def parse_definition(self, content):
        """Parse an experiment definition from string content"""
        error_buf = ctypes.create_string_buffer(self.TAGUCHI_ERROR_SIZE)
        def_ptr = self.lib.taguchi_parse_definition(content.encode('utf-8'), error_buf)
        
        if not def_ptr:
            raise Exception(f"Parse error: {error_buf.value.decode('utf-8')}")
        
        return def_ptr, error_buf
    
    def generate_runs(self, def_ptr):
        """Generate experiment runs from definition"""
        runs_ptr = ctypes.POINTER(ctypes.POINTER(self.ExperimentRun))()
        count = ctypes.c_size_t()
        error_buf = ctypes.create_string_buffer(self.TAGUCHI_ERROR_SIZE)
        
        result = self.lib.taguchi_generate_runs(def_ptr, ctypes.byref(runs_ptr), ctypes.byref(count), error_buf)
        
        if result != 0:
            raise Exception(f"Generate error: {error_buf.value.decode('utf-8')}")
        
        return runs_ptr, count.value
    
    def get_run_details(self, run_ptr):
        """Extract details from a run pointer"""
        run_details = {}
        run_details['id'] = self.lib.taguchi_run_get_id(run_ptr)
        return run_details


def example_advanced_usage():
    """Advanced usage example with the wrapper class"""
    print("=== Advanced Python Example: Wrapper Class Usage ===")
    
    try:
        api = TaguchiAPI()
        
        # List available arrays
        arrays = api.list_arrays()
        print(f"Available arrays: {', '.join(arrays)}")
        
        # Define a more complex experiment
        experiment_content = '''
factors:
  cache_size: 64M, 128M, 256M
  threads: 2, 4, 8
  algorithm: algo1, algo2, algo3
array: L9
'''
        
        print("\nParsing experiment definition...")
        def_ptr, error_buf = api.parse_definition(experiment_content)
        print("Successfully parsed!")
        
        print("\nGenerating runs...")
        runs_ptr, count = api.generate_runs(def_ptr)
        print(f"Generated {count} runs for 3-factor experiment")
        
        # Display run information
        print("\nGenerated runs:")
        for i in range(min(5, count)):  # Show first 5 runs
            run_ptr = runs_ptr[i]
            details = api.get_run_details(run_ptr)
            print(f"  Run {details['id']}: details available")
        
        # Show more details if there are more runs
        if count > 5:
            print(f"  ... and {count - 5} more runs")
        
        # Cleanup
        api.lib.taguchi_free_runs(runs_ptr, count)
        api.lib.taguchi_free_definition(def_ptr)
        
    except Exception as e:
        print(f"Error in advanced example: {e}")


def example_from_file():
    """Example working with .tgu files"""
    print("\n=== Advanced Python Example: File-Based Workflow ===")
    
    # Create a temporary .tgu file for testing
    tgu_content = '''factors:
  optimization_level: -O0, -O1, -O2, -O3
  compiler: gcc, clang, icc
array: L9
'''
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.tgu', delete=False) as f:
        f.write(tgu_content)
        temp_filename = f.name
    
    try:
        api = TaguchiAPI()
        
        # Read and parse the file
        with open(temp_filename, 'r') as f:
            content = f.read()
        
        print(f"Reading experiment definition from: {temp_filename}")
        def_ptr, _ = api.parse_definition(content)
        print("Successfully loaded experiment definition from file")
        
        # Generate runs
        runs_ptr, count = api.generate_runs(def_ptr)
        print(f"Generated {count} experiment runs")
        
        # Cleanup
        api.lib.taguchi_free_runs(runs_ptr, count)
        api.lib.taguchi_free_definition(def_ptr)
        
    except Exception as e:
        print(f"Error in file example: {e}")
    finally:
        # Clean up temp file
        os.unlink(temp_filename)


if __name__ == "__main__":
    print("Advanced Taguchi Array Tool - Python Examples\n")
    
    example_advanced_usage()
    example_from_file()
    
    print("\nAdvanced Python examples completed!")
    print("\nThe wrapper class provides a more Python-friendly interface to the C library.")
    print("This approach makes it easier to handle complex Taguchi experiments in Python.")