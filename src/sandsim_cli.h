/**
 * Module for parsing and handling CLI arguments which control sand-sim.
 */

#ifndef SANDSIM_CLI_H
#define SANDSIM_CLI_H

#include <SDL3/SDL.h>

// Preset sizes for sandbox and size limits, in terms of tiles.
static constexpr int SANDBOX_SMALL_WIDTH = 53;
static constexpr int SANDBOX_SMALL_HEIGHT = 30;
static constexpr int SANDBOX_MEDIUM_WIDTH = 80;
static constexpr int SANDBOX_MEDIUM_HEIGHT = 45;
static constexpr int SANDBOX_LARGE_WIDTH = 160;
static constexpr int SANDBOX_LARGE_HEIGHT = 90;

/**
 * Give argument numbers names which represent what the expected inputs are.
 */
enum num_args {
    NO_ARG = 1,
    HELP_ARG,
    SIZE_ARG,
    THREE_ARG,
    DIM_ARG,
};


/**
 * @brief Print the CLI arguments of sand-sim.
 * 
 * @param binary_name Name of running executable.
 */
void print_usage_string(char *binary_name);


/**
 * Parse the command line arguments passed, manipulating the dimensions of the
 * sandbox, placing the result into the given output integer array.
 * 
 * The given output integer array must be of size at least 2, otherwise
 * undefined behavior results.
 * 
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param dimensions Out-parameter to write parsed dimensions to, written
 * in (width, height) order. If parse fails, (0, 0) is written.
 */
void parse_args(int argc, char **argv, int *dimensions);

#endif // SANDSIM_CLI_H