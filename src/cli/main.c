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
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    const char **arrays = taguchi_list_arrays();
    printf("Available orthogonal arrays:\n");
    for (int i = 0; arrays[i] != NULL; i++) {
        printf("  %s\n", arrays[i]);
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
    
    // Print runs in simple format
    printf("Generated %zu experiment runs:\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("Run %zu: ", taguchi_run_get_id(runs[i]));
        
        // For each factor, print its value
        // For now, we'll just say "Run details available" - we can improve this later
        printf("Run generated successfully");
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
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}