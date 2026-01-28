#include "taguchi.h"
#include "parser.h"
#include "arrays.h"
#include "generator.h"
#include "serializer.h"
#include "utils.h"
#include "../config.h"  // Include config for constants
#include <stdlib.h>     // For malloc, free
#include <stdio.h>      // For snprintf
#include <string.h>

/*
 * ============================================================================
 * Internal structure definitions (for opaque handles)
 * ============================================================================
 */

// Define internal structures that match the opaque handles declared in the public header
struct taguchi_experiment_def {
    ExperimentDef internal_def;
};

struct taguchi_experiment_run {
    ExperimentRun internal_run;
};

struct taguchi_result_set {
    // Placeholder - will be implemented in analysis phase
    int dummy; // Replace with real struct when analyzer is implemented
};

struct taguchi_main_effect {
    // Placeholder - will be implemented in analysis phase
    int dummy; // Replace with real struct when analyzer is implemented
};

/*
 * ============================================================================
 * Experiment Definition API Implementation
 * ============================================================================
 */

taguchi_experiment_def_t *taguchi_parse_definition(const char *content, char *error_buf) {
    taguchi_experiment_def_t *def = xmalloc(sizeof(taguchi_experiment_def_t));
    
    if (parse_experiment_def_from_string(content, &def->internal_def, error_buf) != 0) {
        free(def);
        return NULL;
    }
    
    return def;
}

taguchi_experiment_def_t *taguchi_create_definition(const char *array_type) {
    taguchi_experiment_def_t *def = xmalloc(sizeof(taguchi_experiment_def_t));
    memset(&def->internal_def, 0, sizeof(def->internal_def));
    
    if (strlen(array_type) >= sizeof(def->internal_def.array_type)) {
        return NULL;
    }
    
    strcpy(def->internal_def.array_type, array_type);
    return def;
}

int taguchi_add_factor(taguchi_experiment_def_t *def, const char *name, const char **levels, size_t level_count, char *error_buf) {
    if (!def || !name || !levels || level_count == 0 || level_count > MAX_LEVELS) {
        set_error(error_buf, "Invalid parameters to taguchi_add_factor");
        return -1;
    }

    if (def->internal_def.factor_count >= MAX_FACTORS) {
        set_error(error_buf, "Maximum number of factors (%d) exceeded", MAX_FACTORS);
        return -1;
    }

    Factor *new_factor = &def->internal_def.factors[def->internal_def.factor_count];
    memset(new_factor, 0, sizeof(Factor));

    if (strlen(name) >= MAX_FACTOR_NAME) {
        set_error(error_buf, "Factor name too long: %s", name);
        return -1;
    }
    strcpy(new_factor->name, name);
    new_factor->level_count = level_count;

    for (size_t i = 0; i < level_count && i < MAX_LEVELS; i++) {
        if (strlen(levels[i]) >= MAX_LEVEL_VALUE) {
            set_error(error_buf, "Level value too long: %s", levels[i]);
            return -1;
        }
        strcpy(new_factor->values[i], levels[i]);
    }

    def->internal_def.factor_count++;
    return 0;
}

bool taguchi_validate_definition(const taguchi_experiment_def_t *def, char *error_buf) {
    if (!def) return false;
    return validate_experiment_def(&def->internal_def, error_buf);
}

void taguchi_free_definition(taguchi_experiment_def_t *def) {
    if (def) {
        free_experiment_def(&def->internal_def);
        free(def);
    }
}

/*
 * ============================================================================
 * Array Information API Implementation
 * ============================================================================
 */

const char **taguchi_list_arrays(void) {
    return list_array_names();
}

int taguchi_get_array_info(const char *name, size_t *rows_out, size_t *cols_out, size_t *levels_out) {
    if (!name || !rows_out || !cols_out || !levels_out) {
        return -1;
    }
    
    const OrthogonalArray *array = get_array(name);
    if (!array) {
        return -1;
    }
    
    *rows_out = array->rows;
    *cols_out = array->cols;
    *levels_out = array->levels;
    return 0;
}

/*
 * ============================================================================
 * Generation API Implementation
 * ============================================================================
 */

int taguchi_generate_runs(const taguchi_experiment_def_t *def, taguchi_experiment_run_t ***runs_out, size_t *count_out, char *error_buf) {
    if (!def || !runs_out || !count_out) {
        set_error(error_buf, "Invalid parameters to taguchi_generate_runs");
        return -1;
    }
    
    ExperimentRun *internal_runs = NULL;
    size_t internal_count = 0;
    
    int result = generate_experiments(&def->internal_def, &internal_runs, &internal_count, error_buf);
    if (result != 0) {
        return -1;
    }
    
    // Convert internal runs to opaque handles
    taguchi_experiment_run_t **external_runs = xmalloc(internal_count * sizeof(taguchi_experiment_run_t*));
    for (size_t i = 0; i < internal_count; i++) {
        external_runs[i] = xmalloc(sizeof(taguchi_experiment_run_t));
        memcpy(&external_runs[i]->internal_run, &internal_runs[i], sizeof(ExperimentRun));
    }
    
    *runs_out = external_runs;
    *count_out = internal_count;
    
    // Free the temporary internal structure array (but not the data itself)
    free(internal_runs);
    
    return 0;
}

const char *taguchi_run_get_value(const taguchi_experiment_run_t *run, const char *factor_name) {
    if (!run || !factor_name) return NULL;
    
    // Search for the factor by name and return its value
    for (size_t i = 0; i < run->internal_run.factor_count; i++) {
        if (strcmp(run->internal_run.factor_names[i], factor_name) == 0) {
            return run->internal_run.values[i];
        }
    }
    return NULL;
}

size_t taguchi_run_get_id(const taguchi_experiment_run_t *run) {
    if (!run) return 0;
    return run->internal_run.run_id;
}

const char **taguchi_run_get_factor_names(const taguchi_experiment_run_t *run) {
    if (!run) return NULL;
    
    // This is tricky - we need to return a static array of factor names (or a way to iterate)
    // For simplicity, we'll return NULL and implement differently if needed
    // In a real implementation, this might require more complex memory management
    return NULL; // Temporary - requires more complex implementation
}

void taguchi_free_runs(taguchi_experiment_run_t **runs, size_t count) {
    if (runs) {
        for (size_t i = 0; i < count; i++) {
            if (runs[i]) {
                free(runs[i]);
            }
        }
        free(runs);
    }
}

/*
 * ============================================================================
 * Results API Implementation (Stubs for now)
 * ============================================================================
 */

taguchi_result_set_t *taguchi_create_result_set(const taguchi_experiment_def_t *def, const char *metric_name) {
    // Implementation will be added when analyzer module is developed
    (void)def;  // Suppress unused parameter
    (void)metric_name;  // Suppress unused parameter
    return NULL;
}

int taguchi_add_result(taguchi_result_set_t *results, size_t run_id, double response_value, char *error_buf) {
    // Implementation will be added when analyzer module is developed
    (void)results;  // Suppress unused parameter
    (void)run_id;  // Suppress unused parameter
    (void)response_value;  // Suppress unused parameter
    (void)error_buf;  // Suppress unused parameter
    return -1;
}

void taguchi_free_result_set(taguchi_result_set_t *results) {
    // Implementation will be added when analyzer module is developed
    (void)results;  // Suppress unused parameter
}

/*
 * ============================================================================
 * Analysis API Implementation (Stubs for now)
 * ============================================================================
 */

int taguchi_calculate_main_effects(const taguchi_result_set_t *results, taguchi_main_effect_t ***effects_out, size_t *count_out, char *error_buf) {
    // Implementation will be added when analyzer module is developed
    (void)results;            // Suppress unused parameter
    (void)effects_out;        // Suppress unused parameter
    (void)count_out;          // Suppress unused parameter
    (void)error_buf;          // Suppress unused parameter
    return -1;
}

const char *taguchi_effect_get_factor(const taguchi_main_effect_t *effect) {
    // Implementation will be added when analyzer module is developed
    (void)effect;             // Suppress unused parameter
    return NULL;
}

const double *taguchi_effect_get_level_means(const taguchi_main_effect_t *effect, size_t *level_count_out) {
    // Implementation will be added when analyzer module is developed
    (void)effect;             // Suppress unused parameter
    if (level_count_out) *level_count_out = 0;
    return NULL;
}

double taguchi_effect_get_range(const taguchi_main_effect_t *effect) {
    // Implementation will be added when analyzer module is developed
    (void)effect;             // Suppress unused parameter
    return 0.0;
}

void taguchi_free_effects(taguchi_main_effect_t **effects, size_t count) {
    // Implementation will be added when analyzer module is developed
    (void)effects;            // Suppress unused parameter
    (void)count;              // Suppress unused parameter
}

int taguchi_recommend_optimal(const taguchi_main_effect_t **effects, size_t effect_count, bool higher_is_better, char *recommendation_buf, size_t buf_size) {
    // Implementation will be added when analyzer module is developed
    (void)effects;            // Suppress unused parameter
    (void)effect_count;       // Suppress unused parameter
    (void)higher_is_better;   // Suppress unused parameter
    (void)recommendation_buf; // Suppress unused parameter
    (void)buf_size;           // Suppress unused parameter
    return -1;
}

/*
 * ============================================================================
 * Serialization API Implementation
 * ============================================================================
 */

char *taguchi_runs_to_json(const taguchi_experiment_run_t **runs, size_t count) {
    if (!runs || count == 0) {
        char *result = xmalloc(3);
        strcpy(result, "[]");
        return result;
    }

    // Convert taguchi_experiment_run_t array to internal ExperimentRun array for serialization
    // For now, we'll assume the internal structure is compatible
    ExperimentRun *internal_runs = xmalloc(count * sizeof(ExperimentRun));
    for (size_t i = 0; i < count; i++) {
        memcpy(&internal_runs[i], &runs[i]->internal_run, sizeof(ExperimentRun));
    }

    char *json_result = serialize_runs_to_json(internal_runs, count);

    free(internal_runs);
    return json_result;
}

char *taguchi_effects_to_json(const taguchi_main_effect_t **effects, size_t count) {
    // For now return a basic JSON placeholder - will be implemented when analyzer is complete
    (void)effects;  // Suppress unused parameter
    char *json = xmalloc(100);
    snprintf(json, 100, "[/* %zu effects serialized */]", count);
    return json;
}

void taguchi_free_string(char *str) {
    if (str) {
        free_serialized_string(str);
    }
}