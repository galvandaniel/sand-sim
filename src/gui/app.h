/**
 * Primary module for displaying a sandbox using SDL3.
 */

#ifndef APP_H
#define APP_H

#include "gui/cursor.h"
#include "gui/brush.h"
#include "sim/sandbox.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdint.h>

/**
 * Arrays of length NUM_TILE_TYPES to NULL-terminated strings indicating 
 * relative filepaths of textures in use at the application-level.
 */
extern const char *TILE_TEXTURE_FILENAMES[];
extern const char *PANEL_TEXTURE_FILENAMES[];


/**
 * Type describing all critical components owned by a GUI application.
 */
struct Application
{
    // App GUI rendering components.
    SDL_Renderer *renderer;
    SDL_Window *window;

    // Sandbox owned and displayed by GUI application.
    struct Sandbox *sandbox;

    // Input data captured by GUI application.
    struct Cursor *cursor;

    // State of sandbox interactivity component, we call it 'Brush'.
    struct Brush *brush;

    // Event storage of incoming events to be handled.
    struct EventBuffer *event_buffer;

    // Time passed in milliseconds as an integer multiple of `Sandbox.lifetime`.
    // This member is not necessarily equal SDL_Ticks() at any given time.
    uint64_t logical_time;

    // Minimum size of app window in pixels, depends on sandbox size.
    // Actual window size may grow larger than these values.
    int min_window_width;
    int min_window_height;
};


/**
 * Allocate a Sandbox GUI application using SDL from the given sandbox
 * data.
 * 
 * This function will initialize SDL via `SDL_Init()` if not already.
 * This function will call exit() if any part of the app initialization fails.
 *
 * @param title NULL-terminated bytestring to name the application window.
 * @param sandbox Sandbox to be owned by the initialized GUI application.
 * @return Pointer to created GUI application which renders and owns the passed
 * sandbox object.
 * 
 */
struct Application *create_app(const char *title, struct Sandbox *sandbox);


/**
 * Destroy the sandbox application, freeing any memory it takes up.
 * 
 * Passing NULL to this functions does nothing
 * 
 * @param app Owning GUI application to free.
 */
void destroy_app(struct Application *app);


/**
 * Perform all logical state updates to app related to 1 update step.
 * 
 * This includes advancing sandbox simulation, flushing event buffer, reacting
 * to input events, etc.
 * 
 * If `app` is NULL, this function returns SDL_APP_FAILURE to signal shutdown.
 * 
 * If the event buffer contains an event signaling shutdown, this function
 * return SDL_APP_SUCCESS to signal graceful shutdown.
 * 
 * Otherwise, this function returns SDL_APP_SUCCESS.
 * 
 * @param app App to mutate with 1 update's worth of logical state updates.
 * @return AppResult indicating next app state after updating.
 */
SDL_AppResult app_update(struct Application *app);


/**
 * Display the given application: sandbox simulation and all GUI elements.
 * 
 * This function calls SDL_RenderPresent to display app contents, and therefore
 * assumes SDL has been initialized.
 * 
 * If `app` is NULL, this function does nothing.
 * 
 * @param app App to display.
 */
void app_render(struct Application *app);


/**
 * Poll the given SDL event for any input device activity and update the
 * GUI application state respectfully.
 *
 * @param app GUI Application whose data to update with respect to user input.
 * @param event Input event to update app state with respect to.
 * @return AppResult indicating next app state after processing event.
 * Can by any one of SDL_APP_CONTINUE, SDL_APP_SUCCESS, SDL_APP_FAILURE, etc.
 */
SDL_AppResult process_input_event(struct Application *app, SDL_Event *event);


/**
 * Mutate the given GUI application's owned sandbox with respect to the current
 * input state present in app.
 * 
 * Intended to be called together with get_input() to process user input.
 * 
 * @param app GUI application whose sandbox to update according to user input.
 */
void do_input_update(struct Application *app);


#endif // APP_H
