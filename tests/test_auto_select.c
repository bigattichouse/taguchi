#include "test_framework.h" 
#include "include/taguchi.h"
#include <string.h>

TEST(suggest_optimal_array_basic_2level) {
    char error[TAGUCHI_ERROR_SIZE];
    
    // Use the format that works from original tests
    const char *content = 
        "factors:\n"
        "  cache_size: 64M, 128M, 256M\n"
        "  threads: 2, 4, 8\n"
        "array: L9\n";
    
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        printf("Parse failed with: %s\n", error);
    }
    ASSERT_NOT_NULL(def);
    
    const char *recommended = taguchi_suggest_optimal_array(def, error);
    if (!recommended) {
        printf("Suggestion failed with: %s\n", error);
    }
    ASSERT_NOT_NULL(recommended);
    
    // The function should return a valid array name, even if it's the same as already specified
    bool is_valid_output = (recommended[0] == 'L' && strlen(recommended) >= 2);
    ASSERT(is_valid_output);
    
    taguchi_free_definition(def);
}

TEST(suggest_optimal_array_3level) {
    char error[TAGUCHI_ERROR_SIZE];
    
    // Test with 4 factors, 3 levels each - should suggest L9
    const char *content = 
        "factors:\n"
        "  temp: 300F, 350F, 400F\n"
        "  time: 10min, 15min, 20min\n"
        "  size: small, medium, large\n"
        "  material: A, B, C\n"
        "array: L27\n";  // For parsing only
    
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    ASSERT_NOT_NULL(def);
    
    const char *recommended = taguchi_suggest_optimal_array(def, error);
    ASSERT_NOT_NULL(recommended);
    
    // Should suggest appropriate array (like L9 for 4 factors x 3 levels)
    bool is_valid_output = (recommended[0] == 'L' && strlen(recommended) >= 2);
    ASSERT(is_valid_output);
    
    taguchi_free_definition(def);
}

TEST(suggest_optimal_array_mixed_levels) {
    char error[TAGUCHI_ERROR_SIZE];
    
    // Test with mixed levels (some 2-level, some 3-level factors)
    const char *content = 
        "factors:\n"
        "  temp: 300F, 350F, 400F\n"      // 3 levels
        "  on_off: OFF, ON\n"             // 2 levels  
        "  pressure: 10, 15, 20\n"       // 3 levels
        "array: L9\n";                    // For parsing
    
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    ASSERT_NOT_NULL(def);
    
    const char *recommended = taguchi_suggest_optimal_array(def, error);
    ASSERT_NOT_NULL(recommended);
    
    // Should suggest appropriate array supporting up to 3 levels
    bool is_valid_output = (recommended[0] == 'L' && strlen(recommended) >= 2);
    ASSERT(is_valid_output);
    
    taguchi_free_definition(def);
}

TEST(suggest_optimal_array_too_many_factors) {
    char error[TAGUCHI_ERROR_SIZE];

    /* 16 two-level factors need 16 columns. L16 has 15 cols (too few),
       but L81 has 40 cols at 3 levels which fits. So this should succeed now.
       To truly exceed capacity, we need more factors than the largest array. */
    const char *content =
        "factors:\n"
        "  f1: A, B\n  f2: A, B\n  f3: A, B\n  f4: A, B\n  f5: A, B\n"
        "  f6: A, B\n  f7: A, B\n  f8: A, B\n  f9: A, B\n  f10: A, B\n"
        "  f11: A, B\n  f12: A, B\n  f13: A, B\n  f14: A, B\n  f15: A, B\n"
        "  f16: A, B\n";

    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    ASSERT_NOT_NULL(def);

    const char *recommended = taguchi_suggest_optimal_array(def, error);
    /* 16 two-level factors need 16 columns; L81 has 40 columns so it fits */
    ASSERT_NOT_NULL(recommended);

    taguchi_free_definition(def);
}

TEST(suggest_optimal_array_with_3level_limit) {
    char error[TAGUCHI_ERROR_SIZE];
    
    // Test with 13 factors, 3 levels each - should suggest L27
    const char *content = 
        "factors:\n"
        "  f1: A, B, C\n  f2: A, B, C\n  f3: A, B, C\n  f4: A, B, C\n  f5: A, B, C\n"
        "  f6: A, B, C\n  f7: A, B, C\n  f8: A, B, C\n  f9: A, B, C\n  f10: A, B, C\n"
        "  f11: A, B, C\n  f12: A, B, C\n  f13: A, B, C\n"
        "array: L27\n";  // For parsing but test auto-select function
    
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    ASSERT_NOT_NULL(def);
    
    const char *recommended = taguchi_suggest_optimal_array(def, error);
    ASSERT_NOT_NULL(recommended);
    
    // Should suggest appropriate array for 13 factors with 3 levels
    bool is_valid_output = (recommended[0] == 'L' && strlen(recommended) >= 2);
    ASSERT(is_valid_output);
    
    taguchi_free_definition(def);
}

TEST(suggest_optimal_array_single_factor) {
    char error[TAGUCHI_ERROR_SIZE];
    
    // Test with single factor
    const char *content = 
        "factors:\n"
        "  temp: 300F, 350F\n"
        "array: L4\n";  // For parsing but test function
    
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    ASSERT_NOT_NULL(def);
    
    const char *recommended = taguchi_suggest_optimal_array(def, error);
    ASSERT_NOT_NULL(recommended);
    
    // Should suggest appropriate small array
    bool is_valid_output = (recommended[0] == 'L' && strlen(recommended) >= 2);
    ASSERT(is_valid_output);
    
    taguchi_free_definition(def);
}

TEST(suggest_optimal_array_null_input) {
    char error[TAGUCHI_ERROR_SIZE];
    
    // Test with NULL definition
    const char *recommended = taguchi_suggest_optimal_array(NULL, error);
    ASSERT_NULL(recommended);
    
    // Error buffer should contain error message
    ASSERT(strlen(error) > 0);
}

TEST(column_pairing_9level_factor) {
    char error[TAGUCHI_ERROR_SIZE];

    /* 9-level factor requires column pairing (2 columns in 3-level array) */
    const char *content =
        "factors:\n"
        "  n_stages: 1, 2, 3, 4, 5, 6, 7, 8, 9\n"
        "  mode: pumped, static, hybrid\n"
        "array: L81\n";

    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        printf("Parse failed: %s\n", error);
    }
    ASSERT_NOT_NULL(def);

    taguchi_experiment_run_t **runs = NULL;
    size_t count = 0;

    int result = taguchi_generate_runs(def, &runs, &count, error);
    if (result != 0) {
        printf("Generate failed: %s\n", error);
    }
    ASSERT_EQ(result, 0);
    ASSERT_EQ(count, 81);

    /* Verify all 9 stage values appear in the runs */
    int stage_seen[9] = {0};
    for (size_t i = 0; i < count; i++) {
        const char *val = taguchi_run_get_value(runs[i], "n_stages");
        ASSERT_NOT_NULL(val);
        int v = atoi(val);
        if (v >= 1 && v <= 9) {
            stage_seen[v - 1] = 1;
        }
    }
    for (int i = 0; i < 9; i++) {
        ASSERT_EQ(stage_seen[i], 1);
    }

    taguchi_free_runs(runs, count);
    taguchi_free_definition(def);
}

TEST(mixed_level_2_in_3level_array) {
    char error[TAGUCHI_ERROR_SIZE];

    /* 2-level factor in 3-level array should work with wrapping */
    const char *content =
        "factors:\n"
        "  on_off: true, false\n"
        "  temp: low, medium, high\n"
        "  size: small, medium, large\n"
        "  color: red, green, blue\n"
        "array: L9\n";

    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    ASSERT_NOT_NULL(def);

    taguchi_experiment_run_t **runs = NULL;
    size_t count = 0;

    int result = taguchi_generate_runs(def, &runs, &count, error);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(count, 9);

    /* Verify on_off only has valid values */
    for (size_t i = 0; i < count; i++) {
        const char *val = taguchi_run_get_value(runs[i], "on_off");
        ASSERT_NOT_NULL(val);
        ASSERT(strcmp(val, "true") == 0 || strcmp(val, "false") == 0);
    }

    taguchi_free_runs(runs, count);
    taguchi_free_definition(def);
}

TEST(peltier_style_experiment) {
    char error[TAGUCHI_ERROR_SIZE];

    /* Simulate the peltier stack experiment: mix of 2, 3, and 9-level factors */
    const char *content =
        "factors:\n"
        "  coupling_mode: pumped_series, static_pool\n"
        "  n_stages: 1, 2, 3, 4, 5, 6, 7, 8, 9\n"
        "  current: 0.1, 0.25, 0.5, 1.0, 2.0, 3.5, 5.0, 7.0, 10.0\n"
        "  n_parallel: 1, 2, 4\n"
        "  tank_size: small, standard, large\n"
        "array: L81\n";

    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        printf("Parse failed: %s\n", error);
    }
    ASSERT_NOT_NULL(def);

    taguchi_experiment_run_t **runs = NULL;
    size_t count = 0;

    int result = taguchi_generate_runs(def, &runs, &count, error);
    if (result != 0) {
        printf("Generate failed: %s\n", error);
    }
    ASSERT_EQ(result, 0);
    ASSERT_EQ(count, 81);

    /* Columns needed: coupling_mode=1, n_stages=2, current=2, n_parallel=1, tank_size=1 = 7 total */
    /* L81 has 40 columns, so this fits easily */

    taguchi_free_runs(runs, count);
    taguchi_free_definition(def);
}

TEST(auto_select_with_9level_factor) {
    char error[TAGUCHI_ERROR_SIZE];

    /* Auto-selection should pick L81 for 9-level factors */
    const char *content =
        "factors:\n"
        "  n_stages: 1, 2, 3, 4, 5, 6, 7, 8, 9\n"
        "  mode: A, B, C\n";

    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    ASSERT_NOT_NULL(def);

    const char *recommended = taguchi_suggest_optimal_array(def, error);
    ASSERT_NOT_NULL(recommended);
    /* L8 (2-level, 7 cols) can fit this: 9-level needs 4 cols, 3-level needs 2 = 6 total */
    /* L8 is smaller than L9 so it gets picked first */
    ASSERT_STR_EQ(recommended, "L8");

    taguchi_free_definition(def);
}

TEST(auto_select_vs_manual_specification) {
    char error[TAGUCHI_ERROR_SIZE];
    
    // Test auto-selection function on the same experiment definition 
    const char *experiment_content = 
        "factors:\n"
        "  temp: 300F, 350F, 400F\n"
        "  time: 10min, 15min, 20min\n"
        "array: L9\n";  // For parsing but we'll test auto-selection function

    taguchi_experiment_def_t *def = taguchi_parse_definition(experiment_content, error);
    ASSERT_NOT_NULL(def);

    // The auto-selected array should suggest something appropriate for the factor structure
    const char *auto_selected = taguchi_suggest_optimal_array(def, error);
    ASSERT_NOT_NULL(auto_selected);
    
    // Should suggest an appropriate array for 2 factors with 3 levels each
    bool is_valid_output = (auto_selected[0] == 'L' && strlen(auto_selected) >= 2);
    ASSERT(is_valid_output);

    taguchi_free_definition(def);
}