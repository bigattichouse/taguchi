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

/* L27 is generated algorithmically (GF(3)^3) for guaranteed orthogonality */

/*
 * General GF(p) orthogonal array generator for L(p^n) arrays.
 * Generates arrays by enumerating all n-tuples as rows and using
 * linear combinations over GF(p) as columns.
 *
 * For prime p and n dimensions: rows = p^n, cols = (p^n - 1) / (p - 1)
 * Each column is one representative from each equivalence class of
 * non-zero vectors in GF(p)^n under scalar multiplication.
 *
 * For GF(2): cols = 2^n - 1 (no scalar multiples except self)
 * For GF(3): cols = (3^n - 1) / 2 (pairs {v, 2v})
 */

/* Compute p^n for small n, small prime p */
static size_t pow_p(int p, int n) {
    size_t result = 1;
    for (int i = 0; i < n; i++) {
        result *= (size_t)p;
    }
    return result;
}

/*
 * Check if vector v is the canonical representative of its scalar multiple class.
 * We pick the one whose first non-zero component is 1.
 * Works for both GF(2) and GF(3).
 */
static bool is_canonical(const int *v, int n) {
    (void)n; /* Used in loop bounds */
    for (int i = 0; i < n; i++) {
        if (v[i] != 0) {
            return v[i] == 1;
        }
    }
    return false; /* zero vector - not canonical */
}

/*
 * Generate L(p^n) orthogonal array data for prime p.
 * Returns allocated array data (caller must free).
 *
 * Column ordering: unit vectors (e1..en) come first, then remaining
 * canonical vectors sorted by index. This ensures sequential column
 * assignment picks linearly independent columns for multi-column
 * (paired/tripled) factors.
 */
static int *generate_power_oa(int p, int n, size_t *rows_out, size_t *cols_out) {
    size_t rows = pow_p(p, n);
    size_t cols = (rows - 1) / (size_t)(p - 1);

    *rows_out = rows;
    *cols_out = cols;

    int *data = xmalloc(rows * cols * sizeof(int));

    /* Build the list of canonical column vectors */
    int (*col_vectors)[10] = xmalloc(cols * sizeof(int[10])); /* max n=10 for L1024/L2187 */
    size_t col_idx = 0;

    /* Phase 1: Insert unit vectors first (e_i for i=0..n-1).
     * These are always linearly independent and canonical. */
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            col_vectors[col_idx][k] = (k == i) ? 1 : 0;
        }
        col_idx++;
    }

    /* Phase 2: Insert remaining canonical vectors */
    for (size_t v = 1; v < rows; v++) {
        int vec[10];
        size_t tmp = v;
        for (int k = n - 1; k >= 0; k--) {
            vec[k] = (int)(tmp % (size_t)p);
            tmp /= (size_t)p;
        }
        if (!is_canonical(vec, n)) continue;

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
        int x[10];
        size_t tmp = r;
        for (int k = n - 1; k >= 0; k--) {
            x[k] = (int)(tmp % (size_t)p);
            tmp /= (size_t)p;
        }

        /* Compute each column value */
        for (size_t c = 0; c < cols; c++) {
            int val = 0;
            for (int k = 0; k < n; k++) {
                val += col_vectors[c][k] * x[k];
            }
            data[r * cols + c] = val % p;
        }
    }

    free(col_vectors);
    return data;
}

/*
 * GF(2) orthogonal array generator for L(2^n) arrays.
 * For n dimensions: rows = 2^n, cols = 2^n - 1
 */
static int *generate_power2_oa(int n, size_t *rows_out, size_t *cols_out) {
    return generate_power_oa(2, n, rows_out, cols_out);
}

/*
 * GF(3) orthogonal array generator for L(3^n) arrays.
 * For n dimensions: rows = 3^n, cols = (3^n - 1) / 2
 */
static int *generate_power3_oa(int n, size_t *rows_out, size_t *cols_out) {
    return generate_power_oa(3, n, rows_out, cols_out);
}

/* Generated array data (initialized lazily) */
/* GF(2) series */
static int *L32_data = NULL;
static int *L64_data = NULL;
static int *L128_data = NULL;
static int *L256_data = NULL;
static int *L512_data = NULL;
static int *L1024_data = NULL;
static size_t L32_rows = 0, L32_cols = 0;
static size_t L64_rows = 0, L64_cols = 0;
static size_t L128_rows = 0, L128_cols = 0;
static size_t L256_rows = 0, L256_cols = 0;
static size_t L512_rows = 0, L512_cols = 0;
static size_t L1024_rows = 0, L1024_cols = 0;

/* GF(3) series */
static int *L27_data_gen = NULL;
static int *L81_data = NULL;
static int *L243_data = NULL;
static int *L729_data = NULL;
static int *L2187_data = NULL;
static size_t L27_rows = 0, L27_cols = 0;
static size_t L81_rows = 0, L81_cols = 0;
static size_t L243_rows = 0, L243_cols = 0;
static size_t L729_rows = 0, L729_cols = 0;
static size_t L2187_rows = 0, L2187_cols = 0;

/* Static array entries for predefined 2-level arrays */
#define NUM_STATIC_ARRAYS 4
static const OrthogonalArray static_arrays[] = {
    { "L4", 4, 3, 2, L4_data },
    { "L8", 8, 7, 2, L8_data },
    { "L9", 9, 4, 3, L9_data },
    { "L16", 16, 15, 2, L16_data }
};

/* Full array list including generated arrays */
#define MAX_ARRAYS 16
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

    /* Generate GF(2) series: L32, L64, L128, L256, L512, L1024 */
    L32_data = generate_power2_oa(5, &L32_rows, &L32_cols);
    all_arrays[all_arrays_count].name = "L32";
    all_arrays[all_arrays_count].rows = L32_rows;
    all_arrays[all_arrays_count].cols = L32_cols;
    all_arrays[all_arrays_count].levels = 2;
    all_arrays[all_arrays_count].data = L32_data;
    all_arrays_count++;

    L64_data = generate_power2_oa(6, &L64_rows, &L64_cols);
    all_arrays[all_arrays_count].name = "L64";
    all_arrays[all_arrays_count].rows = L64_rows;
    all_arrays[all_arrays_count].cols = L64_cols;
    all_arrays[all_arrays_count].levels = 2;
    all_arrays[all_arrays_count].data = L64_data;
    all_arrays_count++;

    L128_data = generate_power2_oa(7, &L128_rows, &L128_cols);
    all_arrays[all_arrays_count].name = "L128";
    all_arrays[all_arrays_count].rows = L128_rows;
    all_arrays[all_arrays_count].cols = L128_cols;
    all_arrays[all_arrays_count].levels = 2;
    all_arrays[all_arrays_count].data = L128_data;
    all_arrays_count++;

    L256_data = generate_power2_oa(8, &L256_rows, &L256_cols);
    all_arrays[all_arrays_count].name = "L256";
    all_arrays[all_arrays_count].rows = L256_rows;
    all_arrays[all_arrays_count].cols = L256_cols;
    all_arrays[all_arrays_count].levels = 2;
    all_arrays[all_arrays_count].data = L256_data;
    all_arrays_count++;

    L512_data = generate_power2_oa(9, &L512_rows, &L512_cols);
    all_arrays[all_arrays_count].name = "L512";
    all_arrays[all_arrays_count].rows = L512_rows;
    all_arrays[all_arrays_count].cols = L512_cols;
    all_arrays[all_arrays_count].levels = 2;
    all_arrays[all_arrays_count].data = L512_data;
    all_arrays_count++;

    L1024_data = generate_power2_oa(10, &L1024_rows, &L1024_cols);
    all_arrays[all_arrays_count].name = "L1024";
    all_arrays[all_arrays_count].rows = L1024_rows;
    all_arrays[all_arrays_count].cols = L1024_cols;
    all_arrays[all_arrays_count].levels = 2;
    all_arrays[all_arrays_count].data = L1024_data;
    all_arrays_count++;

    /* Generate GF(3) series: L27, L81, L243, L729, L2187 */
    /* L27 (replaces buggy hardcoded data) */
    L27_data_gen = generate_power3_oa(3, &L27_rows, &L27_cols);
    all_arrays[all_arrays_count].name = "L27";
    all_arrays[all_arrays_count].rows = L27_rows;
    all_arrays[all_arrays_count].cols = L27_cols;
    all_arrays[all_arrays_count].levels = 3;
    all_arrays[all_arrays_count].data = L27_data_gen;
    all_arrays_count++;

    L81_data = generate_power3_oa(4, &L81_rows, &L81_cols);
    all_arrays[all_arrays_count].name = "L81";
    all_arrays[all_arrays_count].rows = L81_rows;
    all_arrays[all_arrays_count].cols = L81_cols;
    all_arrays[all_arrays_count].levels = 3;
    all_arrays[all_arrays_count].data = L81_data;
    all_arrays_count++;

    L243_data = generate_power3_oa(5, &L243_rows, &L243_cols);
    all_arrays[all_arrays_count].name = "L243";
    all_arrays[all_arrays_count].rows = L243_rows;
    all_arrays[all_arrays_count].cols = L243_cols;
    all_arrays[all_arrays_count].levels = 3;
    all_arrays[all_arrays_count].data = L243_data;
    all_arrays_count++;

    L729_data = generate_power3_oa(6, &L729_rows, &L729_cols);
    all_arrays[all_arrays_count].name = "L729";
    all_arrays[all_arrays_count].rows = L729_rows;
    all_arrays[all_arrays_count].cols = L729_cols;
    all_arrays[all_arrays_count].levels = 3;
    all_arrays[all_arrays_count].data = L729_data;
    all_arrays_count++;

    L2187_data = generate_power3_oa(7, &L2187_rows, &L2187_cols);
    all_arrays[all_arrays_count].name = "L2187";
    all_arrays[all_arrays_count].rows = L2187_rows;
    all_arrays[all_arrays_count].cols = L2187_cols;
    all_arrays[all_arrays_count].levels = 3;
    all_arrays[all_arrays_count].data = L2187_data;
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
