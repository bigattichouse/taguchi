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

    // Create content with 16 2-level factors (more than L16 can handle - L16 supports max 15 factors)
    const char *content =
        "factors:\n"
        "  f1: A, B\n  f2: A, B\n  f3: A, B\n  f4: A, B\n  f5: A, B\n"
        "  f6: A, B\n  f7: A, B\n  f8: A, B\n  f9: A, B\n  f10: A, B\n"
        "  f11: A, B\n  f12: A, B\n  f13: A, B\n  f14: A, B\n  f15: A, B\n"
        "  f16: A, B\n"  // 16 factors - exceeds L16's 15 factor limit
        "array: L16\n";  // This would fail because only 15 factors allowed in L16

    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        printf("Parse error: %s\\n", error);
    }
    ASSERT_NOT_NULL(def);

    const char *recommended = taguchi_suggest_optimal_array(def, error);
    // Since 16 factors exceed what any 2-level array supports (max 15 in L16), should return NULL
    ASSERT_NULL(recommended);

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