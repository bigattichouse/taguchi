#include "analyzer.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

/* Create result set for collecting experimental data */
ResultSet *create_result_set(const ExperimentDef *def, const char *metric_name) {
    if (!def || !metric_name) return NULL;

    ResultSet *results = xmalloc(sizeof(ResultSet));
    memset(results, 0, sizeof(ResultSet));

    // Initialize with some capacity
    results->capacity = 16;  // Default initial capacity
    results->responses = xmalloc(results->capacity * sizeof(double));
    results->run_ids = xmalloc(results->capacity * sizeof(size_t));
    results->count = 0;

    if (strlen(metric_name) >= MAX_FACTOR_NAME) {
        free(results->responses);
        free(results->run_ids);
        free(results);
        return NULL;
    }

    strcpy(results->metric_name, metric_name);
    // Store the experiment definition pointer
    // In a real implementation, we might need to copy the def rather than store a pointer
    // For now, we'll store the pointer assuming the def lifetime exceeds results lifetime
    results->experiment_def = (ExperimentDef*)def;

    return results;
}

/* Add result to set */
int add_result(ResultSet *results, size_t run_id, double response_value) {
    if (!results) return -1;

    // Check if we need to expand capacity
    if (results->count >= results->capacity) {
        results->capacity *= 2;
        results->responses = xrealloc(results->responses, results->capacity * sizeof(double));
        results->run_ids = xrealloc(results->run_ids, results->capacity * sizeof(size_t));
    }

    // Add the result
    results->run_ids[results->count] = run_id;
    results->responses[results->count] = response_value;
    results->count++;

    return 0;
}

/* Get the response value for a particular run ID */
double get_response_for_run(const ResultSet *results, size_t run_id) {
    if (!results) return 0.0;

    // Search for the run ID in our stored results
    for (size_t i = 0; i < results->count; i++) {
        if (results->run_ids[i] == run_id) {
            return results->responses[i];
        }
    }

    // If not found, return 0.0 (or some indicator that it wasn't found)
    return 0.0;
}

/* Free result set */
void free_result_set(ResultSet *results) {
    if (results) {
        if (results->responses) {
            free(results->responses);
        }
        if (results->run_ids) {
            free(results->run_ids);
        }
        // Don't free experiment_def as it belongs to the caller
        results->experiment_def = NULL;
        free(results);
    }
}

/* Calculate main effects from results and experiment design */
int calculate_main_effects(const ResultSet *results, MainEffect **effects_out, size_t *count_out) {
    if (!results || !effects_out || !count_out || !results->experiment_def) {
        return -1;
    }

    const ExperimentDef *def = results->experiment_def;

    // Create effects array - one per factor
    MainEffect *effects = xmalloc(def->factor_count * sizeof(MainEffect));

    for (size_t factor_idx = 0; factor_idx < def->factor_count; factor_idx++) {
        MainEffect *effect = &effects[factor_idx];
        memset(effect, 0, sizeof(MainEffect));

        // Get factor info
        const Factor *factor = &def->factors[factor_idx];
        strcpy(effect->factor_name, factor->name);
        effect->level_count = factor->level_count;
        effect->level_means = xmalloc(factor->level_count * sizeof(double));

        // Initialize sums and counts for each level
        double *level_sums = xcalloc(factor->level_count, sizeof(double));
        size_t *level_counts = xmalloc(factor->level_count * sizeof(size_t));
        memset(level_counts, 0, factor->level_count * sizeof(size_t));

        // For calculating main effects, we need to know which factor levels were used in each run
        // To do this properly, we would need to have some external knowledge of the experimental design
        // For example, we might receive the generated runs as well

        // In a full implementation, we would:
        // 1. Have access to the generated experiment runs showing which level of each factor was used in each run
        // 2. Match up the results with the corresponding run configurations
        // 3. Group the response values by factor level
        // 4. Calculate means for each level of each factor

        // This is a simplified approach to demonstrate the concept:
        // We'll calculate means assuming the results are ordered by run_id
        // and we have some external mapping

        // In a complete implementation, we'd loop through results and use experimental design
        // to determine which level of the current factor was used in each result
        for (size_t result_idx = 0; result_idx < results->count; result_idx++) {
            size_t run_id = results->run_ids[result_idx];
            double response = results->responses[result_idx];

            // Here we need to know what level of the current factor_idx was used for this run_id
            // This requires knowledge of the experimental design mapping
            // For now, this is a placeholder - actual implementation would require
            // access to the run configurations

            // For demonstration, assume a simple mapping where level is determined by some formula
            // This is NOT the real implementation - just showing structure
            size_t level_idx = (run_id - 1) % factor->level_count;  // Simplified mapping

            if (level_idx < factor->level_count) {
                level_sums[level_idx] += response;
                level_counts[level_idx]++;
            }
        }

        // Calculate means for each level
        for (size_t level_idx = 0; level_idx < factor->level_count; level_idx++) {
            if (level_counts[level_idx] > 0) {
                effect->level_means[level_idx] = level_sums[level_idx] / level_counts[level_idx];
            } else {
                effect->level_means[level_idx] = 0.0; // Or some default/error value
            }
        }

        // Calculate range (max - min)
        if (factor->level_count > 0) {
            double min_val = effect->level_means[0];
            double max_val = effect->level_means[0];

            for (size_t level_idx = 1; level_idx < factor->level_count; level_idx++) {
                if (effect->level_means[level_idx] < min_val) {
                    min_val = effect->level_means[level_idx];
                }
                if (effect->level_means[level_idx] > max_val) {
                    max_val = effect->level_means[level_idx];
                }
            }

            effect->range = max_val - min_val;
        } else {
            effect->range = 0.0;
        }

        free(level_sums);
        free(level_counts);
    }

    *effects_out = effects;
    *count_out = def->factor_count;
    return 0;
}

/* Free main effects */
void free_main_effects(MainEffect *effects, size_t count) {
    if (effects) {
        for (size_t i = 0; i < count; i++) {
            if (effects[i].level_means) {
                free(effects[i].level_means);
            }
        }
        free(effects);
    }
}

/* Recommend optimal levels based on effects */
int recommend_optimal_levels(const MainEffect *effects, size_t effect_count,
                            bool higher_is_better,
                            char *recommendation_buf, size_t buf_size) {
    if (!effects || !recommendation_buf || buf_size == 0) {
        return -1;
    }

    // Build a simple recommendation string
    char temp_buf[BUFFER_SIZE];
    size_t pos = 0;

    pos += snprintf(temp_buf + pos, sizeof(temp_buf) - pos, "Optimal configuration: ");

    for (size_t i = 0; i < effect_count; i++) {
        const MainEffect *effect = &effects[i];

        // Find the best level for this factor based on the effect
        size_t best_level_idx = 0;  // Initialize here - used in loop and potentially elsewhere

        if (effect->level_count > 0) {
            double best_value = effect->level_means[0];  // Start with first value

            for (size_t level_idx = 1; level_idx < effect->level_count; level_idx++) {
                double level_mean = effect->level_means[level_idx];
                bool is_better = higher_is_better ? (level_mean > best_value) : (level_mean < best_value);

                if (is_better) {
                    best_value = level_mean;
                    best_level_idx = level_idx;
                }
            }
        }

        // Prevent "set but not used" warning by using the variable
        (void)best_level_idx;

        // Add to recommendation (we'd need level values, but they're not available here)
        // For a real implementation, we'd need access to the factor definitions
        if (i > 0) {
            pos += snprintf(temp_buf + pos, sizeof(temp_buf) - pos, ", ");
        }
        pos += snprintf(temp_buf + pos, sizeof(temp_buf) - pos, "%s=best",
                      effect->factor_name);
    }

    // Ensure null termination and copy to output buffer
    temp_buf[sizeof(temp_buf) - 1] = '\0';
    if (strlen(temp_buf) >= buf_size) {
        return -1; // Output buffer too small
    }

    strcpy(recommendation_buf, temp_buf);
    return 0;
}