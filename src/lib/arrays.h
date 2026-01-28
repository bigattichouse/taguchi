#ifndef ARRAYS_H
#define ARRAYS_H

#include <stddef.h>
#include <stdbool.h>  // for bool type
#include "../../src/config.h"  // Include config for constants
#include "parser.h"   // for ExperimentDef structure

/* Orthogonal array structure (internal) */
typedef struct {
    const char *name;
    size_t rows;      /* Number of experiments */
    size_t cols;      /* Number of factors */
    size_t levels;    /* Levels per factor (2 or 3) */
    const int *data;  /* Row-major array */
} OrthogonalArray;

/* Lookup array by name */
const OrthogonalArray *get_array(const char *name);

/* List all available arrays (for public API) */
const char **list_array_names(void);

/* Suggest most appropriate orthogonal array based on factor requirements */
const char *suggest_optimal_array(const ExperimentDef *def, char *error_buf);

/* List all array structures (for internal use) */
const OrthogonalArray *get_all_arrays(size_t *count_out);

#endif /* ARRAYS_H */
