/**
 * Primary module for displaying a sandbox using SDL3.
 */

#ifndef APP_H
#define APP_H

#include "cursor.h"
#include "brush.h"
#include "sandbox.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

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

    // Input data captured by GUI application.
    struct Cursor *cursor;

    // State of sandbox interactivity component, we call it 'Brush'.
    struct Brush *brush;

    // Sandbox owned and displayed by GUI application.
    struct Sandbox *sandbox;

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
 * Quit the sandbox application, freeing any memory it takes up, shutting down
 * any libraries initialized by init_gui(), and exiting the running process
 * gracefully.
 * 
 * @param app Owning GUI application to shutdown and free.
 */
void quit_app(struct Application *app);


/**
 * Given the filename for a location to a JPG or PNG, load the image as an
 * SDL_Texture on the given application.
 *
 * @param app App to load image on.
 * @param filename Filepath of image to load from, relative to program location.
 * @return Image loaded as SDL_Texture
 */
SDL_Texture *load_texture(struct Application *app, const char *filename);



/**
 * Given the filename of a JPG or PNG, load the image as an SDL_Texture on the
 * given application with the given alpha value set.
 * 
 * The loaded texture will have alpha blending enabled for drawing with opacity.
 * This will force the resulting texture to have an SDL pixel format which
 * supports an alpha channel.
 * 
 * @param app App to load image on.
 * @param filename Filepath of image to load from, relative to program location.
 * @param alpha Alpha value of returned SDL_Texture for displaying with opacity.
 * @return Transparent image loaded as SDL_Texture.
 */
SDL_Texture *load_texture_alpha(struct Application *app, const char *filename, unsigned char alpha);



/**
 * Draw the given SDL texture on the given app at the given screen coordinates
 *
 * @param app Application to draw texture on.
 * @param texture Loaded texture to draw.
 * @param window_coords x,y screen coordinates to draw texture at.
 */
void blit_texture(struct Application *app, SDL_Texture *texture, SDL_FPoint window_coords);


/**
 * Draw a rectangle of the given dimensions and RGBA value to the screen of
 * the given GUI application at the rectangle's (x,y) window coordinates.
 * 
 * @param app Application to draw rectangle on.
 * @param rect Dimensions of rectangle to draw packed in an SDL_FRect.
 * @param color RGBA color of rectangle packed as an SDL_Color.
 * @param do_fill If true, rectangle is filled with color. If false, only
 * outline of rectangle with color is drawn.
 */
void blit_rectangle(struct Application *app, SDL_FRect rect, SDL_Color color, bool do_fill);


/**
 * Setup a black background on the given application.
 *
 * This background can be used as a base for further blitting.
 *
 * @param app App whose background to set.
 */
void set_black_background(struct Application *app);


/**
 * Draw a passed GUI application's currently owned sandbox, where each tile 
 * represents one pixel onscreen, scaled in size according to TILE_SCALE.
 *
 * SDL_RenderPresent() is NOT called inside this function.
 *
 * @param app App whose owned sandbox will be drawn.
 */
void draw_sandbox(struct Application *app);


/**
 * Draw the tile located at the given sandbox coordinates which lives inside
 * the passed GUI application's owned sandbox to the application's screen.
 * 
 * If the tile located at the given indices is empty, this function does
 * nothing.
 * 
 * @param app GUI application to draw tile on.
 * @param coords Coordinates of some tile located inside the sandbox owned by
 * the passed app.
 */
void draw_tile(struct Application *app, struct SandboxPoint coords);


/**
 * Render sand-sim UI elements to the given app which display ontop of sandbox
 * simulation render.
 *
 * @param app App to render UI elements to.
 *
 */
void draw_ui(struct Application *app);


/**
 * Poll SDL for any input device activity and update the
 * GUI application state respectfully.
 *
 * @param app GUI Application whose data to update with respect to user input.
 */
void get_input(struct Application *app);


/**
 * Mutate the given GUI application's owned sandbox with respect to the current
 * input state present in app.
 * 
 * Intended to be called together with get_input() to process user input.
 * 
 * @param app GUI application whose sandbox to update according to user input.
 */
void handle_input(struct Application *app);


#endif // APP_H
