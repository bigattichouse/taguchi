#ifndef ARRAYS_H
#define ARRAYS_H

#include <stddef.h>

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

#endif /* ARRAYS_H */
