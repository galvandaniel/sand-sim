/**
 * Application entrypoint of sand-sim.
 */

#include "sandsim_cli.h"
#include "gui.h"
#include "sandbox.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdlib.h>

static const char *APP_NAME = "sand-sim";

// Time waited before proceeding to next frame.
static constexpr int MILLISECONDS_BETWEEN_FRAMES = 33;

int main(int argc, char **argv)
{
    // Change the dimensions of sandbox w.r.t arguments.
    int sandbox_dimensions[2];
    parse_args(argc, argv, sandbox_dimensions);

    // Form a sandbox of user's desired dimensions along with GUI app which
    // owns this sandbox.
    struct Sandbox *sandbox = create_sandbox(sandbox_dimensions[0], sandbox_dimensions[1]);
    struct Application *app = init_gui(APP_NAME, sandbox);

    while (true)
    {
        // Do 1 frame of sandbox processing before any input is obtained..
        process_sandbox(sandbox);

        // Update application state and its sandbox state w.r.t user input.
        get_input(app);
        handle_input(app);

        // Clear canvas, draw sandbox, then draw UI above sandbox.
        set_black_background(app);
        draw_sandbox(app);
        draw_ui(app);

        // Display all rendered graphics.
        SDL_RenderPresent(app->renderer);
        SDL_Delay(MILLISECONDS_BETWEEN_FRAMES);
    }

    return EXIT_SUCCESS;
}
