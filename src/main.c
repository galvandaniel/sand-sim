#include "gui.h"
#include "sandbox.h"

#include <SDL.h>
#include <stdlib.h>
#include <string.h>

// Preset sizes for sandbox, in tiles.
#define SANDBOX_SMALL_WIDTH 16
#define SANDBOX_SMALL_HEIGHT 9
#define SANDBOX_MEDIUM_WIDTH 80
#define SANDBOX_MEDIUM_HEIGHT 45
#define SANDBOX_LARGE_WIDTH 160
#define SANDBOX_LARGE_HEIGHT 90


static const char *APP_NAME = "Sandbox";


/**
 * @brief Print the CLI arguments of sand-sim.
 * 
 * @param binary_name Name of running executable.
 */
static void print_usage_string(char *binary_name)
{
    // Prefer SDL_Log over printf for portability. Does not require SDL_init.
    SDL_Log("\nSand Simulation, a simple sandbox simulation written in C using SDL2.\n"
            "Usage: %s [options]\n"
            "Options: \n"
            "  -h/--help \t This message.\n"
            "  --size \t Size preset of sandbox, either \"small\", \"medium\", or \"large\".\n"
            "  --width \t Set tile width of the sandbox. Overrides --size. If specified, height must be specified too.\n"
            "  --height \t Set tile height of the sandbox. Overrides --size. If specified, width must be specified too.\n",
            binary_name);
}


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
static void parse_args(int argc, char **argv, int *dimensions)
{
    int width_index = 0;
    int height_index = 1;

    dimensions[width_index] = 0;
    dimensions[height_index] = 0;

    // For no argments specified, default to medium-size sandbox.
    if (argc == 1)
    {
        dimensions[width_index] = SANDBOX_MEDIUM_WIDTH;
        dimensions[height_index] = SANDBOX_MEDIUM_HEIGHT;
    }

    // Print usage help string if all options are absent.
    if (argc == 2)
    {
        print_usage_string(argv[0]);
        exit(EXIT_FAILURE);
    }

    // Only time 3 arguments is acceptable is with supplied size argument.
    // If size argument does not confirm to any of the options, reject.
    if (argc == 3)
    {
        if (strcmp("--size", argv[1]) != 0)
        {
            print_usage_string(argv[0]);
            exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
        }

    }

    // Never valid if only 3 arguments are supplied.
    if (argc == 4)
    {
        print_usage_string(argv[0]);
        exit(EXIT_FAILURE);
    }


    if (argc == 5)
    {
        // Must specify width and height in correct order.
        if (strcmp("--height", argv[1]) == 0)
        {
            print_usage_string(argv[0]);
            exit(EXIT_FAILURE);  
        }

        // Must specify width and height flags.
        if (strcmp("--width", argv[1]) != 0 || strcmp("--height", argv[3]) != 0)
        {
            print_usage_string(argv[0]);
            exit(EXIT_FAILURE);     
        }

        // Attempt to parse width and height options.
        int user_width = atoi(argv[2]);
        int user_height = atoi(argv[4]);

        // Stop if atoi() failed to parse into valid integer within INT_MAX.
        if (user_width <= 0 || user_height <= 0)
        {
            print_usage_string(argv[0]);
            exit(EXIT_FAILURE);
        }

        dimensions[width_index] = user_width;
        dimensions[height_index] = user_height;
    }
}


/**
 * Entry point and main loop of sand-sim. 
 */
int main(int argc, char **argv)
{
    // Change the dimensions of sandbox w.r.t arguments.
    int sandbox_dimensions[2];
    parse_args(argc, argv, sandbox_dimensions);

    // Form a sandbox of user's desired dimensions.
    struct Sandbox *sandbox = create_sandbox(sandbox_dimensions[0], sandbox_dimensions[1]);

    // Initialize SDL, create an app, and load in textures.
    struct Application *app = init_gui(APP_NAME, sandbox);

    while (true)
    {
        set_black_background(app);

        // Update application state and its sandbox state w.r.t user input.
        get_input(app);
        handle_input(app);

        // Do 1 frame of sandbox processing and draw the result to the renderer.
        process_sandbox(sandbox);
        draw_sandbox(app);

        // Draw UI after the sandbox so that its above all tiles.
        draw_ui(app);

        // Display all rendered graphics.
        SDL_RenderPresent(app->renderer);

        // Run at ~30 FPS. (wait 33 milliseconds before proceeding to next frame)
        SDL_Delay(33);
    }

    return EXIT_SUCCESS;
}
