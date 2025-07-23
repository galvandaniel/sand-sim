#ifndef GUI_H
#define GUI_H

/**
 * A collection of functions for displaying a sand simulation using SDL2 and
 * the logic presented in sandbox.h
 */

#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

// Upscaling for individual pixels when drawing to screen.
#define PIXEL_SCALE 8


// Array of pointers to all textures used by tiles.
extern SDL_Texture **TILE_TEXTURES;

// Array of pointers to all textures used by panels. 
// Panels display the element currently selected.
extern SDL_Texture **PANEL_TEXTURES;

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
    unsigned char selected_tile;
};


/**
 * Type describing all critical components owned by a GUI application.
 */
struct Application
{
    // App GUI components.
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
 * @param app App to load image on.
 * @param filename Filepath of image to load from src folder as root.
 *
 * @return Image loaded as SDL_Texture
 */
SDL_Texture *load_texture(struct Application *app, char *filename);


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
 * Obtain the panel texture for the given tile, to be used for showing the
 * tile as currently selected tile type.
 *
 * @param tile Tile to fetch panel texture for.
 *
 * @return Panel texture corresponding to given tile, blittable to screen.
 */
SDL_Texture *get_panel_texture(unsigned char tile);


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
 * represents one pixel onscreen, scaled in size according to PIXEL_SCALE.
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
 * Poll SDL for any user-input (mouse input, keyboard input) and react
 * accordingly within the sandbox application.
 *
 * @param app Application to react on due to input.
 */
void get_input(struct Application *app);


/**
 * Switch the mouse's selected tile type to the given tile_type.
 *
 * @param mouse Mouse to switch contained tile type of.
 * @param tile_type Value from 0 to 15 representing the new tile type.
 */
void switch_selected_tile(struct Mouse *mouse, unsigned char tile_type);


/**
 * Place a tile of the mouse's currently selected type at the mouse's relative 
 * location in the given sandbox.
 *
 * Mouse screen coordinates are scaled down by PIXEL_SCALE to place a tile
 * within sandbox.
 *
 * @param mouse Pointer to mouse to get placement location and tile type.
 * @param sandbox Sandbox to mutate and place tile in.
 */
void place_tile(struct Mouse *mouse, struct Sandbox *sandbox);


#endif // GUI_H
