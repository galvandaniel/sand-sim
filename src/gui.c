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
 * The value of this constant must be consistent with the (n x n) dimensions of
 * assets/tiles/$(tile_type).png for the sandbox to display as intended.
 * Ex: If sand.png is 8x8, TILE_SCALE should be 8.
 *
 * If TILE_SCALE < n, tiles will display on top of eachother.
 * If TILE_SCALE > n, tiles will display with empty space between eachother.
 */
const int TILE_SCALE = 8;

const int MAX_TARGET_RADIUS = 5;

/**
 * The order of these filepaths must be consistent with the order of
 * enum tile_type, otherwise the wrong colors will display for tiles.
 */
const char *TILE_TEXTURE_FILENAMES[] = {
    "assets/tiles/air.png",
    "assets/tiles/sand.png",
    "assets/tiles/water.png",
    "assets/tiles/wood.png",
    "assets/tiles/steam.png",
    "assets/tiles/fire.png"
};
const char *PANEL_TEXTURE_FILENAMES[] = {
    "assets/panels/air_panel.png",
    "assets/panels/sand_panel.png",
    "assets/panels/water_panel.png",
    "assets/panels/wood_panel.png",
    "assets/panels/steam_panel.png",
    "assets/panels/fire_panel.png"
};


/**
 * Constant SDL pixel format allocated for lifetime of any GUI application.
 * This format is guaranteed to support an alpha channel no matter the
 * endianness of the running system.
 */
static SDL_PixelFormat *ALPHA_PIXEL_FORMAT = NULL;


SDL_Texture **TILE_TEXTURES;
SDL_Texture **PANEL_TEXTURES;
SDL_Texture **HIGHLIGHT_TEXTURES;


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


// ----- PRIVATE FUNCTIONS -----


/**
 * Scale the given window (x, y) coordinates down to (row, col) sandbox
 * coordinates. 
 * 
 * x is scaled w.r.t sandbox width to get a col index in the range [0, width].
 * y is scaled w.r.t sandbox height to get a row index in the range [0, height].
 * 
 * @param window_coords (x, y) coordinates in a window packed into an SDL point.
 * @param sandbox Sandbox whose dimensions will be used to scale down window
 * coodinates.
 * @return (x, y) scaled to (row, col) coordinates packed into a point.
 */
static struct SandboxPoint _scale_screen_coords(SDL_Point *window_coords, struct Sandbox *sandbox)
{
    int x = window_coords->x;
    int y = window_coords->y;

    // Downscale the window coordinates to floating sandbox coordinates.
    float raw_row = (float) y / (float) TILE_SCALE;
    float raw_col = (float) x / (float) TILE_SCALE;

    // Chop off decimal portion to obtain valid sandbox indices.
    int row = (int) raw_row;
    int col = (int) raw_col;

    // If window coordinates go outside sandbox, clamp targeted indices to edge
    // of sandbox.
    row = (row >= sandbox->height) ? sandbox->height - 1 : row;
    col = (col >= sandbox->width) ? sandbox->width - 1 : col;

    row = (row < 0) ? 0 : row;
    col = (col < 0) ? 0 : col;

    struct SandboxPoint sandbox_coords = {row, col};
    return sandbox_coords;
}


/**
 * Scale the (x, y) location of the given mouse located in some window down to 
 * (row, col) sandbox coordinates.
 * 
 * @param mouse Mouse whose window coordinates will be scaled down to sandbox.
 * @param sandbox Sandbox whose dimensions will be used to scale down mouse
 * coodinates.
 * @return Mouse coordinates scaled to (row, col) coordinates packed into point.
 */
static struct SandboxPoint _scale_mouse_coords(struct Mouse *mouse, struct Sandbox *sandbox)
{
    SDL_Point mouse_coords = {mouse->x, mouse->y};
    return _scale_screen_coords(&mouse_coords, sandbox);
}


/**
 * Scale the given (row, col) sandbox coordinates located in some sandbox up to 
 * (x, y) SDL window coordinates.
 * 
 * row is scaled by TILE_SCALE up to a y screen coordinate.
 * col is scaled by TILE_SCALE up to a x screen coordinate.
 * 
 * This function assumes the given coordinates are valid for whatever sandbox
 * they came from.
 * 
 * @param sandbox_coords Coordinates into some sandbox packed as a point.
 * @return x, y Screen coordinates packed into an SDL point.
 */
static SDL_Point _scale_sandbox_coords(struct SandboxPoint sandbox_coords)
{
    int x = sandbox_coords.col * TILE_SCALE;
    int y = sandbox_coords.row * TILE_SCALE;
    SDL_Point window_coords = {x, y};
    return window_coords;
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
 * Perform any application updates that need to occur as a result of scrolling 
 * or pressing the mouse wheel.
 * 
 * @param app App to mutate as a result of mousewheel motion.
 * @param event Mouse wheel event containing data on wheel motion.
 */
static void _do_mouse_wheel_motion(struct Application *app, SDL_MouseWheelEvent *event)
{
    // Amount of vertical scroll is platform dependent.
    // Example: Scroll is +/- 1 on Linux, is +/- INT_MAX on Windows.
    // To account for this, apply sign function on scroll value to limit to
    // +/- 1. (scroll is never 0 if SDL Mousewheel event is triggered)
    int vertical_scroll = event->y;
    int scroll_sign = (vertical_scroll > 0) ? 1 : -1;

    // Holding lctrl enables changing brush size.
    // Clamp target_radius to stay within inclusive range [0, MAX_TARGET_RADIUS].
    if (app->mouse->is_holding_lctrl)
    {
        app->mouse->target_radius += scroll_sign;
        app->mouse->target_radius = (app->mouse->target_radius < 0) ? 0 : app->mouse->target_radius;
        app->mouse->target_radius = (app->mouse->target_radius > MAX_TARGET_RADIUS) ? MAX_TARGET_RADIUS : app->mouse->target_radius;
        return;
    }

    // Treating amount scrolled as a displacement, change tile type on scroll. 
    // Prevent selecting AIR, whose selection should instead roll-over depending
    // on direction of scroll.
    int new_type = ((int) app->mouse->selected_type + scroll_sign) % NUM_TILE_TYPES;
    if (new_type == AIR)
    {
        new_type = (vertical_scroll > 0) ? SAND : NUM_TILE_TYPES - 1;
    }
    switch_selected_type(app->mouse, (enum tile_type) new_type);
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
            switch_selected_type(app_mouse, SAND);
            break;

        case SDLK_2:
            switch_selected_type(app_mouse, WATER);
            break;

        case SDLK_3:
            switch_selected_type(app_mouse, WOOD);
            break;

        case SDLK_4:
            switch_selected_type(app_mouse, STEAM);
            break;

        case SDLK_5:
            switch_selected_type(app_mouse, FIRE);
            break;

        case SDLK_LCTRL:
            app_mouse->is_holding_lctrl = true;
            break;        

        // In the case of pressing ESC, the app will quit.
        case SDLK_ESCAPE:
            quit_gui(app);

        // In an unhandled keypress, do nothing.
        default:
            break;
    }
}


/**
 * Perform any application updates that need to occur as a result of any
 * keyboard key release.
 *
 * @param app App to mutate as a result of key release.
 * @param event Keyboard event containing data on what key was released.
 */
static void _do_keyboard_release(struct Application *app, SDL_KeyboardEvent *event)
{
    struct Mouse *app_mouse = app->mouse;

    // Gather information about the key released.
    SDL_Keysym key_data = event->keysym;
    SDL_Keycode keycode = key_data.sym;

    switch (keycode)
    {
        case SDLK_LCTRL:
            app_mouse->is_holding_lctrl = false;
            break;

        // In an unhandled key release, do nothing.
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
 * Compute the sidelength of a square target area in terms of sandbox tiles given
 * a radius.
 * 
 * @param radius Radius of target area.
 * @return Sidelength of target area in terms of sandbox tiles.
 */
static int _compute_target_area_sidelength(int radius)
{
    return (radius * 2) + 1;
}


/**
 * Compute the size of a square target area in terms of sandbox tiles given a 
 * radius.
 * 
 * @param radius Radius of target area.
 * @return Size of target area in terms of sandbox tiles.
 */
static int _compute_target_area_size(int radius)
{
    // Careful for negative radius which, when cast to size_t for malloc() or
    // passed into a VLA, will cause malloc() to return NULL or cause UB.
    if (radius < 0)
    {
        SDL_Log("\nWARNING: %s:%d\nReason: Attempted to compute target area size of negative radius!\n", __FILE__, __LINE__);
        return 0;
    }

    // Zero radius is a special case of 1-tile size draw area.
    if (radius == 0)
    {
        return 1;
    }

    // Compute square area of draw area.
    int sidelength = _compute_target_area_sidelength(radius);
    return sidelength * sidelength;
}


/**
 * Compute a target area inside a given sandbox which surrounds and includes the 
 * given origin point in terms of sandbox (row, col) coordinates.
 * 
 * Any target area points which would go OOB the given sandbox are not included
 * in the output array of points.
 * 
 * The space for array of coordinates is allocated by the caller as the output
 * parameter target_area. 
 * This array must be of size _compute_target_area_size() + 1, though the contents 
 * of the array are not guaranteed to be this long.
 * 
 * The output array is terminated by the (row, col) coordinates (-1, -1) to 
 * indicate the end of array.
 * 
 * @param sandbox Sandbox bounding points within computed target area.
 * @param origin_coords Origin of target area in terms of sandbox coordinates.
 * @param radius Radius of the target area computed.
 * @param target_area Output to place computed target area sandbox coordonates.
 */
static void _get_sandbox_target_area(struct Sandbox *sandbox, 
                                     struct SandboxPoint origin_coords, 
                                     int radius,
                                     struct SandboxPoint *target_area)
{
    struct SandboxPoint terminator = {-1, -1};

    // Zero radius is a special case of 1-tile size draw area.
    if (radius == 0)
    {
        target_area[0] = origin_coords;
        target_area[1] = terminator;
        return;
    }

    // Compute non-OOB square of target area by starting from topleft of square
    // and going through the tiles in row-major order.
    // Maintain an index into the array to terminate the end.
    int sidelength = _compute_target_area_sidelength(radius);
    struct SandboxPoint topleft = {origin_coords.row - radius, origin_coords.col - radius};
    int flattened_index = 0;

    for (int row_offset = 0; row_offset < sidelength; row_offset++)
    {
        for (int col_offset = 0; col_offset < sidelength; col_offset++)
        {
            struct SandboxPoint next_point = {topleft.row + row_offset, topleft.col + col_offset};

            if (is_coord_oob(sandbox, next_point))
            {
                continue;
            }

            target_area[flattened_index] = next_point;
            flattened_index++;
        }
    }
    target_area[flattened_index] = terminator;
}


/**
 * Draw the target area highlight for the single tile located at the given 
 * sandbox coordinates. 
 * 
 * @param app GUI application to draw a single tile of target area highlight.
 * @param coords Sandbox coordinates of tile to draw highlight for.
 */
static void _draw_tile_highlight(struct Application *app, struct SandboxPoint coords)
{
    SDL_Texture *highlight_texture = get_highlight_texture(app->mouse->selected_type);
    SDL_Point highlight_coords = _scale_sandbox_coords(coords);

    // Do not show highlight ontop of non-empty tiles when placing.
    if (!is_tile_empty(app->sandbox->grid[coords.row][coords.col]) 
     && app->mouse->mode == PLACE)
    {
        return;
    }

    // Show a red outline ontop of tiles about to be deleted.
    // Query any highlight texture to get width/height data for red square.
    if (app->mouse->mode == DELETE)
    {
        SDL_Rect dest;
        dest.x = highlight_coords.x;
        dest.y = highlight_coords.y;

        SDL_CHECK_CODE(SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, 128), __LINE__);
        SDL_CHECK_CODE(SDL_QueryTexture(highlight_texture, NULL, NULL, &dest.w, &dest.h), __LINE__);
        SDL_CHECK_CODE(SDL_RenderDrawRect(app->renderer, &dest), __LINE__);

        return;
    }

    blit_texture(app, highlight_texture, highlight_coords);
}


/**
 * Draw the drawing-area highlight showing tiles around the mouse that are about
 * to be placed/replaced/deleted.
 * 
 * The pixels drawn by this function must be displayed by SDL_RenderPresent()
 * to show to screen.
 * 
 * @param app GUI Application to draw drawing-area highlight for.
 */
static void _draw_highlight(struct Application *app)
{
    // Snap mouse coordinate to nearest sandbox coordinates.
    struct SandboxPoint sandbox_coords = _scale_mouse_coords(app->mouse, app->sandbox);

    // Get target area and draw a single tile highlight over all coordinates.
    // Allocate 1 extra element for target area to account for terminator.
    int target_area_size = _compute_target_area_size(app->mouse->target_radius);
    struct SandboxPoint target_area[target_area_size + 1];
    _get_sandbox_target_area(app->sandbox, 
                             sandbox_coords, 
                             app->mouse->target_radius, 
                             target_area);
    int i = 0;
    while (target_area[i].row != -1)
    {
        _draw_tile_highlight(app, target_area[i]);
        i++;
    }
}



/**
 * Unload all tile textures from memory, destroying them and freeing the array
 * of tile_textures.
 */
static void _destroy_textures(void)
{
    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        SDL_DestroyTexture(TILE_TEXTURES[i]);
        SDL_DestroyTexture(PANEL_TEXTURES[i]);
        SDL_DestroyTexture(HIGHLIGHT_TEXTURES[i]);
    }

    free(TILE_TEXTURES);
    free(PANEL_TEXTURES);
    free(HIGHLIGHT_TEXTURES);
}


/**
 * Free any memory the passed GUI application takes up, shutting down any
 * libraries loading by init_gui().
 *
 * @param app Owning GUI application to free.
 */
static void _cleanup(struct Application *app)
{
    // Shut down SDL and free memory taken up by app.
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    sandbox_free(app->sandbox);
    free(app->mouse);
    free(app);

    // Remove textures and the universal alpha format before exiting.
    _destroy_textures();
    SDL_FreeFormat(ALPHA_PIXEL_FORMAT);

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
    app->min_window_width = sandbox->width * TILE_SCALE;
    app->min_window_height = sandbox->height * TILE_SCALE;

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

    // Zero-out mouse to start out with non-left clicking and mode PLACE.
    struct Mouse *new_mouse = calloc(1, sizeof(*new_mouse));
    new_mouse->selected_type = SAND;
    app->mouse = new_mouse;

    // Allocate memory for all textures used by all possible tile types, and
    // the universal alpha pixel format if not yet allocated.
    // Enable alpha blending for transparent textures on renderer.
    if (ALPHA_PIXEL_FORMAT == NULL)
    {
        ALPHA_PIXEL_FORMAT = SDL_CHECK_PTR(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32), __LINE__);
    }
    SDL_CHECK_CODE(SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND), __LINE__);
    init_textures(app);

    return app;
}


void init_textures(struct Application *app)
{
    // Allocate memory for array to hold pointers to all textures.
    TILE_TEXTURES = malloc((size_t) NUM_TILE_TYPES * sizeof(*TILE_TEXTURES));
    PANEL_TEXTURES = malloc((size_t) NUM_TILE_TYPES * sizeof(*PANEL_TEXTURES));
    HIGHLIGHT_TEXTURES = malloc((size_t) NUM_TILE_TYPES * sizeof(*HIGHLIGHT_TEXTURES));

    // Load all tile, panel, and highlight textures.
    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        TILE_TEXTURES[i] = load_texture(app, TILE_TEXTURE_FILENAMES[i], false);
    }

    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        PANEL_TEXTURES[i] = load_texture(app, PANEL_TEXTURE_FILENAMES[i], false);
    }

    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        // Enable alpha blending on highlight textures and set 50% transparent.
        // Highlight textures are based off tile textures.
        HIGHLIGHT_TEXTURES[i] = load_texture(app, TILE_TEXTURE_FILENAMES[i], true);
        SDL_CHECK_CODE(SDL_SetTextureAlphaMod(HIGHLIGHT_TEXTURES[i], 128), __LINE__);
    }
}


void quit_gui(struct Application *app)
{
    _cleanup(app);
    exit(EXIT_SUCCESS);
}


SDL_Texture *load_texture(struct Application *app, const char *filename, bool enable_alphablend)
{
    // For no blending support, can call SDL_image to load image.
    if (!enable_alphablend)
    {
        SDL_Texture *texture = SDL_CHECK_PTR(IMG_LoadTexture(app->renderer, filename), __LINE__);
        return texture;
    }

    // SDL_Image makes no guarantee on the image format of loaded textures.
    //
    // To guarantee alpha channel presence, temporarily load image as surface,
    // then conver surface to a portable alpha channel format. Finally, convert
    // back to texture, enable blending, and free the temporary surfaces.
    SDL_Surface *raw_surface = SDL_CHECK_PTR(IMG_Load(filename), __LINE__);
    SDL_Surface *alpha_surface = SDL_CHECK_PTR(SDL_ConvertSurface(raw_surface, ALPHA_PIXEL_FORMAT, 0), __LINE__);
    SDL_Texture *texture = SDL_CHECK_PTR(SDL_CreateTextureFromSurface(app->renderer, alpha_surface), __LINE__);
    SDL_CHECK_CODE(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND), __LINE__);
    SDL_FreeSurface(raw_surface);
    SDL_FreeSurface(alpha_surface);

    return texture;
}


void blit_texture(struct Application *app, SDL_Texture *texture, SDL_Point window_coords)
{
    // Setup rectangle to draw texture.
    SDL_Rect dest;
    dest.x = window_coords.x;
    dest.y = window_coords.y;

    // Fill in rectangle dimension data by querying the texture.
    SDL_CHECK_CODE(SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h), __LINE__);

    // Draw texture, passing in NULL to copy whole texture.
    SDL_CHECK_CODE(SDL_RenderCopy(app->renderer, texture, NULL, &dest), __LINE__);
}


SDL_Texture *get_tile_texture(unsigned char tile)
{
    enum tile_type current_type = get_tile_type(tile);
    return TILE_TEXTURES[current_type];
}


SDL_Texture *get_panel_texture(enum tile_type type)
{
    return PANEL_TEXTURES[type];
}


SDL_Texture *get_highlight_texture(enum tile_type type)
{
    return HIGHLIGHT_TEXTURES[type];
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
            struct SandboxPoint sandbox_coords = {row, col};
            unsigned char current_tile = app->sandbox->grid[row][col];

            // Don't draw empty tiles.
            if (is_tile_empty(current_tile))
            {
                continue;
            }

            // Compute the screen coordinates that a tile should be blitted at.
            SDL_Point window_coords = _scale_sandbox_coords(sandbox_coords);

            // Grab the associated tile texture and blit it to screen.
            SDL_Texture *tile_texture = get_tile_texture(current_tile);
            blit_texture(app, tile_texture, window_coords);
        }
    }
}


void draw_ui(struct Application *app)
{
    _draw_highlight(app);

    // Draw panel texture and blit to topleft of screen.
    SDL_Texture *panel_texture = get_panel_texture(app->mouse->selected_type);
    SDL_Point topleft = {0, 0};
    blit_texture(app, panel_texture, topleft);
}


void get_input(struct Application *app)
{
    // Take in an input event and react.
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
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

            // React to scrolling of mouse wheel.
            case SDL_MOUSEWHEEL:
                _do_mouse_wheel_motion(app, &event.wheel);
                break;

            // When a key gets pressed/released, perform any keyboard updates.
            case SDL_KEYDOWN:
                _do_keyboard_press(app, &event.key);
                break;
            case SDL_KEYUP:
                _do_keyboard_release(app, &event.key);
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
        alter_tile(app->mouse, app->sandbox);
    }
}


void alter_tile(struct Mouse *mouse, struct Sandbox *sandbox)
{
    // Snap mouse coordinate to nearest sandbox coordinates.
    struct SandboxPoint sandbox_coords = _scale_mouse_coords(mouse, sandbox);

    // Get target area and perform mouse mode operation for all tiles in target
    // area.
    int target_area_size = _compute_target_area_size(mouse->target_radius);
    struct SandboxPoint target_area[target_area_size + 1];
    _get_sandbox_target_area(sandbox, 
                             sandbox_coords, 
                             mouse->target_radius,
                             target_area);
    int i = 0;
    while (target_area[i].row != -1)
    {
        switch (mouse->mode)
        {
            case PLACE:
                place_tile(sandbox, target_area[i], mouse->selected_type);
                break;
            
            case DELETE:
                delete_tile(sandbox, target_area[i]);
                break;
            
            case REPLACE:
                replace_tile(sandbox, target_area[i], mouse->selected_type);
                break;

            default:
                break;
        }
        i++;
    }
}



void switch_selected_type(struct Mouse *mouse, enum tile_type new_type)
{
    // For invalid tile types, do nothing.
    if (new_type > NUM_TILE_TYPES - 1)
    {
        return;
    }
    
    mouse->selected_type = new_type;
}

