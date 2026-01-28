#include "test_framework.h"
#include "src/lib/arrays.h"

TEST(get_array_valid) {
    const OrthogonalArray *array = get_array("L4");
    ASSERT_NOT_NULL(array);
    ASSERT_STR_EQ(array->name, "L4");
    ASSERT_EQ(array->rows, 4);
    ASSERT_EQ(array->cols, 3);
    ASSERT_EQ(array->levels, 2);
}

TEST(get_array_invalid) {
    const OrthogonalArray *array = get_array("L5");
    ASSERT_NULL(array);
}

TEST(get_array_null) {
    const OrthogonalArray *array = get_array(NULL);
    ASSERT_NULL(array);
}

TEST(list_arrays_valid) {
    const char **names = list_array_names();
    ASSERT_NOT_NULL(names);
    ASSERT_STR_EQ(names[0], "L4");
    ASSERT_STR_EQ(names[1], "L8");
    ASSERT_STR_EQ(names[2], "L9");
    ASSERT_STR_EQ(names[3], "L16");
    ASSERT_STR_EQ(names[4], "L27");
    ASSERT_NULL(names[5]);
}

// Helper function to check the balance property of an orthogonal array
void check_orthogonality(const OrthogonalArray *array) {
    if (array->cols < 2) {
        return; // Nothing to check
    }

    size_t levels = array->levels;
    size_t rows = array->rows;
    const int *data = array->data;

    // The number of times each pair of levels should appear
    size_t expected_count = rows / (levels * levels);

    // Check every pair of columns
    for (size_t c1 = 0; c1 < array->cols; c1++) {
        for (size_t c2 = c1 + 1; c2 < array->cols; c2++) {
            
            // Initialize a counter for each pair of levels
            size_t pair_counts[levels][levels];
            for (size_t i = 0; i < levels; i++) {
                for (size_t j = 0; j < levels; j++) {
                    pair_counts[i][j] = 0;
                }
            }

            // Count the occurrences of each pair of levels
            for (size_t r = 0; r < rows; r++) {
                int level1 = data[r * array->cols + c1];
                int level2 = data[r * array->cols + c2];
                pair_counts[level1][level2]++;
            }

            // Check if each pair count is correct
            for (size_t i = 0; i < levels; i++) {
                for (size_t j = 0; j < levels; j++) {
                    if (pair_counts[i][j] != expected_count) {
                        printf("Orthogonality check failed for array %s\n", array->name);
                        printf("Columns: %zu and %zu\n", c1, c2);
                        printf("Levels: %zu and %zu\n", i, j);
                        printf("Expected count: %zu, but got: %zu\n", expected_count, pair_counts[i][j]);
                    }
                    ASSERT_EQ(pair_counts[i][j], expected_count);
                }
            }
        }
    }
}


TEST(l4_is_orthogonal) {
    const OrthogonalArray *array = get_array("L4");
    check_orthogonality(array);
}

TEST(l8_is_orthogonal) {
    const OrthogonalArray *array = get_array("L8");
    check_orthogonality(array);
}

TEST(l9_is_orthogonal) {
    const OrthogonalArray *array = get_array("L9");
    check_orthogonality(array);
}

TEST(l16_is_orthogonal) {
    const OrthogonalArray *array = get_array("L16");
    check_orthogonality(array);
}

TEST(l27_is_orthogonal) {
    const OrthogonalArray *array = get_array("L27");
    check_orthogonality(array);
}
