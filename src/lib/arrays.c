#include "arrays.h"
#include "utils.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

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

/*
 * GF(3) orthogonal array generator for L(3^n) arrays.
 * Generates arrays by enumerating all n-tuples as rows and using
 * linear combinations over GF(3) as columns.
 *
 * For n dimensions: rows = 3^n, cols = (3^n - 1) / 2
 * Each column is one representative from each pair {v, 2v} of
 * non-zero vectors in GF(3)^n.
 */

/* Compute 3^n for small n */
static size_t pow3(int n) {
    size_t result = 1;
    for (int i = 0; i < n; i++) {
        result *= 3;
    }
    return result;
}

/*
 * Check if vector v is the canonical representative of {v, 2v}.
 * We pick the one whose first non-zero component is 1.
 */
static bool is_canonical_gf3(const int *v, int n) {
    for (int i = 0; i < n; i++) {
        if (v[i] != 0) {
            return v[i] == 1;
        }
    }
    return false; /* zero vector - not canonical */
}

/*
 * Generate L(3^n) orthogonal array data.
 * Returns allocated array data (caller must free).
 *
 * Column ordering: unit vectors (e1..en) come first, then remaining
 * canonical vectors sorted by weight. This ensures sequential column
 * assignment picks linearly independent columns for multi-column
 * (paired/tripled) factors.
 */
static int *generate_power3_oa(int n, size_t *rows_out, size_t *cols_out) {
    size_t rows = pow3(n);
    size_t cols = (rows - 1) / 2;

    *rows_out = rows;
    *cols_out = cols;

    int *data = xmalloc(rows * cols * sizeof(int));

    /* Build the list of canonical column vectors */
    int (*col_vectors)[5] = xmalloc(cols * sizeof(int[5])); /* max n=5 */
    size_t col_idx = 0;

    /* Phase 1: Insert unit vectors first (e_i for i=0..n-1).
     * These are always linearly independent and canonical. */
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            col_vectors[col_idx][k] = (k == i) ? 1 : 0;
        }
        col_idx++;
    }

    /* Phase 2: Insert remaining canonical vectors (weight >= 2) */
    for (size_t v = 1; v < rows; v++) {
        int vec[5];
        size_t tmp = v;
        for (int k = n - 1; k >= 0; k--) {
            vec[k] = (int)(tmp % 3);
            tmp /= 3;
        }
        if (!is_canonical_gf3(vec, n)) continue;

        /* Skip unit vectors (already added in phase 1) */
        int nonzero_count = 0;
        for (int k = 0; k < n; k++) {
            if (vec[k] != 0) nonzero_count++;
        }
        if (nonzero_count <= 1) continue;

        for (int k = 0; k < n; k++) {
            col_vectors[col_idx][k] = vec[k];
        }
        col_idx++;
    }

    /* Generate array data: for each row (n-tuple) and column (linear combination) */
    for (size_t r = 0; r < rows; r++) {
        /* Decode row index into n-tuple */
        int x[5];
        size_t tmp = r;
        for (int k = n - 1; k >= 0; k--) {
            x[k] = (int)(tmp % 3);
            tmp /= 3;
        }

        /* Compute each column value */
        for (size_t c = 0; c < cols; c++) {
            int val = 0;
            for (int k = 0; k < n; k++) {
                val += col_vectors[c][k] * x[k];
            }
            data[r * cols + c] = val % 3;
        }
    }

    free(col_vectors);
    return data;
}

/* Generated array data (initialized lazily) */
static int *L81_data = NULL;
static int *L243_data = NULL;
static size_t L81_rows = 0, L81_cols = 0;
static size_t L243_rows = 0, L243_cols = 0;

/* Static array entries for predefined arrays */
#define NUM_STATIC_ARRAYS 5
static const OrthogonalArray static_arrays[] = {
    { "L4", 4, 3, 2, L4_data },
    { "L8", 8, 7, 2, L8_data },
    { "L9", 9, 4, 3, L9_data },
    { "L16", 16, 15, 2, L16_data },
    { "L27", 27, 13, 3, L27_data }
};

/* Full array list including generated arrays */
#define MAX_ARRAYS 7
static OrthogonalArray all_arrays[MAX_ARRAYS];
static size_t all_arrays_count = 0;
static bool arrays_initialized = false;

static const char *array_names[MAX_ARRAYS + 1];

/* Initialize generated arrays on first use */
static void ensure_arrays_initialized(void) {
    if (arrays_initialized) return;

    /* Copy static arrays */
    for (size_t i = 0; i < NUM_STATIC_ARRAYS; i++) {
        all_arrays[i] = static_arrays[i];
    }
    all_arrays_count = NUM_STATIC_ARRAYS;

    /* Generate L81 */
    L81_data = generate_power3_oa(4, &L81_rows, &L81_cols);
    all_arrays[all_arrays_count].name = "L81";
    all_arrays[all_arrays_count].rows = L81_rows;
    all_arrays[all_arrays_count].cols = L81_cols;
    all_arrays[all_arrays_count].levels = 3;
    all_arrays[all_arrays_count].data = L81_data;
    all_arrays_count++;

    /* Generate L243 */
    L243_data = generate_power3_oa(5, &L243_rows, &L243_cols);
    all_arrays[all_arrays_count].name = "L243";
    all_arrays[all_arrays_count].rows = L243_rows;
    all_arrays[all_arrays_count].cols = L243_cols;
    all_arrays[all_arrays_count].levels = 3;
    all_arrays[all_arrays_count].data = L243_data;
    all_arrays_count++;

    /* Build name list */
    for (size_t i = 0; i < all_arrays_count; i++) {
        array_names[i] = all_arrays[i].name;
    }
    array_names[all_arrays_count] = NULL;

    arrays_initialized = true;
}

const OrthogonalArray *get_array(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    ensure_arrays_initialized();
    for (size_t i = 0; i < all_arrays_count; i++) {
        if (strcmp(name, all_arrays[i].name) == 0) {
            return &all_arrays[i];
        }
    }
    return NULL;
}

const char **list_array_names(void) {
    ensure_arrays_initialized();
    return array_names;
}

/*
 * Calculate how many OA columns a factor needs based on its level count
 * and the array's base level count. Uses column pairing for multi-level factors.
 */
size_t columns_needed_for_factor(size_t level_count, size_t base_levels) {
    if (level_count <= 1 || base_levels <= 1) return 1;
    size_t cols = 0;
    size_t capacity = 1;
    while (capacity < level_count) {
        capacity *= base_levels;
        cols++;
    }
    return cols > 0 ? cols : 1;
}

/*
 * Calculate total OA columns needed for all factors, accounting for column pairing.
 */
size_t total_columns_needed(const ExperimentDef *def, size_t base_levels) {
    size_t total = 0;
    for (size_t i = 0; i < def->factor_count; i++) {
        total += columns_needed_for_factor(def->factors[i].level_count, base_levels);
    }
    return total;
}

/* Helper function to determine if an array can accommodate the factors */
static bool can_accommodate_factors(const OrthogonalArray *array, const ExperimentDef *def) {
    if (!array || !def) return false;

    /* Check total columns needed (with column pairing) against available columns */
    size_t needed = total_columns_needed(def, array->levels);
    return needed <= array->cols;
}

/* Get all array structures for internal use */
const OrthogonalArray *get_all_arrays(size_t *count_out) {
    ensure_arrays_initialized();
    if (count_out) {
        *count_out = all_arrays_count;
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

    ensure_arrays_initialized();

    /* Find the smallest array that can accommodate all factors with column pairing */
    for (size_t i = 0; i < all_arrays_count; i++) {
        const OrthogonalArray *array = &all_arrays[i];

        if (can_accommodate_factors(array, def)) {
            return array->name;
        }
    }

    /* If no array fits, return NULL with error message */
    size_t max_levels = 0;
    for (size_t i = 0; i < def->factor_count; i++) {
        if (def->factors[i].level_count > max_levels) {
            max_levels = def->factors[i].level_count;
        }
    }
    if (error_buf) {
        set_error(error_buf, "No suitable array found for %zu factors (max %zu levels each). "
                "Try reducing factor count or level count per factor.",
                def->factor_count, max_levels);
    }
    return NULL;
}
