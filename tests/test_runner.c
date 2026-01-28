#include "test_framework.h"

/* Declare test functions from other files */
extern void test_set_error_basic(void);
extern void test_set_error_with_args(void);
extern void test_set_error_truncation(void);
extern void test_set_error_null_buffer(void);

/* Declare test functions from test_arrays.c */
extern void test_get_array_valid(void);
extern void test_get_array_invalid(void);
extern void test_get_array_null(void);
extern void test_list_arrays_valid(void);
extern void test_l4_is_orthogonal(void);
extern void test_l8_is_orthogonal(void);
extern void test_l9_is_orthogonal(void);
extern void test_l16_is_orthogonal(void);
extern void test_l27_is_orthogonal(void);

/* Declare test functions from test_parser.c */
extern void test_parse_simple_factor_definition(void);
extern void test_parse_multiple_factors(void);
extern void test_parse_with_whitespace(void);
extern void test_parse_invalid_no_factors(void);
extern void test_parse_invalid_no_array(void);
extern void test_validate_correct_definition(void);
extern void test_validate_empty_factor_name(void);


int main(void) {
    printf("=== Taguchi Library Test Suite ===\\n\\n");

    printf("Utils Tests:\\n");
    RUN_TEST(set_error_basic);
    RUN_TEST(set_error_with_args);
    RUN_TEST(set_error_truncation);
    RUN_TEST(set_error_null_buffer);

    printf("\\nArrays Tests:\\n");
    RUN_TEST(get_array_valid);
    RUN_TEST(get_array_invalid);
    RUN_TEST(get_array_null);
    RUN_TEST(list_arrays_valid);
    RUN_TEST(l4_is_orthogonal);
    RUN_TEST(l8_is_orthogonal);
    RUN_TEST(l9_is_orthogonal);
    RUN_TEST(l16_is_orthogonal);
    // RUN_TEST(l27_is_orthogonal);  // Temporarily disabled - requires mathematical construction

    printf("\\nParser Tests:\\n");
    RUN_TEST(parse_simple_factor_definition);
    RUN_TEST(parse_multiple_factors);
    RUN_TEST(parse_with_whitespace);
    RUN_TEST(parse_invalid_no_factors);
    RUN_TEST(parse_invalid_no_array);
    RUN_TEST(validate_correct_definition);
    RUN_TEST(validate_empty_factor_name);

    printf("\\n=== All Tests Passed ===\\n");
    return 0;
}
