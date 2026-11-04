/**
 * Application entrypoint of sand-sim.
 * 
 * sand-sim's application loop is handled by SDL3's callback functions, rather
 * than a traditional main() entrypoint.
 */

#include "gui/sdl_utils.h"
#include "sandsim_cli.h"
#include "gui/app.h"
#include "gui/event_buffer.h"
#include "sim/sandbox.h"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdint.h>

// App aims to perform 40 state updates per second, but can render at any FPS.
static constexpr uint64_t ONE_SECOND_IN_MS = 1000;
static constexpr uint64_t TARGET_FPS = 40;
static constexpr uint64_t MS_BETWEEN_UPDATES = ONE_SECOND_IN_MS / TARGET_FPS;

static const char *APP_NAME = "sand-sim";

/**
 * Application entrypoint. 
 * 
 * Followed by SDL_AppEvent() and SDL_AppIterate() operating concurrently,
 * until eventually SDL_AppQuit() is called to exit application. 
 */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    // Change the dimensions of sandbox according to arguments.
    int sandbox_dimensions[2];
    bool successful_parse = parse_args(argc, argv, sandbox_dimensions);

    if (!successful_parse)
    {
        return SDL_APP_FAILURE;
    }

    // Form a sandbox of desired dimensions alongside owning GUI app.
    struct Sandbox *sandbox = create_sandbox(sandbox_dimensions[0], sandbox_dimensions[1]);
    struct Application *app = create_app(APP_NAME, sandbox);

    // Preserve GUI app state so other SDL callback functions can mutate and read.
    *appstate = app;
    return SDL_APP_CONTINUE;
}


/**
 * Application main loop. This function handles all logic updates and drawing 
 * for 1 frame of simulation.
 * 
 * Called concurrently with SDL_AppEvent().
 */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    struct Application *app = appstate;
    uint64_t current_time = SDL_GetTicks();
    
    // If VSync is enabled, let VSync determine when renderer is ready.
    // Otherwise, fallback to waiting manually for next frame.
    if (app->logical_time > current_time && !is_vsync_enabled(app->renderer))
    {
        SDL_Delay((uint32_t) (app->logical_time - current_time));
        return SDL_APP_CONTINUE;
    }

    while (app->logical_time <= current_time)
    {
        // If input handling signaled for shutdown, don't bother rendering.
        SDL_AppResult update_result = app_update(app);
        if (update_result == SDL_APP_FAILURE || update_result == SDL_APP_SUCCESS)
        {
            return update_result;
        }
        app->logical_time += MS_BETWEEN_UPDATES;
    }

    app_render(app);
    return SDL_APP_CONTINUE;
}


/**
 * Application event handler. Called for every queued SDL_Event.
 * 
 * Called concurrently with SDL_AppIterate() and SDL_AppQuit(). 
 * 
 * This function must be thread safe, as SDL makes no guarantee about the thread
 * this function is called from.
 * 
 * Although SDL docs advise that under 'normal operation' SDL itself serially 
 * pushes events on main thread, this function must still be thread safe to 
 * maintain maximal safety and cross-platform compatability. 
 */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    // Atomically pipe events for handling by SDL_AppIterate() to allow for
    // synchronous app logic updates with respect to events.
    struct Application *app = appstate;
    event_buffer_append(app->event_buffer, *event);
    return SDL_APP_CONTINUE;
}


/**
 * Application exit logic. This function handles all memory cleanup and
 * destruction.
 * 
 * Called concurrently with SDL_AppEvent().
 */
void SDL_AppQuit([[maybe_unused]]void *appstate, [[maybe_unused]] SDL_AppResult result)
{
    /**
     * Purposefully 'leak' app memory, let the OS clean up for us.
     * 
     * Appstate memory is allocated only once for sand-sim lifetime, so no need
     * to worry about out-of-memory errors from repeated allocations.
     * 
     * This way, no need to worry that a concurrent call to SDL_AppEvent() in
     * between SDL_AppQuit() execution tries to access appstate and cause UB
     * after appstate memory has already been freed by SDL_AppQuit().
     * 
     * In other words: By letting OS kernel itself free Appstate memory, it is
     * guaranteed that no thread is actively executing SDL_AppEvent().
     * 
     * REMARK: To safely use destroy_app(), try using a semaphore?
     */
}