# Taguchi Array Tool - Flexible Level Count Examples

One of the key strengths of the Taguchi Array Tool is its flexibility in handling different numbers of levels for different factors. Unlike full factorial designs which require the same number of levels for all factors, Taguchi orthogonal arrays can accommodate mixed-level experiments.

## How Mixed-Level Experiments Work

### Different Numbers of Levels Per Factor

You can define experiments with factors that have different numbers of levels:

```yaml
# Example: Mixed-level experiment (L16 array)
factors:
  temperature: 350F, 375F          # 2 levels only
  pressure: 10psi, 15psi, 20psi   # 3 levels 
  time: 5min, 10min, 15min, 20min # 4 levels
  catalyst: A, B                  # 2 levels
array: L16
```

In this example:
- Temperature: 2 levels
- Pressure: 3 levels  
- Time: 4 levels
- Catalyst: 2 levels

### Supported Mixed-Level Combinations

Different orthogonal arrays support different mixes of factor levels:

- **L4 array**: Up to 3 factors, 2 levels each (2^3-1 design)
- **L8 array**: Up to 7 factors, 2 levels each (2^7-4 design) 
- **L9 array**: Up to 4 factors, 3 levels each (3^4-2 design)
- **L16 array**: Up to 15 factors, 2 levels each OR mixed levels
- **L27 array**: Up to 13 factors, 3 levels each OR mixed levels

### Practical Examples

#### Example 1: Two temperatures only
```yaml
factors:
  temperature: 350F, 400F          # Only 2 levels
  sugar: 1/2 cup, 3/4 cup, 1 cup  # 3 levels
  butter: 1/2 cup, 3/4 cup        # Only 2 levels  
array: L8  # Can accommodate this 2-3-2 level combination
```

#### Example 2: Four or more values for one factor
```yaml
factors:
  algorithm: alg1, alg2, alg3, alg4, alg5, alg6  # 6 levels
  dataset_size: small, large                      # 2 levels
  threads: 1, 2, 4, 8                          # 4 levels
array: L16  # Would need an L16 array or higher
```

Note: For factors with 4+ levels, you'd typically need to use:
- Mixed-level arrays (like L16, L32, etc.)
- Or split the factor into multiple binary/multi-level factors
- Or use a custom array construction (advanced usage)

#### Example 3: Real-world scenario
```yaml
factors:
  # 2-level factors
  optimization: O0, O2
  compiler: gcc, clang
  
  # 3-level factors  
  cache_size: 64M, 128M, 256M
  threads: 2, 4, 8
  
  # 4-level factor
  algorithm: base, opt1, opt2, opt3  # Would need special handling
  
  # For 4-level factors, you might use:
  # L16 array with special level assignments
  # Or split into multiple 2-level factors:
  # algorithm_v1: base/opt1 vs opt2/opt3
  # algorithm_v2: base/opt2 vs opt1/opt3
array: L16  # For up to 15 2-level factors
```

## Important Notes

1. **Array Compatibility**: The selected array must support the maximum number of levels among your factors
   - Use L9/L27 arrays for 3-level factors
   - For 4+ levels, use mixed-level arrays or level splitting techniques

2. **Factor Count Limits**: Each array has a maximum number of factors it can handle
   - L4: Max 3 factors
   - L8: Max 7 factors  
   - L9: Max 4 factors
   - L16: Max 15 factors
   - L27: Max 13 factors

3. **Orthogonality Guarantee**: Within the limits of the selected array, all factor combinations are tested optimally

4. **Efficiency**: Even with mixed levels, you get maximum information with minimum experimental runs compared to full factorial.

The library handles all these variations automatically when you specify your experiment definition. The engine ensures that the chosen orthogonal array can accommodate your factor requirements efficiently.