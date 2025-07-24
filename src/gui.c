/*
 * Implementation of gui.h interface.
 *
 */

#include "gui.h"
#include "sandbox.h"

#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>


// ----- SDL API CALL WRAPPERS -----


/**
 * Wrapper for all SDL calls which can fail on returning a negative error code.
 * 
 * Not intended to be called on a raw pointer.
 * 
 * @param status_code Code returned from an SDL API call.
 * @param line_number The expanded value of __LINE__ when API call is made.
 */
static int SDL_CHECK_CODE(int status_code, int line_number)
{
    if (status_code < 0)
    {
        SDL_Log("\nSDL ERROR: %s:%d\nReason: %s\n",
                __FILE__,
                line_number,
                SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    return status_code;
}

/**
 * Wrapper for all SDL calls which can fail on returning a NULL pointer.
 * 
 * Not intended to be called on a raw pointer.
 * 
 * @param sdl_ptr A pointer to an SDL type as returned from an SDL API call.
 * @param line_number The expanded value of __LINE__ when API call is made.
 */
static void *SDL_CHECK_PTR(void *sdl_ptr, int line_number)
{
   if (sdl_ptr == NULL)
   {
       SDL_Log("\nSDL ERROR: %s:%d\nReason: %s\n",
               __FILE__,
               line_number,
               SDL_GetError());
       exit(EXIT_FAILURE);
   }

   return sdl_ptr;
}


// There are at most 16 unique tile IDs, and therefore 16 unique textures.
static const int NUM_UNIQUE_TILES = 16;

SDL_Texture **TILE_TEXTURES;
SDL_Texture **PANEL_TEXTURES;

// ----- PRIVATE FUNCTIONS -----


/**
 * Scale the given window (x, y) coordinates down to (row, col) sandbox
 * coordinates. 
 * 
 * x is scaled w.r.t sandbox width to get a col index in the range [0, width].
 * y is scaled w.r.t sandbox height to get a row index in the range [0, height].
 * 
 * The given window coordinates are expected to be placed in an array of size
 * at least 2 in order [x, y].
 * 
 * The sandbox coordinates are given as an output parameter in order [row, col]
 * 
 * @param window_coords Array of size at least 2, holding (x, y) window coords.
 * @param sandbox Sandbox whose dimensions will be used to scale down window
 * coodinates
 * @param sandbox_coords Out-parameter array of size at least 2 to place 
 * resulting sandbox coordinates in.
 */
static void _scale_screen_coords(int *window_coords, struct Sandbox *sandbox, int *sandbox_coords)
{
    int x = window_coords[0];
    int y = window_coords[1];

    // Downscale the window coordinates to floating sandbox coordinates.
    float raw_row = (float) y / PIXEL_SCALE;
    float raw_col = (float) x / PIXEL_SCALE;

    // Chop off decimal portion to obtain valid sandbox indices.
    int row = (int) raw_row;
    int col = (int) raw_col;

    // If window coordinates go outside sandbox, clamp targeted indices to edge
    // of sandbox.
    row = (row >= sandbox->height) ? sandbox->height - 1 : row;
    col = (col >= sandbox->width) ? sandbox->width - 1 : col;

    row = (row < 0) ? 0 : row;
    col = (col < 0) ? 0 : col;

    sandbox_coords[0] = row;
    sandbox_coords[1] = col;
}


/**
 * Scale the (x, y) location of the given mouse located in some window down to 
 * (row, col) sandbox coordinates.
 * 
 * The sandbox coordinates are given as an output parameter in order [row, col]
 * 
 * @param mouse Mouse whose window coordinates will 
 * @param sandbox Sandbox whose dimensions will be used to scale down mouse
 * coodinates
 * @param sandbox_coords Out-parameter array of size at least 2 to place 
 * resulting sandbox coordinates in.
 */
static void _scale_mouse_coords(struct Mouse *mouse, struct Sandbox *sandbox, int *sandbox_coords)
{
    int mouse_coords[] = {mouse->x, mouse->y};
    _scale_screen_coords(mouse_coords, sandbox, sandbox_coords);
}


/**
 * Update mouse button pressed-down data in the given app by extracting mouse 
 * data from the mouse button event.
 *
 * @param app App containing mouse click data to update.
 * @param event Mouse event containing mouse data to extract.
 */
static void _do_mouse_button_down(struct Application *app, SDL_MouseButtonEvent *event)
{
    unsigned char mouse_button = event->button;

    switch (mouse_button)
    {
        case SDL_BUTTON_LEFT:
            app->mouse->is_left_clicking = true;
            break;
        
        // Do nothing on unhandled mouse press.
        default:
            break;
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
    unsigned char mouse_button = event->button;

    switch (mouse_button)
    {
        case SDL_BUTTON_LEFT:
            app->mouse->is_left_clicking = false;
            break;
        
        // Loop through mouse placement modes on RMB press.
        case SDL_BUTTON_RIGHT:
            app->mouse->mode = (app->mouse->mode + 1) % NUM_MOUSE_MODES;
            break;
        
        // Do nothing on unhandled mouse press.
        default:
            break;
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
    struct Mouse *app_mouse = app->mouse;

    // Gather information about the key pressed.
    SDL_Keysym key_data = event->keysym;
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
            quit_gui(app);

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


/**
 * Free any memory the passed GUI application takes up, shutting down any
 * libraries loading by init_gui().
 *
 * @param app Owning GUI application to free.
 */
void _cleanup(struct Application *app)
{
    // Shut down SDL and free memory taken up by app.
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    sandbox_free(app->sandbox);
    free(app->mouse);
    free(app);

    // Remove textures before exiting.
    _destroy_textures();

    // Quit SDL and SDL_Image.
    IMG_Quit();
    SDL_Quit();
}



// ----- PUBLIC FUNCTIONS -----


struct Application *init_gui(const char *title, struct Sandbox *sandbox)
{
    struct Application *app = malloc(sizeof(*app));

    // Initialize window screen dimensions as a scale of the sandbox dimensions.
    app->sandbox = sandbox;
    app->min_window_width = sandbox->width * PIXEL_SCALE;
    app->min_window_height = sandbox->height * PIXEL_SCALE;

    // Let SDL pick the first suitable renderer.
    unsigned int renderer_flags = 0;

    // Window creation flags.
    unsigned int window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

    // Attempt to initialize SDL2 video subsystem.
    SDL_CHECK_CODE(SDL_Init(SDL_INIT_VIDEO), __LINE__);

    // Init SDL_Image for use with PNGs and JPGs
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    // Create app window once video is initialized.
    app->window = SDL_CHECK_PTR(SDL_CreateWindow(title, 
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            app->min_window_width,
            app->min_window_height,
            window_flags), __LINE__);
    SDL_SetWindowMinimumSize(app->window, app->min_window_width, app->min_window_height);

    // Use nearest interpolation to scale resolution for pixel-perfect tiles.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Create renderer using the first graphics acceleration device found.
    // Set a logical drawing area for automatic resolution scaling of rendered contents.
    // Logical area is big enough to render sandbox at full resolution.
    app->renderer = SDL_CHECK_PTR(SDL_CreateRenderer(app->window, -1, renderer_flags), __LINE__);
    SDL_CHECK_CODE(SDL_RenderSetLogicalSize(app->renderer, app->min_window_width, app->min_window_height), __LINE__);

    // Zero-out mouse to start selected tile as being sand.
    struct Mouse *new_mouse = calloc(1, sizeof(*new_mouse));
    new_mouse->selected_tile = SAND;
    app->mouse = new_mouse;

    // Allocate memory for all textures used by the 16 possible tile types.
    init_textures(app);

    return app;
}


void init_textures(struct Application *app)
{
    // Allocate memory for array to hold pointers to all textures.
    TILE_TEXTURES = malloc((size_t) NUM_UNIQUE_TILES * sizeof(*TILE_TEXTURES));
    PANEL_TEXTURES = malloc((size_t) NUM_UNIQUE_TILES * sizeof(*PANEL_TEXTURES));

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


void quit_gui(struct Application *app)
{
    _cleanup(app);
    exit(EXIT_SUCCESS);
}


SDL_Texture *load_texture(struct Application *app, char *filename)
{
    // Call SDL_image to load image.
    SDL_Texture *texture = SDL_CHECK_PTR(IMG_LoadTexture(app->renderer, filename), __LINE__);

    return texture;
}


void blit_texture(struct Application *app, SDL_Texture *texture, int x, int y)
{
    // Setup rectangle to draw texture.
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;

    // Fill in rectangle dimension data by querying the texture.
    SDL_CHECK_CODE(SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h), __LINE__);

    // Draw texture, passing in NULL to copy whole texture.
    SDL_CHECK_CODE(SDL_RenderCopy(app->renderer, texture, NULL, &dest), __LINE__);
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
    SDL_CHECK_CODE(SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255), __LINE__);

    // Clear the window with the current set color.
    SDL_CHECK_CODE(SDL_RenderClear(app->renderer), __LINE__);
}


void draw_sandbox(struct Application *app)
{
    for (int row = 0; row < app->sandbox->height; row++)
    {
        for (int col = 0; col < app->sandbox->width; col++)
        {
            unsigned char current_tile = app->sandbox->grid[row][col];

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
    SDL_Texture *panel_texture = get_panel_texture(app->mouse->selected_tile);
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
            // Quit app and exit process on closing window input event.
            case SDL_QUIT:
                quit_gui(app);
                break;

            // We obtain mouse coordinates in an event, as unlike SDL_GetMouseState(),
            // mouse coordinates captured this way are unaffected by logical renderer
            // scaling (these are mouse coordinates within absolute window size).
            case SDL_MOUSEMOTION:
                app->mouse->x = event.motion.x;
                app->mouse->y = event.motion.y;
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


void handle_input(struct Application *app)
{
    // Left clicking controls placing/deleting tiles on the owned sandbox.
    if (app->mouse->is_left_clicking)
    {
        switch (app->mouse->mode)
        {
            case PLACE:
                place_tile(app->mouse, app->sandbox);
                break;
            
            case DELETE:
                delete_tile(app->mouse, app->sandbox);
                break;
            
            case REPLACE:
                replace_tile(app->mouse, app->sandbox);
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
    
    mouse->selected_tile = tile_type;
}


void place_tile(struct Mouse *mouse, struct Sandbox *sandbox)
{
    int sandbox_coords[2];
    _scale_mouse_coords(mouse, sandbox, sandbox_coords);
    int row = sandbox_coords[0];
    int col = sandbox_coords[1];

    // Don't replace tiles, only place them ontop of air.
    if (get_tile_id(sandbox->grid[row][col]) != AIR)
    {
        return;
    }

    // Sync new tile to the sandbox lifetime to prevent update until next frame.
    sandbox->grid[row][col] = mouse->selected_tile;
    set_tile_updated(&(sandbox->grid[row][col]), sandbox->lifetime);
}


void delete_tile(struct Mouse *mouse, struct Sandbox *sandbox)
{
    // Scale down mouse window coordinates to sandbox coordinates.
    int sandbox_coords[2];
    _scale_mouse_coords(mouse, sandbox, sandbox_coords);
    int row = sandbox_coords[0];
    int col = sandbox_coords[1];

    sandbox->grid[row][col] = AIR;
}

void replace_tile(struct Mouse *mouse, struct Sandbox *sandbox)
{
    int sandbox_coords[2];
    _scale_mouse_coords(mouse, sandbox, sandbox_coords);
    int row = sandbox_coords[0];
    int col = sandbox_coords[1];

    // Sync new tile to the sandbox lifetime to prevent update until next frame.
    sandbox->grid[row][col] = mouse->selected_tile;
    set_tile_updated(&(sandbox->grid[row][col]), sandbox->lifetime);
}

