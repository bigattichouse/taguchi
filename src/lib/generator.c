#include "generator.h"
#include "utils.h"
#include "arrays.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Check if factors fit in specified array */
bool check_array_compatibility(const ExperimentDef *def, const OrthogonalArray *array, char *error_buf) {
    if (!def || !array) {
        if (error_buf) {
            strcpy(error_buf, "Invalid parameters to check_array_compatibility");
        }
        return false;
    }
    
    // Check if array supports enough factors
    if (def->factor_count > array->cols) {
        if (error_buf) {
            set_error(error_buf, "Array %s supports max %zu factors, but %zu were provided", 
                     array->name, array->cols, def->factor_count);
        }
        return false;
    }
    
    // Check if all factors have levels compatible with array
    for (size_t i = 0; i < def->factor_count; i++) {
        const Factor *factor = &def->factors[i];
        if (factor->level_count > array->levels) {
            if (error_buf) {
                set_error(error_buf, "Factor '%s' has %zu levels, but array '%s' only supports %zu levels", 
                         factor->name, factor->level_count, array->name, array->levels);
            }
            return false;
        }
    }
    
    return true;
}

/* Generate experiments from definition */
int generate_experiments(const ExperimentDef *def, ExperimentRun **runs_out, size_t *count_out, char *error_buf) {
    if (!def || !runs_out || !count_out) {
        if (error_buf) {
            strcpy(error_buf, "Invalid parameters to generate_experiments");
        }
        return -1;
    }
    
    // Find the corresponding array
    const OrthogonalArray *array = get_array(def->array_type);
    if (!array) {
        if (error_buf) {
            set_error(error_buf, "Unknown array type: %s", def->array_type);
        }
        return -1;
    }
    
    // Check compatibility
    if (!check_array_compatibility(def, array, error_buf)) {
        return -1;
    }
    
    // Allocate runs array based on array rows
    ExperimentRun *runs = xcalloc(array->rows, sizeof(ExperimentRun));
    
    // Generate each run
    for (size_t run_idx = 0; run_idx < array->rows; run_idx++) {
        ExperimentRun *run = &runs[run_idx];
        run->run_id = run_idx + 1;  // 1-indexed
        run->factor_count = def->factor_count;
        
        // Copy factor names for this run
        for (size_t factor_idx = 0; factor_idx < def->factor_count; factor_idx++) {
            strcpy(run->factor_names[factor_idx], def->factors[factor_idx].name);
        }
        
        // Map array values to factor levels for this run
        for (size_t factor_idx = 0; factor_idx < def->factor_count; factor_idx++) {
            // Get the level index from the orthogonal array
            int level_index = array->data[run_idx * array->cols + factor_idx];
            
            // Map this index to the actual factor value
            if (level_index >= 0 && (size_t)level_index < def->factors[factor_idx].level_count) {
                strcpy(run->values[factor_idx], def->factors[factor_idx].values[level_index]);
            } else {
                // This shouldn't happen if array compatibility was checked
                if (error_buf) {
                    set_error(error_buf, "Invalid level index %d at run %zu, factor %zu", 
                             level_index, run_idx, factor_idx);
                }
                free_experiments(runs, array->rows);
                return -1;
            }
        }
    }
    
    *runs_out = runs;
    *count_out = array->rows;
    return 0;
}

/* Free generated runs */
void free_experiments(ExperimentRun *runs, size_t count) {
    if (runs) {
        // Free each individual run's data (in this case, no dynamic allocation needed)
        // The data is stored in fixed-size arrays within the structure
        (void)count; // Suppress unused parameter warning
        free(runs);
    }
}