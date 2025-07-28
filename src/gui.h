#ifndef GUI_H
#define GUI_H

/**
 * A collection of functions for displaying a sand simulation using SDL2 and
 * the logic presented in sandbox.h
 */

#include "sandbox.h"

#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

/**
 * Scale-factor which determines how many screen pixels it takes to draw 1 side 
 * of 1 particle tile in the sandbox.
 * Ex: TILE_SCALE = 8 indicates 1 tile particle takes up 64 screen pixels.
 */
extern const int TILE_SCALE;

/**
 * Unique modes the mouse can be in when placing tiles in the sandbox with LMB.
 * 
 * PLACE: Place tile only on empty spaces.
 * DELETE: Place empty (air) tile on any other tile.
 * REPLACE: Place tile on any other tile.
 */
enum mouse_mode {PLACE, DELETE, REPLACE, NUM_MOUSE_MODES};


// Arrays of length NUM_TILE_TYPES to NULL-terminates strings indicating 
// relative filepaths of textures used by tiles and panels.
extern const char *TILE_TEXTURE_FILENAMES[];
extern const char *PANEL_TEXTURE_FILENAMES[];


// Array of pointers to all textures used by tiles.
extern SDL_Texture **TILE_TEXTURES;

// Array of pointers to all textures used by panels. 
// Panels display the element currently selected.
extern SDL_Texture **PANEL_TEXTURES;


// Array of pointers to textures used to draw transparent mouse highlight.
extern SDL_Texture **HIGHLIGHT_TEXTURES;

/**
 * Type descrbing GUI Application data controlled by a mouse input device.
 */
struct Mouse
{
    // Mouse coordinates in app window in terms of absolute window size.
    int x;
    int y;

    // Whether LMB is currently held down or not.
    bool is_left_clicking;

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
 *
 * @return Pointer to created GUI application which renders and owns the passed
 * sandbox object.
 * This function will call exit() if any part of the app initialization fails.
 */
struct Application *init_gui(const char *title, struct Sandbox *sandbox);


/**
 * Loads into memory all textures used by tiles and panels in the sandbox.
 *
 * Use get_tile_texture to retrieve the texture a tile should use.
 * Use get_panel_texture to retrieve the texture a panel should use.
 *
 * @param app Owning GUI application holding renderer to load textures onto.
 */
void init_textures(struct Application *app);


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
 * The loaded texture may optionally have alpha blending enabled for drawing
 * with opacity. This will force the resulting texture to have an SDL format
 * which supports an alpha channel.
 *
 * @param app App to load image on.
 * @param filename Filepath of image to load from, relative to program location.
 * @param enable_alphablend If true, texture is loaded to support alpha blending.
 * If false, texture may or may not support alpha blending, depending on the
 * image contents of the file loaded.
 *
 * @return Image loaded as SDL_Texture
 */
SDL_Texture *load_texture(struct Application *app, const char *filename, bool enable_alphablend);


/**
 * Draw the given SDL texture on the given app at the given x and y screen
 * coordinates
 *
 * @param app Application to draw texture on.
 * @param texture Loaded texture to draw.
 * @param x, y Coordinates to draw texture at.
 */
void blit_texture(struct Application *app, SDL_Texture *texture, int x, int y);


/**
 * Obtain the texture a tile must render to based on its type.
 *
 * @param tile Tile to fetch texture for.
 *
 * @return Texture of tile that can be blit to screen.
 */
SDL_Texture *get_tile_texture(unsigned char tile);


/**
 * Obtain the panel texture for the given tile type, to be used for showing the
 * tile type as currently selected tile type.
 *
 * @param tile Tile type to fetch panel texture for.
 *
 * @return Panel texture corresponding to given tile type, blittable to screen.
 */
SDL_Texture *get_panel_texture(enum tile_type type);


/**
 * Obtain the texture used for the given tile type to show a single tile of 
 * drawing-area highlight.
 * 
 * @param type Tile type to get highlight texture for.
 * 
 * @return Texture showing a single tile of highlight, blittable to screen.
 */
SDL_Texture *get_highlight_texture(enum tile_type type);


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
 * Place a tile of the given mouse's currently selected type at its screen
 * location, scaled down the dimensions of the passed sandbox according to
 * TILE_SCALE.
 *
 *
 * @param mouse Mouse whose coordinates and selected tile type will be used.
 * @param sandbox Sandbox to mutate and place tile in according to mouse data.
 */
void place_tile(struct Mouse *mouse, struct Sandbox *sandbox);


/**
 * Remove the tile located at the given mouse's screen location, scaled down to
 * the dimensions of the passed sandbox according to TILE_SCALE.
 * 
 * @param mouse Mouse whose coordinates will be used to delete a tile.
 * @param sandbox Sandbox to mutate and delete tile in.
 */
void delete_tile(struct Mouse *mouse, struct Sandbox *sandbox);


/**
 * Replace the tile located at the given mouse's screen location, scaled down to
 * the dimensions of the passed sandbox according to TILE_SCALE, with the
 * mouse's currently selected type.
 * 
 * @param mouse Mouse whose coordinates and selected tile type will be used.
 * @param sandbox Sandbox to mutate and replace tile in.
 */
void replace_tile(struct Mouse *mouse, struct Sandbox *sandbox);


#endif // GUI_H
