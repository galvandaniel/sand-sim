#ifndef GUI_H
#define GUI_H

/**
 * A collection of functions for displaying a sand simulation using SDL2 and
 * the logic presented in sandbox.h
 * 
 * If any API call to SDL2 fails, the functions defined here will call exit()
 * and quit the running program.
 */

#include "sandbox.h"

#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

/**
 * Scale-factor which determines how many screen pixels it takes to draw 1 side 
 * of 1 particle tile in the sandbox.
 * Ex: TILE_SCALE = 8 indicates 1 tile particle takes up 64 screen pixels.
 * 
 * This is equivalent to the n x n dimensions of all tile PNGs. As such, this
 * property assumes that all assets/tiles/$(tile_type).png are the same square 
 * size.
 */
extern int TILE_SCALE;

/**
 * Maximum allowed value for target_radius field of Mouse.
 */
extern const int MAX_TARGET_RADIUS;

/**
 * Unique modes the mouse can be in when placing tiles in the sandbox with LMB.
 * 
 * PLACE: Place tile only on empty spaces.
 * DELETE: Place empty (air) tile on any other tile.
 * REPLACE: Place tile on any other tile.
 */
enum mouse_mode {PLACE, DELETE, REPLACE, NUM_MOUSE_MODES};


/**
 * Arrays of length NUM_TILE_TYPES to NULL-terminated strings indicating 
 * relative filepaths of textures used by tiles and panels.
 */
extern const char *TILE_TEXTURE_FILENAMES[];
extern const char *PANEL_TEXTURE_FILENAMES[];


/**
 * Type describing GUI Application data controlled by a mouse input device.
 */
struct Mouse
{
    // Mouse coordinates in app window in terms of absolute window size.
    int x;
    int y;

    // Radius of square target area in sandbox.
    // The target area determines where tiles are placed.
    int target_radius;

    // Whether LMB or LCTRL are currently held down or not.
    // LCTRL enables controlling size of target_radius.
    bool is_left_clicking;
    bool is_holding_lctrl;

    // Selected particle tile type to place down.
    enum tile_type selected_type;

    // Mode controlling whether mouse deletes or places tiles into sandbox.
    enum mouse_mode mode;
};


/**
 * Type describing all critical components owned by a GUI application.
 */
struct Application
{
    // App GUI rendering components.
    SDL_Renderer *renderer;
    SDL_Window *window;

    // Mouse data captured by GUI application.
    struct Mouse *mouse;

    // Sandbox owned and displayed by GUI application.
    struct Sandbox *sandbox;

    // Minimum size of app window in pixels, depends on sandbox size.
    // Actual window size may grow larger than these values.
    int min_window_width;
    int min_window_height;
};


/**
 * Initialize Sandbox GUI application and SDL with respect to the given sandbox
 * data.
 *
 * @param title NULL-terminated bytestring to name the application window.
 * @param sandbox Sandbox to be owned by the initialized GUI application.
 * @return Pointer to created GUI application which renders and owns the passed
 * sandbox object.
 * 
 * This function will call exit() if any part of the app initialization fails.
 */
struct Application *init_gui(const char *title, struct Sandbox *sandbox);


/**
 * Quit the sandbox application, freeing any memory it takes up, shutting down
 * any libraries initialized by init_gui(), and exiting the running process
 * gracefully.
 * 
 * @param app Owning GUI application to shutdown and free.
 */
void quit_gui(struct Application *app);


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
void blit_texture(struct Application *app, SDL_Texture *texture, SDL_Point window_coords);


/**
 * Draw a rectangle of the given dimensions and RGBA value to the screen of
 * the given GUI application at the rectangle's (x,y) window coordinates.
 * 
 * @param app Application to draw rectangle on.
 * @param rect Dimensions of rectangle to draw packed in an SDL_Rect.
 * @param color RGBA color of rectangle packed as an SDL_Color.
 * @param do_fill If true, rectangle is filled with color. If false, only
 * outline of rectangle with color is drawn.
 */
void blit_rectangle(struct Application *app, const SDL_Rect rect, SDL_Color color, bool do_fill);


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
 * @param app 
 * @param coords 
 */
void draw_tile(struct Application *app, struct SandboxPoint coords);


/**
 * Render sand-sim UI elements to the given app.
 *
 * This includes panels displaying which tile type is currently selected by the
 * app's mouse.
 *
 * @param app App to render UI elements to.
 *
 */
void draw_ui(struct Application *app);


/**
 * Poll SDL for any user-input (mouse input, keyboard input) and update the
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


/**
 * Switch the mouse's selected tile type to the given new_type.
 *
 * @param mouse Mouse to switch contained tile type of.
 * @param new_type Value from 0 to 15 representing the new tile type.
 */
void switch_selected_type(struct Mouse *mouse, enum tile_type new_type);


/**
 * Alter the sandbox by changing the tile located at the mouse's screen 
 * location, scaled down sandbox coordinates via TILE_SCALE, according to the
 * mouse mode. 
 * 
 * Typically, this is the logic which runs when LMB is pressed.
 * 
 * @param mouse Mouse whose coordiantes and selected tile type will be used
 * to mutate a tile in the given sandbox.
 * @param sandbox Sandbox to mutate and alter tile in according to mouse data.
 */
void alter_tile(struct Mouse *mouse, struct Sandbox *sandbox);


#endif // GUI_H
