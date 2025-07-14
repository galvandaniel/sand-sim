/*
 * Implementation of gui.h interface.
 *
 */

#include "gui.h"
#include "sandbox.h"

#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>


/**
 * Wrapper for all SDL calls which can fail on returning a negative error code.
 * 
 * Not intended to be called on a raw pointer.
 */
static int SDL_CHECK_CODE(int status_code)
{
    if (status_code < 0)
    {
        SDL_Log("\nSDL ERROR: %s\nReason: %s\n",
                __FILE__,
                SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    return status_code;
}

/**
 * Wrapper for all SDL calls which can fail on returning a NULL pointer.
 * 
 * Not intended to be called on a raw pointer.
 */
static void *SDL_CHECK_PTR(void *sdl_ptr)
{
   if (sdl_ptr == NULL)
   {
       SDL_Log("\nSDL ERROR: %s\nReason: %s\n",
               __FILE__,
               SDL_GetError());
       exit(EXIT_FAILURE);
   }

   return sdl_ptr;
}


// There are at most 16 unique tile IDs, and therefore 16 unique textures.
static const int NUM_UNIQUE_TILES = 16;

// Dimensions are chosen by user at the command line when the program begins
// however here, we default to a medium-size sandbox.
int SANDBOX_WIDTH = SANDBOX_MEDIUM_WIDTH;
int SANDBOX_HEIGHT = SANDBOX_MEDIUM_HEIGHT;

int WINDOW_WIDTH;
int WINDOW_HEIGHT;


SDL_Texture **TILE_TEXTURES;

SDL_Texture **PANEL_TEXTURES;

// ----- PRIVATE FUNCTIONS -----

/**
 * Update mouse button pressed-down data in the given app by extracting mouse 
 * data from the mouse button event.
 *
 * @param app App containing mouse click data to update.
 * @param event Mouse event containing mouse data to extract.
 */
static void _do_mouse_button_down(struct Application *app, SDL_MouseButtonEvent *event)
{
    if (event -> button == SDL_BUTTON_LEFT)
    {
        app -> mouse -> is_left_clicking = true;
    }
}


/**
 * Update mouse button lift-up data in the given app by extracting mouse data
 * from the mouse button event.
 *
 * @param app App containing mouse click data to update.
 * @param event Mouse event containing mouse data to extract.
 */
static void _do_mouse_button_up(struct Application *app, SDL_MouseButtonEvent *event)
{
    // In the event the mouse button that was just lifted up was the left button,
    // the user is no longer holding down left.
    if (event -> button == SDL_BUTTON_LEFT)
    {
        app -> mouse -> is_left_clicking = false;
    }
}


/**
 * Perform any application updates that need to occur as a result of any
 * keyboard keypress.
 *
 * @param app App to mutate as a result of keypress.
 * @param event Keyboard event containing data on what key was pressed.
 */
static void _do_keyboard_press(struct Application *app, SDL_KeyboardEvent *event)
{
    struct Mouse *app_mouse = app -> mouse;

    // Gather information about the key pressed.
    SDL_Keysym key_data = event -> keysym;
    SDL_Keycode keycode = key_data.sym;

    switch (keycode)
    {
        // In the event of keys 0 - 9, switch mouse tile to appropriate type.
        case SDLK_1:
            switch_selected_tile(app_mouse, SAND);
            break;

        case SDLK_2:
            switch_selected_tile(app_mouse, WATER);
            break;

        case SDLK_3:
            switch_selected_tile(app_mouse, WOOD);
            break;

        case SDLK_4:
            switch_selected_tile(app_mouse, STEAM);
            break;

        // In the case of pressing q, the app will quit.
        case SDLK_q:
            cleanup(app);
            exit(EXIT_SUCCESS);

        // In an unhandled keypress, do nothing.
        default:
            break;

    }
}


/**
 * Perform any application updates that occur due to any changes in the window
 * state.
 * 
 * @param app App to mutate due to change in window state.
 * @param event Window event containing data on what state change occurred.
 */
static void _do_window_change(struct Application *app, SDL_WindowEvent *event)
{
    SDL_WindowEventID event_id = event->event;

    switch (event_id)
    {
        // SDL handles window resizing automatically, and with the logical 
        // renderer size set, will handle resizing content automatically too.
        //
        // Cover window with black to prevent resizing causing ugly stretching 
        // of content at border. 
        case SDL_WINDOWEVENT_RESIZED:

            set_black_background(app);
            SDL_UpdateWindowSurface(app->window);
            break;
        
        // Do nothing on unhandled window event.
        default:
            break;
    }
}


/**
 * Unload all tile textures from memory, destroying them and freeing the array
 * of tile_textures.
 */
static void _destroy_textures(void)
{
    for (int i = 0; i < NUM_UNIQUE_TILES; i++)
    {
        SDL_DestroyTexture(TILE_TEXTURES[i]);
        SDL_DestroyTexture(PANEL_TEXTURES[i]);
    }

    free(TILE_TEXTURES);
    free(PANEL_TEXTURES);
}


// ----- PUBLIC FUNCTIONS -----


struct Application *init_gui(char *title)
{
    // Allocate memory for the app.
    struct Application *app = malloc(sizeof(*app));

    // Let SDL pick the first suitable renderer.
    unsigned int renderer_flags = 0;

    // Window creation flags.
    unsigned int window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

    // Attempt to initialize SDL2 video subsystem.
    SDL_CHECK_CODE(SDL_Init(SDL_INIT_VIDEO));

    // Init SDL_Image for use with PNGs and JPGs
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    // Initialize window dimensions as a scale of the sandbox dimensions.
    WINDOW_WIDTH = SANDBOX_WIDTH * PIXEL_SCALE;
    WINDOW_HEIGHT = SANDBOX_HEIGHT * PIXEL_SCALE;

    // Create app window once video is initialized.
    app -> window = SDL_CHECK_PTR(SDL_CreateWindow(title, 
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            window_flags));
    SDL_SetWindowMinimumSize(app -> window, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Use nearest interpolation to scale resolution for pixel-perfect tiles.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Create renderer using the first graphics acceleration device found.
    // Set a logical drawing area for automatic resolution scaling of rendered contents.
    // Logical area is big enough to render sandbox at full resolution.
    app -> renderer = SDL_CHECK_PTR(SDL_CreateRenderer(app -> window, -1, renderer_flags));
    SDL_CHECK_CODE(SDL_RenderSetLogicalSize(app -> renderer, WINDOW_WIDTH, WINDOW_HEIGHT));

    // Zero-out mouse to start selected tile as being sand.
    struct Mouse *new_mouse = calloc(1, sizeof(*new_mouse));
    new_mouse -> selected_tile = SAND;
    app -> mouse = new_mouse;

    // Allocate memory for all textures used by the 16 possible tile types.
    init_textures(app);

    return app;
}


void init_textures(struct Application *app)
{
    // Allocate memory for array to hold pointers to all textures.
    TILE_TEXTURES = malloc((unsigned long) NUM_UNIQUE_TILES * sizeof(*TILE_TEXTURES));
    PANEL_TEXTURES = malloc((unsigned long) NUM_UNIQUE_TILES * sizeof(*PANEL_TEXTURES));

    // Load all tile textures.
    TILE_TEXTURES[AIR] = load_texture(app, "assets/tiles/air.png");
    TILE_TEXTURES[SAND] = load_texture(app, "assets/tiles/sand.png");
    TILE_TEXTURES[WATER] = load_texture(app, "assets/tiles/water.png");
    TILE_TEXTURES[WOOD] = load_texture(app, "assets/tiles/wood.png");
    TILE_TEXTURES[STEAM] = load_texture(app, "assets/tiles/steam.png");
    TILE_TEXTURES[FIRE] = load_texture(app, "assets/tiles/fire.png");

    // Load all panel textures.
    PANEL_TEXTURES[AIR] = load_texture(app, "assets/panels/air_panel.png");
    PANEL_TEXTURES[SAND] = load_texture(app, "assets/panels/sand_panel.png");
    PANEL_TEXTURES[WATER] = load_texture(app, "assets/panels/water_panel.png");
    PANEL_TEXTURES[WOOD] = load_texture(app, "assets/panels/wood_panel.png");
    PANEL_TEXTURES[STEAM] = load_texture(app, "assets/panels/steam_panel.png");
    PANEL_TEXTURES[FIRE] = load_texture(app, "assets/panels/fire_panel.png");

    // Placeholder textures while other tiles are being implemented.
    for (int i  = 6; i < NUM_UNIQUE_TILES; i++)
    {
        TILE_TEXTURES[i] = load_texture(app, "assets/tiles/air.png");
        PANEL_TEXTURES[i] = load_texture(app, "assets/panels/air_panel.png");
    }
}


void cleanup(struct Application *app)
{
    // Shut down SDL and free memory taken up by app.
    SDL_DestroyWindow(app -> window);
    SDL_DestroyRenderer(app -> renderer);
    free(app -> mouse);
    free(app);

    // Remove textures before exiting.
    _destroy_textures();

    // Quit SDL and SDL_Image.
    IMG_Quit();
    SDL_Quit();
}


SDL_Texture *load_texture(struct Application *app, char *filename)
{
    // Call SDL_image to load image.
    SDL_Texture *texture = SDL_CHECK_PTR(IMG_LoadTexture(app -> renderer, filename));

    return texture;
}


void blit_texture(struct Application *app, SDL_Texture *texture, int x, int y)
{
    // Setup rectangle to draw texture.
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;

    // Fill in rectangle dimension data by querying the texture.
    SDL_CHECK_CODE(SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h));

    // Draw texture, passing in NULL to copy whole texture.
    SDL_CHECK_CODE(SDL_RenderCopy(app -> renderer, texture, NULL, &dest));
}


SDL_Texture *get_tile_texture(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);
    return TILE_TEXTURES[tile_type];
}


SDL_Texture *get_panel_texture(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);
    return PANEL_TEXTURES[tile_type];
}


void set_black_background(struct Application *app)
{
    // Set color to black.
    SDL_CHECK_CODE(SDL_SetRenderDrawColor(app -> renderer, 0, 0, 0, 255));

    // Clear the window with the current set color.
    SDL_CHECK_CODE(SDL_RenderClear(app -> renderer));
}


void draw_sandbox(struct Application *app, unsigned char **sandbox, int height, int width)
{
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            unsigned char current_tile = sandbox[row][col];

            // Don't draw air.
            if (get_tile_id(current_tile) == AIR)
            {
                continue;
            }

            // Compute the coordinates that a tile should be blitted at by
            // scaling up their position as dictated by the scale factor.
            int tile_x = col * PIXEL_SCALE;
            int tile_y = row * PIXEL_SCALE;

            // Grab the associated tile texture and blit it to screen.
            SDL_Texture *tile_texture = get_tile_texture(current_tile);
            blit_texture(app, tile_texture, tile_x, tile_y);
        }
    }
}


void draw_ui(struct Application *app)
{
    // Grab the panel texture for the currently selected tile, and blit to the
    // topleft of the screen.
    SDL_Texture *panel_texture = get_panel_texture(app -> mouse -> selected_tile);
    blit_texture(app, panel_texture, 0, 0);
}


void get_input(struct Application *app)
{
    // Take in an input event and react.
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // Free memory taken up by app, then shutdown.
            case SDL_QUIT:
                cleanup(app);
                exit(EXIT_SUCCESS);

            // We obtain mouse coordinates in an event, as unlike SDL_GetMouseState(),
            // mouse coordinates captured this way are unaffected by logical renderer
            // scaling (these are mouse coordinates relative to window size).
            case SDL_MOUSEMOTION:
                app -> mouse -> x = event.motion.x;
                app -> mouse -> y = event.motion.y;
                break;

            // Record player holding down mouse button by keeping track of
            // when it is pressed down and up.
            case SDL_MOUSEBUTTONDOWN:
                _do_mouse_button_down(app, &event.button);
                break;

            case SDL_MOUSEBUTTONUP:
                _do_mouse_button_up(app, &event.button);
                break;

            // When a key gets pressed, perform any keyboard-related updates.
            case SDL_KEYDOWN:
                _do_keyboard_press(app, &event.key);
                break;

            
            case SDL_WINDOWEVENT:
                _do_window_change(app, &event.window);
                break;
    

            default:
                break;
        }
    }
}


void switch_selected_tile(struct Mouse *mouse, unsigned char tile_type)
{
    // For invalid tile types, do nothing.
    if (tile_type > NUM_UNIQUE_TILES - 1)
    {
        return;
    }
    
    mouse -> selected_tile = tile_type;
}


void place_tile(struct Mouse *mouse, unsigned char **sandbox, int height, int width)
{
    // Downscale the mouse coordinates to floating sandbox coordinates.
    float row_coordinate = (float) mouse -> y / PIXEL_SCALE;
    float col_coordinate = (float) mouse -> x / PIXEL_SCALE;

    // Chop off decimal portion to obtain valid sandbox indices.
    int row_index = (int) row_coordinate;
    int col_index = (int) col_coordinate;

    // If mouse coordinates go outside window, clamp targeted indices to edge
    // of sandbox.
    row_index = (row_index >= height) ? height - 1 : row_index;
    col_index = (col_index >= width) ? width - 1 : col_index;

    row_index = (row_index < 0) ? 0 : row_index;
    col_index = (col_index < 0) ? 0 : col_index;

    // Don't replace tiles, only place them ontop of air.
    if (get_tile_id(sandbox[row_index][col_index]) != AIR)
    {
        return;
    }


    // Place tile, replacing air.
    //
    // A tile type has value (0000 XXXX) where XXXX is the tile type.
    // This produces a new, non-updated, non-static tile of type XXXX.
    sandbox[row_index][col_index] = mouse -> selected_tile;
}



