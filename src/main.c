#include "gui.h"
#include "sandbox.h"

#include <SDL.h>
#include <stdlib.h>
#include <string.h>


const char *APP_NAME = "Sandbox";


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
 * sandbox.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
static void parse_args(int argc, char **argv)
{
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
            SANDBOX_WIDTH = SANDBOX_SMALL_WIDTH;
            SANDBOX_HEIGHT = SANDBOX_SMALL_HEIGHT;
        }
        else if (strcmp("medium", argv[2]) == 0)
        {
            SANDBOX_WIDTH = SANDBOX_MEDIUM_WIDTH;
            SANDBOX_HEIGHT = SANDBOX_MEDIUM_HEIGHT;
        }
        else if (strcmp("large", argv[2]) == 0)
        {
            SANDBOX_WIDTH = SANDBOX_LARGE_WIDTH;
            SANDBOX_HEIGHT = SANDBOX_LARGE_HEIGHT;
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

        // Stop if atoi() failed.
        if (user_width == 0 || user_height == 0)
        {
            print_usage_string(argv[0]);
            exit(EXIT_FAILURE);
        }

        SANDBOX_WIDTH = user_width;
        SANDBOX_HEIGHT = user_height;
    }
}


/**
 * Entry point and main loop of sand-sim. 
 */
int main(int argc, char **argv)
{
    // Change the dimensions of sandbox w.r.t arguments.
    parse_args(argc, argv);

    // Initialize SDL, create an app, and load in textures.
    struct Application *app = init_gui(APP_NAME);

    // Form a sandbox.
    unsigned char **sandbox = create_sandbox(SANDBOX_HEIGHT, SANDBOX_WIDTH);

    while (true)
    {
        // Render full black to the window.
        set_black_background(app);

        get_input(app);

        if (app -> mouse -> is_left_clicking)
        {
            place_tile(app -> mouse, sandbox, SANDBOX_HEIGHT, SANDBOX_WIDTH);
        }

        // Do 1 frame of sandbox processing and draw the result to the renderer.
        process_sandbox(sandbox, SANDBOX_HEIGHT, SANDBOX_WIDTH);
        draw_sandbox(app, sandbox, SANDBOX_HEIGHT, SANDBOX_WIDTH);

        // Draw UI elements above the sandbox so that they aren't covered.
        draw_ui(app);

        // Display all rendered graphics.
        SDL_RenderPresent(app -> renderer);

        // Run at ~30 FPS. (wait 33 milliseconds before proceeding to next frame)
        SDL_Delay(33);
    }

    return EXIT_SUCCESS;
}
