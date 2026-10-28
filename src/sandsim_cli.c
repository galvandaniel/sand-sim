#include "sandsim_cli.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>


void print_usage_string(char *binary_name)
{
    // Prefer SDL_Log over printf for portability. Does not require SDL_init.
    SDL_Log("Sand Simulation, a simple sandbox simulation written in C using SDL3.\n"
            "\nUsage: %s [options]\n"
            "Options: \n"
            "  -h/--help \t This message.\n"
            "  --size \t Size preset of sandbox, either \"small\", \"medium\", or \"large\".\n"
            "  --width \t Set tile width of the sandbox. Overrides --size. If specified, height must be specified too.\n"
            "  --height \t Set tile height of the sandbox. Overrides --size. If specified, width must be specified too.\n",
            binary_name);
}


bool parse_args(int argc, char **argv, int *dimensions)
{
    int width_index = 0;
    int height_index = 1;

    dimensions[width_index] = 0;
    dimensions[height_index] = 0;

    // Default to medium-size sandbox.
    if (argc == NO_ARG)
    {
        dimensions[width_index] = SANDBOX_MEDIUM_WIDTH;
        dimensions[height_index] = SANDBOX_MEDIUM_HEIGHT;
    }

    // Just one argument is never valid, assume help string.
    if (argc == HELP_ARG)
    {
        print_usage_string(argv[0]);
        return false;
    }

    // If size argument does not confirm to any of the options, reject.
    if (argc == SIZE_ARG)
    {
        if (strcmp("--size", argv[1]) != 0)
        {
            print_usage_string(argv[0]);
            return false;
        }

        if (strcmp("small", argv[2]) == 0)
        {
            dimensions[width_index] = SANDBOX_SMALL_WIDTH;
            dimensions[height_index] = SANDBOX_SMALL_HEIGHT;
        }
        else if (strcmp("medium", argv[2]) == 0)
        {
            dimensions[width_index] = SANDBOX_MEDIUM_WIDTH;
            dimensions[height_index] = SANDBOX_MEDIUM_HEIGHT;
        }
        else if (strcmp("large", argv[2]) == 0)
        {
            dimensions[width_index] = SANDBOX_LARGE_WIDTH;
            dimensions[height_index] = SANDBOX_LARGE_HEIGHT;
        }
        else
        {
            print_usage_string(argv[0]);
            return false;
        }
    }

    // Never valid if only 3 arguments are supplied.
    if (argc == THREE_ARG)
    {
        print_usage_string(argv[0]);
        return false;
    }

    if (argc == DIM_ARG)
    {
        // Must specify width and height in correct order.
        if (strcmp("--height", argv[1]) == 0)
        {
            print_usage_string(argv[0]);
            return false;
        }

        // Must specify BOTH width and height flags.
        if (strcmp("--width", argv[1]) != 0 || strcmp("--height", argv[3]) != 0)
        {
            print_usage_string(argv[0]);
            return false; 
        }

        // Attempt to parse width and height options as base-10 numbers.
        int user_width = (int) strtol(argv[2], nullptr, 0);
        int user_height = (int) strtol(argv[4], nullptr, 0);

        // Stop if atoi() failed to parse into valid integer within INT_MAX.
        if (user_width <= 0 || user_height <= 0)
        {
            print_usage_string(argv[0]);
            return false;
        }

        // Auto scale within bounds of smallest and largest dimensions.
        dimensions[width_index] = clamp(user_width, SANDBOX_SMALL_WIDTH, SANDBOX_LARGE_WIDTH);
        dimensions[height_index] = clamp(user_height, SANDBOX_SMALL_HEIGHT, SANDBOX_LARGE_HEIGHT);
    }

    return true;
}
