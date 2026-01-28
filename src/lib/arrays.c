#include "arrays.h"
#include "utils.h"  // for set_error function
#include <string.h>
#include <stdbool.h>

/* Predefined arrays (const static data) */

static const int L4_data[] = {
    0, 0, 0,
    0, 1, 1,
    1, 0, 1,
    1, 1, 0
};

static const int L8_data[] = {
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1,
    0, 1, 1, 0, 0, 1, 1,
    0, 1, 1, 1, 1, 0, 0,
    1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 1, 0, 1, 0,
    1, 1, 0, 0, 1, 1, 0,
    1, 1, 0, 1, 0, 0, 1
};

static const int L9_data[] = {
    0, 0, 0, 0,
    0, 1, 1, 1,
    0, 2, 2, 2,
    1, 0, 1, 2,
    1, 1, 2, 0,
    1, 2, 0, 1,
    2, 0, 2, 1,
    2, 1, 0, 2,
    2, 2, 1, 0
};

static const int L16_data[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
    0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
    1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0,
    1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0,
    1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1,
    1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};

static const int L27_data[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0,
    1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2,
    0, 1, 2, 2, 0, 1, 1, 2, 0, 2, 0, 1, 0,
    0, 2, 1, 1, 2, 0, 0, 1, 2, 0, 1, 2, 1,
    1, 0, 2, 0, 1, 2, 2, 0, 1, 0, 2, 1, 2,
    1, 1, 1, 2, 2, 2, 0, 0, 0, 2, 2, 2, 0,
    1, 2, 0, 1, 0, 2, 1, 2, 0, 1, 0, 2, 1,
    2, 0, 1, 2, 1, 0, 0, 2, 1, 2, 1, 0, 2,
    2, 1, 0, 0, 2, 1, 2, 1, 0, 0, 2, 1, 1,
    2, 2, 2, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 2, 2, 2, 1, 1, 1, 2, 2, 2, 0,
    0, 1, 2, 1, 2, 0, 0, 2, 1, 2, 1, 0, 1,
    0, 2, 1, 0, 1, 2, 1, 0, 2, 1, 2, 0, 2,
    1, 0, 2, 2, 0, 1, 2, 1, 0, 0, 2, 1, 0,
    1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 2,
    1, 2, 0, 1, 0, 2, 0, 0, 2, 1, 0, 2, 0,
    2, 0, 1, 2, 1, 0, 1, 2, 0, 2, 0, 1, 2,
    2, 1, 0, 0, 2, 1, 0, 1, 2, 2, 1, 0, 1,
    2, 2, 2, 1, 1, 1, 2, 2, 2, 0, 0, 0, 1
};


static const OrthogonalArray all_arrays[] = {
    { "L4", 4, 3, 2, L4_data },
    { "L8", 8, 7, 2, L8_data },
    { "L9", 9, 4, 3, L9_data },
    { "L16", 16, 15, 2, L16_data },
    { "L27", 27, 13, 3, L27_data }
};

static const char *array_names[] = {
    "L4",
    "L8",
    "L9",
    "L16",
    "L27",
    NULL
};

const OrthogonalArray *get_array(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < sizeof(all_arrays) / sizeof(all_arrays[0]); i++) {
        if (strcmp(name, all_arrays[i].name) == 0) {
            return &all_arrays[i];
        }
    }
    return NULL;
}

const char **list_array_names(void) {
    return array_names;
}

/* Helper function to determine if an array can accommodate the factors */
static bool can_accommodate_factors(const OrthogonalArray *array, const ExperimentDef *def) {
    if (!array || !def) return false;

    // Check if array supports enough factors
    if (def->factor_count > array->cols) {
        return false;
    }

    // Check if all factors have levels compatible with array
    for (size_t i = 0; i < def->factor_count; i++) {
        const Factor *factor = &def->factors[i];
        if (factor->level_count > array->levels) {
            return false;
        }
    }

    return true;
}

/* Get all array structures for internal use */
const OrthogonalArray *get_all_arrays(size_t *count_out) {
    if (count_out) {
        *count_out = sizeof(all_arrays) / sizeof(all_arrays[0]);
    }
    return all_arrays;
}

/* Function to automatically suggest the most appropriate orthogonal array */
const char *suggest_optimal_array(const ExperimentDef *def, char *error_buf) {
    if (!def) {
        if (error_buf) {
            strcpy(error_buf, "Invalid definition for array suggestion");
        }
        return NULL;
    }

    // Find maximum number of levels required among all factors
    size_t max_levels = 0;
    for (size_t i = 0; i < def->factor_count; i++) {
        if (def->factors[i].level_count > max_levels) {
            max_levels = def->factors[i].level_count;
        }
    }

    // Find the smallest array that can accommodate all factors
    for (size_t i = 0; i < sizeof(all_arrays) / sizeof(all_arrays[0]); i++) {
        const OrthogonalArray *array = &all_arrays[i];

        // Check if this array can accommodate the factors
        if (can_accommodate_factors(array, def)) {
            // Also check that the array supports the required number of levels
            if (array->levels >= max_levels) {
                return array->name;
            }
        }
    }

    // If no array fits, return NULL with error message
    if (error_buf) {
        set_error(error_buf, "No suitable array found for %zu factors (max %zu levels each). "
                "Try reducing factor count or level count per factor.",
                def->factor_count, max_levels);
    }
    return NULL;
}
