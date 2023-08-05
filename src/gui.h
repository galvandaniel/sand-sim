#ifndef GUI_H
#define GUI_H

/*
 * A collection of functions for displaying a sand simulation using SDL2 and
 * the logic presented in sandbox.h
 *
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "sandbox.h"

// Upscaling for individual pixels when drawing to screen.
#define PIXEL_SCALE 8

// Width and height of sandbox simulation in tiles.
extern unsigned int SANDBOX_WIDTH;
extern unsigned int SANDBOX_HEIGHT;

// Width and height of window in pixels.
// Window size depends on sandbox size, and sandbox size depends on user input.
extern unsigned int WINDOW_WIDTH;
extern unsigned int WINDOW_HEIGHT;

// Array of pointers to all textures used by tiles.
extern SDL_Texture **TILE_TEXTURES;

// Struct for holding mouse location data, button data, and the user's
// currently selected tile.
struct Mouse
{
    int x;
    int y;
    bool is_left_clicking;
    unsigned char selected_tile;
};


// Struct for holding references to the GUI application's most integral pieces:
// The window, renderer, and mouse.
struct Application
{
    SDL_Renderer *renderer;
    SDL_Window *window;
    struct Mouse *mouse;
};


/*
 * Initializes sandbox application and SDL, allocating memory for a window 
 * and renderer together in a single application struct.
 *
 * @param title - NULL-terminated bytestring to name the application window.
 *
 * @return - Pointer to created application containing pointers to the resulting
 * window and renderer. 
 * This function returns NULL if any part of the app initialization fails
 */
struct Application *init_gui(char *title);


/*
 * Loads into memory all textures used by tiles in the sandbox.
 *
 * Use get_tile_texture to retrieve the texture a tile should use.
 *
 * @param app - Application holding renderer to load textures onto.
 */
void init_tile_textures(struct Application *app);


/*
 * Cleanup the given sandbox application, freeing any memory it takes up, and 
 * shutdown SDL.
 *
 * @param app - Application to shutdown and free.
 */
void cleanup(struct Application *app);


/*
 * Given the filename for a location to a JPG or PNG, load the image as an
 * SDL_Texture on the given application.
 *
 * @param app - App to load image on.
 * @param filename - Filepath of image to load from src folder as root.
 *
 * @return - Image loaded as SDL_Texture
 */
SDL_Texture *load_texture(struct Application *app, char *filename);


/*
 * Draw the given SDL texture on the given app at the given x and y screen
 * coordinates
 *
 * @param app - Application to draw texture on.
 * @param texture - Loaded texture to draw.
 * @param x, y - Coordinates to draw texture at.
 */
void blit_texture(struct Application *app, SDL_Texture *texture, int x, int y);


/*
 * Obtain the texture a tile must render to based on its type.
 *
 * @param tile - Tile to fetch texture for.
 *
 * @return - Texture of tile that can be blit to screen.
 */
SDL_Texture *get_tile_texture(unsigned char tile);


/*
 * Setup a black background on the given application.
 *
 * This background can be used as a base for further blitting.
 *
 * @param app - App whose background to set.
 */
void set_black_background(struct Application *app);


/*
 * Draw a sandbox to the app, where each tile represents one pixel onscreen,
 * scaled in size according to PIXEL_SCALE.
 *
 * The given sandbox must be of a smaller dimension than the screen resolution,
 * and must not be NULL.
 *
 * @param app - App to draw sandbox to.
 * @param sandbox - Sandbox of tiles to draw to screen.
 * @param height, width - Dimensions of sandbox.
 */
void draw_sandbox(struct Application *app, unsigned char **sandbox, unsigned int height, unsigned int width);


/*
 * Poll SDL for any user-input (mouse input, keyboard input) and react
 * accordingly within the sandbox application.
 *
 * @param app - Application to react on due to input.
 */
void get_input(struct Application *app);


/*
 * Place a tile of the mouse's currently selected type at the mouse's location
 * in the given sandbox with dimension (width x height).
 *
 * Mouse screen coordinates are scaled down by PIXEL_SCALE to place a tile
 * within sandbox.
 *
 * @param mouse - Pointer to mouse to get placement location and tile type.
 * @param sandbox - Sandbox to mutate and place tile in.
 * @param height, width - Dimensions of the given sandbox in tiles.
 *
 */
void place_tile(struct Mouse *mouse,
        unsigned char **sandbox,
        unsigned int height,
        unsigned int width);


#endif
