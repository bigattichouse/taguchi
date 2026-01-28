"""
Python Examples Demonstrating Auto-Selection Feature

This example demonstrates the new automatic array selection feature
where if no array is specified, the system finds the optimal array automatically.
"""

import ctypes
import json
import os
from pathlib import Path

# Load Taguchi library
def load_lib():
    lib_paths = [
        "./libtaguchi.so",
        "../libtaguchi.so", 
        "../../libtaguchi.so",
        os.path.join(os.path.dirname(os.path.dirname(__file__)), "libtaguchi.so")
    ]
    
    lib_path = None
    for path in lib_paths:
        if os.path.exists(path):
            lib_path = path
            break
    
    if not lib_path:
        print("Error: Could not find libtaguchi.so library")
        print("Please build the library with 'make lib' first")
        return None
    
    try:
        return ctypes.CDLL(lib_path)
    except OSError as e:
        print(f"Error loading library: {e}")
        return None

def demo_auto_selection():
    print("Python Examples - Auto-Selection Feature Demo")
    print("===========================================")
    
    lib = load_lib()
    if not lib:
        return
    
    # Define function signatures
    TAGUCHI_ERROR_SIZE = 256
    
    # Core API functions
    lib.taguchi_parse_definition.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    lib.taguchi_parse_definition.restype = ctypes.c_void_p
    
    lib.taguchi_free_definition.argtypes = [ctypes.c_void_p]
    lib.taguchi_free_definition.restype = None
    
    lib.taguchi_validate_definition.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    lib.taguchi_validate_definition.restype = ctypes.c_bool
    
    lib.taguchi_list_arrays.argtypes = []
    lib.taguchi_list_arrays.restype = ctypes.POINTER(ctypes.c_char_p)
    
    # NEW: Auto-selection function
    lib.taguchi_suggest_optimal_array.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    lib.taguchi_suggest_optimal_array.restype = ctypes.c_char_p
    
    print("\\nNew Auto-Selection Capability:")
    print("- When no array is specified, the tool automatically suggests the best array")
    print("- Based on number of factors and levels per factor")
    print("- Minimizes experimental runs while maintaining statistical validity")
    
    # List available arrays
    print("\\nAvailable orthogonal arrays:")
    array_names_ptr = lib.taguchi_list_arrays()
    i = 0
    while True:
        name_ptr = ctypes.cast(array_names_ptr[i], ctypes.c_char_p)
        if not name_ptr.value:
            break
        print(f"  - {name_ptr.value.decode('utf-8')}")
        i += 1
    
    print("\\nExample scenarios for auto-selection:")
    
    # Scenario 1: 3 factors, 2 levels each -> should suggest L4/L8
    scenario1 = '''factors:
  temp: 350F, 400F
  pressure: 10, 15
  speed: slow, fast
# array: [not specified - will auto-select]'''

    print("\\nScenario 1: 3 factors with 2 levels each")
    print("  Factors: temp[2], pressure[2], speed[2]")
    print("  Expected: L8 (or L4 if available with 3 factors)")
    
    # Scenario 2: 4 factors, 3 levels each -> should suggest L9
    scenario2 = '''factors:
  temp: 300F, 350F, 400F
  time: 10min, 15min, 20min
  size: small, medium, large
  material: A, B, C
# array: [not specified - will auto-select]'''

    print("\\nScenario 2: 4 factors with 3 levels each") 
    print("  Factors: temp[3], time[3], size[3], material[3]")
    print("  Expected: L9 (perfect match for 4 factors at 3 levels)")
    
    # Scenario 3: Mixed levels -> depends on max levels and factor count
    scenario3 = '''factors:
  temp: 300F, 350F, 400F  # 3 levels
  on_off: OFF, ON         # 2 levels
  pressure: 10, 15, 20    # 3 levels
# array: [not specified - will auto-select]'''

    print("\\nScenario 3: Mixed levels (2-3 levels per factor)")
    print("  Factors: temp[3], on_off[2], pressure[3]")
    print("  Expected: L9 or L16 (since max levels is 3)")
    
    print("\\nAPI Function Added:")
    print("- taguchi_suggest_optimal_array() - Automatically recommends best array")
    print("- Maintains all existing functionality and compatibility")
    print("- Makes the tool more user-friendly by eliminating guesswork")
    
    print("\\nThis feature makes the Taguchi Array Tool much more accessible!")
    

if __name__ == "__main__":
    demo_auto_selection()