#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include "include/taguchi.h"

static void print_usage(const char *program_name) {
    fprintf(stderr, 
        "Usage: %s [OPTIONS] <command> [ARGS]\n"
        "\n"
        "Commands:\n"
        "  generate <file.tgu>     Generate experiment runs\n"
        "  run <file.tgu> <script> Execute experiments with external script\n"
        "  analyze <file.tgu> <results.csv> Analyze experimental results\n"
        "  effects <file.tgu> <results.csv> Calculate main effects\n"
        "  validate <file.tgu>     Validate experiment definition\n"
        "  list-arrays             List available orthogonal arrays\n"
        "  --help                  Show this help message\n"
        "  --version               Show version information\n"
        "\n"
        "Examples:\n"
        "  %s generate experiment.tgu\n"
        "  %s run experiment.tgu './my_script.sh'\n"
        "  %s analyze experiment.tgu results.csv --metric throughput\n",
        program_name, program_name, program_name, program_name);
}

static void print_version(void) {
    printf("Taguchi Array Tool v%d.%d.%d\n", 
           TAGUCHI_VERSION_MAJOR, TAGUCHI_VERSION_MINOR, TAGUCHI_VERSION_PATCH);
}

static int cmd_help(int argc, char *argv[]) {
    (void)argc; // Suppress unused parameter warning
    print_usage(argv[0]);
    return 0;
}

static int cmd_version(int argc, char *argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    print_version();
    return 0;
}

static int cmd_list_arrays(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    const char **arrays = taguchi_list_arrays();
    printf("Available orthogonal arrays:\n");
    for (int i = 0; arrays[i] != NULL; i++) {
        size_t rows, cols, levels;
        if (taguchi_get_array_info(arrays[i], &rows, &cols, &levels) == 0) {
            printf("  %-5s (%3zu runs, %3zu cols, %zu levels)\n",
                   arrays[i], rows, cols, levels);
        } else {
            printf("  %s\n", arrays[i]);
        }
    }
    return 0;
}

static int cmd_generate(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: generate command requires .tgu file\n");
        print_usage(argv[0]);
        return 1;
    }
    
    const char *filename = argv[1];
    
    // Read the file
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    
    char content[4096];
    size_t bytes_read = fread(content, 1, sizeof(content) - 1, file);
    fclose(file);
    
    if (bytes_read >= sizeof(content) - 1) {
        fprintf(stderr, "Error: file too large\n");
        return 1;
    }
    
    content[bytes_read] = '\0';
    
    // Parse the definition
    char error[TAGUCHI_ERROR_SIZE];
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        fprintf(stderr, "Error parsing file %s: %s\n", filename, error);
        return 1;
    }
    
    // Generate runs
    taguchi_experiment_run_t **runs = NULL;
    size_t count = 0;
    
    if (taguchi_generate_runs(def, &runs, &count, error) != 0) {
        fprintf(stderr, "Error generating runs: %s\n", error);
        taguchi_free_definition(def);
        return 1;
    }
    
    // Print runs with factor details
    printf("Generated %zu experiment runs:\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("Run %zu: ", taguchi_run_get_id(runs[i]));

        // Get factor count from the original definition to know how many to print
        size_t factor_count = taguchi_def_get_factor_count(def);

        // Print each factor-value pair
        for (size_t f = 0; f < factor_count; f++) {
            const char *factor_name = taguchi_def_get_factor_name(def, f);
            if (factor_name) {
                const char *factor_value = taguchi_run_get_value(runs[i], factor_name);
                if (factor_value) {
                    if (f > 0) printf(", "); // Comma separator except for first
                    printf("%s=%s", factor_name, factor_value);
                }
            }
        }
        printf("\n");
    }
    
    // Cleanup
    taguchi_free_runs(runs, count);
    taguchi_free_definition(def);
    
    return 0;
}

static int cmd_validate(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: validate command requires .tgu file\n");
        print_usage(argv[0]);
        return 1;
    }
    
    const char *filename = argv[1];
    
    // Read the file
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    
    char content[4096];
    size_t bytes_read = fread(content, 1, sizeof(content) - 1, file);
    fclose(file);
    
    if (bytes_read >= sizeof(content) - 1) {
        fprintf(stderr, "Error: file too large\n");
        return 1;
    }
    
    content[bytes_read] = '\0';
    
    // Parse the definition
    char error[TAGUCHI_ERROR_SIZE];
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        fprintf(stderr, "Error: Invalid .tgu file %s: %s\n", filename, error);
        return 1;
    }
    
    // Validate the definition
    if (!taguchi_validate_definition(def, error)) {
        fprintf(stderr, "Validation failed: %s\n", error);
        taguchi_free_definition(def);
        return 1;
    }
    
    printf("Valid .tgu file: %s\n", filename);
    taguchi_free_definition(def);
    
    return 0;
}

// Command to run experiments with external script
static int cmd_run(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Error: run command requires .tgu file and script\n");
        print_usage(argv[0]);
        return 1;
    }
    
    const char *tgu_file = argv[1];
    const char *script = argv[2];
    
    // Read the .tgu file
    FILE *file = fopen(tgu_file, "r");
    if (!file) {
        perror("Error opening .tgu file");
        return 1;
    }
    
    char content[4096];
    size_t bytes_read = fread(content, 1, sizeof(content) - 1, file);
    fclose(file);
    
    if (bytes_read >= sizeof(content) - 1) {
        fprintf(stderr, "Error: .tgu file too large\n");
        return 1;
    }
    
    content[bytes_read] = '\0';
    
    // Parse the definition
    char error[TAGUCHI_ERROR_SIZE];
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        fprintf(stderr, "Error parsing .tgu file %s: %s\n", tgu_file, error);
        return 1;
    }
    
    // Generate runs
    taguchi_experiment_run_t **runs = NULL;
    size_t count = 0;
    
    if (taguchi_generate_runs(def, &runs, &count, error) != 0) {
        fprintf(stderr, "Error generating runs: %s\n", error);
        taguchi_free_definition(def);
        return 1;
    }
    
    // Execute each run as a separate process
    printf("Executing %zu experiment runs using '%s'...\n", count, script);
    
    for (size_t i = 0; i < count; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process: set environment variables and run the script

            // Set run ID as environment variable
            char run_id_env[64];
            snprintf(run_id_env, sizeof(run_id_env), "TAGUCHI_RUN_ID=%zu", taguchi_run_get_id(runs[i]));
            putenv(strdup(run_id_env));

            // Set environment variables for each factor-value pair
            size_t factor_count = taguchi_run_get_factor_count(runs[i]);
            for (size_t f = 0; f < factor_count; f++) {
                const char *factor_name = taguchi_run_get_factor_name_at_index(runs[i], f);
                const char *factor_value = taguchi_run_get_value(runs[i], factor_name);

                if (factor_name && factor_value) {
                    // Set environment variable (uppercase name with "TAGUCHI_" prefix)
                    char env_var[256];
                    snprintf(env_var, sizeof(env_var), "TAGUCHI_%s=%s", factor_name, factor_value);
                    putenv(strdup(env_var));
                }
            }

            // Execute the script
            execl("/bin/sh", "sh", "-c", script, (char *)NULL);
            
            // If execl returns, it failed
            perror("exec failed");
            exit(1);
        } else if (pid > 0) {
            // Parent process: wait for child process
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printf("Run %zu completed with exit code %d\n", taguchi_run_get_id(runs[i]), exit_code);
            } else {
                printf("Run %zu terminated abnormally\n", taguchi_run_get_id(runs[i]));
            }
        } else {
            // Fork failed
            perror("fork failed");
            taguchi_free_runs(runs, count);
            taguchi_free_definition(def);
            return 1;
        }
    }
    
    // Cleanup
    taguchi_free_runs(runs, count);
    taguchi_free_definition(def);
    
    printf("All experiment runs completed.\n");
    return 0;
}

/* Helper: read a file into a stack buffer */
static int read_file_content(const char *filename, char *buf, size_t buf_size) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }
    size_t bytes_read = fread(buf, 1, buf_size - 1, file);
    fclose(file);
    if (bytes_read >= buf_size - 1) {
        fprintf(stderr, "Error: file too large\n");
        return -1;
    }
    buf[bytes_read] = '\0';
    return 0;
}

/*
 * Parse a CSV results file. Expected format:
 *   run_id,response_value
 *   1,42.5
 *   2,38.1
 *   ...
 *
 * Lines starting with '#' are treated as comments. The header line is skipped.
 */
static int parse_csv_results(const char *filename, taguchi_result_set_t *results, char *error_buf) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        snprintf(error_buf, TAGUCHI_ERROR_SIZE, "Cannot open results file: %s", filename);
        return -1;
    }

    char line[1024];
    int line_num = 0;
    int data_lines = 0;

    while (fgets(line, sizeof(line), file)) {
        line_num++;
        /* Trim trailing newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }

        /* Skip empty lines and comments */
        if (len == 0 || line[0] == '#') continue;

        /* Skip header line */
        if (line_num == 1 && (strstr(line, "run_id") || strstr(line, "run"))) continue;

        /* Parse run_id,response */
        char *comma = strchr(line, ',');
        if (!comma) {
            snprintf(error_buf, TAGUCHI_ERROR_SIZE, "Invalid CSV format at line %d", line_num);
            fclose(file);
            return -1;
        }

        *comma = '\0';
        char *endptr;
        long run_id = strtol(line, &endptr, 10);
        if (*endptr != '\0' || run_id < 1) {
            snprintf(error_buf, TAGUCHI_ERROR_SIZE, "Invalid run_id at line %d", line_num);
            fclose(file);
            return -1;
        }

        double response = strtod(comma + 1, &endptr);
        if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r') {
            snprintf(error_buf, TAGUCHI_ERROR_SIZE, "Invalid response value at line %d", line_num);
            fclose(file);
            return -1;
        }

        if (taguchi_add_result(results, (size_t)run_id, response, error_buf) != 0) {
            fclose(file);
            return -1;
        }
        data_lines++;
    }

    fclose(file);

    if (data_lines == 0) {
        snprintf(error_buf, TAGUCHI_ERROR_SIZE, "No data rows found in %s", filename);
        return -1;
    }

    return 0;
}

static int cmd_effects(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Error: effects command requires .tgu file and results CSV\n");
        fprintf(stderr, "Usage: effects <file.tgu> <results.csv> [--metric name]\n");
        return 1;
    }

    const char *tgu_file = argv[1];
    const char *csv_file = argv[2];
    const char *metric_name = "response";

    /* Parse optional --metric flag */
    for (int i = 3; i < argc - 1; i++) {
        if (strcmp(argv[i], "--metric") == 0) {
            metric_name = argv[i + 1];
            break;
        }
    }

    char content[4096];
    if (read_file_content(tgu_file, content, sizeof(content)) != 0) return 1;

    char error[TAGUCHI_ERROR_SIZE];
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        fprintf(stderr, "Error parsing %s: %s\n", tgu_file, error);
        return 1;
    }

    taguchi_result_set_t *results = taguchi_create_result_set(def, metric_name);
    if (!results) {
        fprintf(stderr, "Error creating result set\n");
        taguchi_free_definition(def);
        return 1;
    }

    if (parse_csv_results(csv_file, results, error) != 0) {
        fprintf(stderr, "Error reading results: %s\n", error);
        taguchi_free_result_set(results);
        taguchi_free_definition(def);
        return 1;
    }

    taguchi_main_effect_t **effects = NULL;
    size_t effect_count = 0;
    if (taguchi_calculate_main_effects(results, &effects, &effect_count, error) != 0) {
        fprintf(stderr, "Error calculating effects: %s\n", error);
        taguchi_free_result_set(results);
        taguchi_free_definition(def);
        return 1;
    }

    /* Print main effects table */
    printf("Main Effects for metric: %s\n", metric_name);
    printf("%-20s %8s   Level Means\n", "Factor", "Range");
    printf("%-20s %8s   -----------\n", "------", "-----");

    for (size_t i = 0; i < effect_count; i++) {
        const char *name = taguchi_effect_get_factor(effects[i]);
        double range = taguchi_effect_get_range(effects[i]);
        size_t level_count = 0;
        const double *means = taguchi_effect_get_level_means(effects[i], &level_count);

        printf("%-20s %8.3f   ", name, range);
        for (size_t lv = 0; lv < level_count; lv++) {
            if (lv > 0) printf(", ");
            printf("L%zu=%.3f", lv + 1, means[lv]);
        }
        printf("\n");
    }

    taguchi_free_effects(effects, effect_count);
    taguchi_free_result_set(results);
    taguchi_free_definition(def);
    return 0;
}

static int cmd_analyze(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Error: analyze command requires .tgu file and results CSV\n");
        fprintf(stderr, "Usage: analyze <file.tgu> <results.csv> [--metric name] [--minimize]\n");
        return 1;
    }

    const char *tgu_file = argv[1];
    const char *csv_file = argv[2];
    const char *metric_name = "response";
    bool higher_is_better = true;

    /* Parse optional flags */
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--metric") == 0 && i + 1 < argc) {
            metric_name = argv[++i];
        } else if (strcmp(argv[i], "--minimize") == 0) {
            higher_is_better = false;
        }
    }

    char content[4096];
    if (read_file_content(tgu_file, content, sizeof(content)) != 0) return 1;

    char error[TAGUCHI_ERROR_SIZE];
    taguchi_experiment_def_t *def = taguchi_parse_definition(content, error);
    if (!def) {
        fprintf(stderr, "Error parsing %s: %s\n", tgu_file, error);
        return 1;
    }

    taguchi_result_set_t *results = taguchi_create_result_set(def, metric_name);
    if (!results) {
        fprintf(stderr, "Error creating result set\n");
        taguchi_free_definition(def);
        return 1;
    }

    if (parse_csv_results(csv_file, results, error) != 0) {
        fprintf(stderr, "Error reading results: %s\n", error);
        taguchi_free_result_set(results);
        taguchi_free_definition(def);
        return 1;
    }

    taguchi_main_effect_t **effects = NULL;
    size_t effect_count = 0;
    if (taguchi_calculate_main_effects(results, &effects, &effect_count, error) != 0) {
        fprintf(stderr, "Error calculating effects: %s\n", error);
        taguchi_free_result_set(results);
        taguchi_free_definition(def);
        return 1;
    }

    /* Print effects summary */
    printf("Analysis for metric: %s (%s)\n\n",
           metric_name, higher_is_better ? "maximizing" : "minimizing");

    printf("Main Effects:\n");
    printf("%-20s %8s   Level Means\n", "Factor", "Range");
    printf("%-20s %8s   -----------\n", "------", "-----");

    for (size_t i = 0; i < effect_count; i++) {
        const char *name = taguchi_effect_get_factor(effects[i]);
        double range = taguchi_effect_get_range(effects[i]);
        size_t level_count = 0;
        const double *means = taguchi_effect_get_level_means(effects[i], &level_count);

        printf("%-20s %8.3f   ", name, range);
        for (size_t lv = 0; lv < level_count; lv++) {
            if (lv > 0) printf(", ");
            printf("L%zu=%.3f", lv + 1, means[lv]);
        }
        printf("\n");
    }

    /* Print recommendation */
    char recommendation[1024];
    if (taguchi_recommend_optimal((const taguchi_main_effect_t **)effects, effect_count,
                                   higher_is_better, recommendation, sizeof(recommendation)) == 0) {
        printf("\nOptimal Configuration: %s\n", recommendation);
    }

    taguchi_free_effects(effects, effect_count);
    taguchi_free_result_set(results);
    taguchi_free_definition(def);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *command = argv[1];
    
    // Shift arguments for command handlers
    int sub_argc = argc - 1;
    char **sub_argv = argv + 1;
    
    if (strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    } else if (strcmp(command, "--version") == 0 || strcmp(command, "-v") == 0) {
        print_version();
        return 0;
    } else if (strcmp(command, "help") == 0) {
        return cmd_help(sub_argc, sub_argv);
    } else if (strcmp(command, "version") == 0) {
        return cmd_version(sub_argc, sub_argv);
    } else if (strcmp(command, "list-arrays") == 0) {
        return cmd_list_arrays(sub_argc, sub_argv);
    } else if (strcmp(command, "generate") == 0) {
        return cmd_generate(sub_argc, sub_argv);
    } else if (strcmp(command, "validate") == 0) {
        return cmd_validate(sub_argc, sub_argv);
    } else if (strcmp(command, "run") == 0) {
        return cmd_run(sub_argc, sub_argv);
    } else if (strcmp(command, "analyze") == 0) {
        return cmd_analyze(sub_argc, sub_argv);
    } else if (strcmp(command, "effects") == 0) {
        return cmd_effects(sub_argc, sub_argv);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}