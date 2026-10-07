/*
 * Implementation of gui.h interface.
 */

#include "gui.h"
#include "sandbox.h"
#include "utils.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

// Wrappers around all SDL API calls to check for and report on failure.
#define SDL_CHECK_BOOL(result) (sdl_check_bool(result, __FILE__, __LINE__))
#define SDL_CHECK_PTR(sdl_ptr) (sdl_check_ptr(sdl_ptr, __FILE__, __LINE__))
#define SDL_CHECK_PTR_TO_CONST(sdl_ptr) (sdl_check_ptr_to_const(sdl_ptr, __FILE__, __LINE__))

/**
 * The value of this constant is consistent with the (n x n) dimensions of
 * assets/tiles/$(tile_type).png for the sandbox to display as intended.
 * Ex: If sand.png is 8x8, TILE_SCALE is 8.
 */
float TILE_SCALE = 0;

/**
 * Maximum allowed size of mouse target area 'brush' size.
 */
constexpr int MAX_TARGET_RADIUS = 5;

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
    "assets/tiles/fire.png",
    "assets/tiles/fuel.png",
};
const char *PANEL_TEXTURE_FILENAMES[] = {
    "assets/panels/air_panel.png",
    "assets/panels/sand_panel.png",
    "assets/panels/water_panel.png",
    "assets/panels/wood_panel.png",
    "assets/panels/steam_panel.png",
    "assets/panels/fire_panel.png",
    "assets/panels/fuel_panel.png",
};
const char *CURSOR_TEXTURE_FILENAMES[] = 
{
    "assets/cursors/place.png",
    "assets/cursors/delete.png",
    "assets/cursors/replace.png",
};


/**
 * Constant SDL pixel format used for lifetime of any GUI application.
 * This format is guaranteed to support an alpha channel no matter the
 * endianness of the running system.
 * 
 * (Currently unused, but maybe helpful in future?)
 */
static const SDL_PixelFormatDetails *ALPHA_PIXEL_FORMAT = nullptr;


/**
 * Array to primary colors of all tiles, indexed by enum tile_type.
 */
static SDL_Color TILE_COLORS[NUM_TILE_TYPES] = {};

/**
 * Various color constants.
 */
static constexpr SDL_Color NO_COLOR = {.r = 0, .g = 0, .b = 0, .a = 0};
static constexpr SDL_Color RED = {.r = 255, .g = 0, .b = 0, .a = 255};
static constexpr SDL_Color WHITE = {.r = 255, .g = 255, .b = 255, .a = 255};
static constexpr SDL_Color BLACK = {.r = 0, .g = 0, .b = 0, .a = 255};

/**
 * Constants which determine what tile color variations looks like. 
 *
 * Each color variation has its color modulated by an empirically chosen value
 * for each sand tile color variant code.
 * Ex: For a color mod factor of 10, Color code 0 of a sand tile is modulated 
 * by 0*10=0,Color code 3 is modulated by 3*10=30.
 */
static constexpr unsigned char COLOR_MOD_FACTOR = 10;


/**
 * Array of pointers to all textures used by tiles and panels, indexed by
 * enum tile_type.
 */
static SDL_Texture *TILE_TEXTURES[NUM_TILE_TYPES] = {};
static SDL_Texture *PANEL_TEXTURES[NUM_TILE_TYPES] = {};


// ----- SDL API CALL WRAPPERS -----


/**
 * Wrapper for all SDL calls which can fail on returning false.
 * 
 * Not intended to be called on any other value.
 * 
 * @param result Bool result from an SDL API call.
 * @param file The expanded value of __FILE__ when API call is made.
 * @param line The expanded value of __LINE__ when API call is made.
 */
static bool sdl_check_bool(bool result, const char *file, int line)
{
    if (!result)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "\nSDL FAILURE: %s:%d\nReason: %s\n",
                     file,
                     line,
                     SDL_GetError());
        exit(EXIT_FAILURE);
    }
    return result;
}

/**
 * Wrapper for all SDL calls which can fail on returning a NULL pointer.
 * 
 * Not intended to be called on a raw pointer.
 * 
 * @param sdl_ptr A pointer to an SDL type as returned from an SDL API call.
 * @param file The expanded value of __FILE__ when API call is made.
 * @param line The expanded value of __LINE__ when API call is made.
 */
static void *sdl_check_ptr(void *sdl_ptr, const char *file, int line)
{
   if (sdl_ptr == nullptr)
   {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "\nSDL FAILURE: %s:%d\nReason: %s\n",
                    file,
                    line,
                    SDL_GetError());
       exit(EXIT_FAILURE);
   }
   return sdl_ptr;
}


/**
 * Wrapper for all SDL calls which can fail on returning a NULL pointer.
 * 
 * Not intended to be called on a raw pointer.
 * 
 * @param sdl_ptr A pointer to an SDL type as returned from an SDL API call.
 * @param file The expanded value of __FILE__ when API call is made.
 * @param line The expanded value of __LINE__ when API call is made.
 */
static const void *sdl_check_ptr_to_const(const void *sdl_ptr, const char *file, int line)
{
   if (sdl_ptr == nullptr)
   {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "\nSDL FAILURE: %s:%d\nReason: %s\n",
                    file,
                    line,
                    SDL_GetError());
       exit(EXIT_FAILURE);
   }
   return sdl_ptr;
}


// ----- PRIVATE FUNCTIONS -----


/**
 * Get the RGB value of the surface pixel located at the given coordinates.
 * 
 * @param surface Surface to read pixel data from.
 * @param surface_coords (x,y) coordinates within surface to fetch pixel from.
 * @return Pixel RGB data packed into an SDL_Color.
 */
static SDL_Color get_pixel(SDL_Surface *surface, SDL_Point surface_coords)
{
    const SDL_PixelFormatDetails *format_details = SDL_CHECK_PTR_TO_CONST(SDL_GetPixelFormatDetails(surface->format));
    int bpp = format_details->bytes_per_pixel;

    // Lock surface for directly reading off pixel data, if necessary.
    if (SDL_MUSTLOCK(surface))
    {
        SDL_CHECK_BOOL(SDL_LockSurface(surface));
    }

    // Credit of implemetantion goes to:
    // https://stackoverflow.com/questions/53033971/how-to-get-the-color-of-a-specific-pixel-from-sdl-surface

    // Advance pixel pointer to beginning of requested pixel.
    ptrdiff_t offset = (surface_coords.y * surface->pitch) + (surface_coords.x * bpp);
    Uint8 *pixel = ((Uint8 *) surface->pixels + offset);


    Uint32 pixel_data;
    constexpr int BYTE_SHIFT = 8;
    constexpr int TWO_BYTE_SHIFT = 16;
    switch (bpp)
    {
        // If 1 byte per pixel, read the one byte.
        case 1:
            pixel_data = *pixel;
            break;
        
        // If 2 bytes per pixel, cast pointer to read 2 bytes at once.
        case 2:
            pixel_data = *(Uint16 *) pixel;
            break;
        
        // If 3 bytes per pixel, manually copy.
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                pixel_data = (Uint32) (pixel[0] << TWO_BYTE_SHIFT | pixel[1] << BYTE_SHIFT | pixel[2]);
            }
            else
            {
                pixel_data = (Uint32) (pixel[0] | pixel[1] << BYTE_SHIFT | pixel[2] << TWO_BYTE_SHIFT);
            }
            break;
        
        case 4:
            pixel_data = *(Uint32 *) pixel;
            break;
        
        default:
            pixel_data = 0;
            break;
    }

    // Fill color with RGB components using the surface pixel format.
    SDL_Color rgb;
    rgb.a = SDL_ALPHA_OPAQUE;
    SDL_GetRGB(pixel_data, format_details, SDL_GetSurfacePalette(surface) , &rgb.r, &rgb.g, &rgb.b);

    // Unlock surface if it was previously locked above.
    if (SDL_MUSTLOCK(surface))
    {
        SDL_UnlockSurface(surface);
    }

    return rgb;
}



/**
 * Scale the given window (x, y) coordinates down to (row, col) sandbox
 * coordinates. 
 * 
 * x is scaled w.r.t sandbox width to get a col index in the range [0, width].
 * y is scaled w.r.t sandbox height to get a row index in the range [0, height].
 * 
 * @param window_coords (x, y) coordinates in a window packed into an SDL fpoint.
 * @param sandbox Sandbox whose dimensions will be used to scale down window
 * coodinates.
 * @return (x, y) scaled to (row, col) coordinates packed into a point.
 */
static struct SandboxPoint scale_screen_coords(SDL_FPoint window_coords, struct Sandbox *sandbox)
{
    // Downscale the window coordinates to sandbox coordinates.
    float raw_row = window_coords.y / TILE_SCALE;
    float raw_col = window_coords.x / TILE_SCALE;

    // Chop off decimal portion to obtain valid sandbox indices.
    int row = (int) raw_row;
    int col = (int) raw_col;

    // Prevent indices from going OOB.
    row = clamp(row, 0, sandbox->height - 1);
    col = clamp(col, 0, sandbox->width - 1);

    struct SandboxPoint sandbox_coords = {.row = row, .col = col};
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
static struct SandboxPoint scale_mouse_coords(struct Mouse *mouse, struct Sandbox *sandbox)
{
    SDL_FPoint mouse_coords = {.x = mouse->x, .y = mouse->y};
    return scale_screen_coords(mouse_coords, sandbox);
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
 * @return Screen coordinates packed into an SDL point.
 */
static SDL_FPoint scale_sandbox_coords(struct SandboxPoint sandbox_coords)
{
    float window_x = (float) sandbox_coords.col * TILE_SCALE;
    float window_y = (float) sandbox_coords.row * TILE_SCALE;
    SDL_FPoint window_coords = {.x = window_x, .y = window_y};
    return window_coords;
}


/**
 * Update mouse button pressed-down data in the given app by extracting mouse 
 * data from the mouse button event.
 *
 * @param app App containing mouse click data to update.
 * @param event Mouse event containing mouse data to extract.
 */
static void do_mouse_button_down(struct Application *app, SDL_MouseButtonEvent *event)
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
static void do_mouse_button_up(struct Application *app, SDL_MouseButtonEvent *event)
{
    unsigned char mouse_button = event->button;

    switch (mouse_button)
    {
        case SDL_BUTTON_LEFT:
            app->mouse->is_left_clicking = false;
            break;
        
        // Loop through mouse placement modes on RMB press.
        case SDL_BUTTON_RIGHT:
            update_mode(app->mouse, (app->mouse->mode + 1) % NUM_MOUSE_MODES);
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
static void do_mouse_wheel_motion(struct Application *app, SDL_MouseWheelEvent *event)
{
    float vertical_scroll = event->y;

    // Holding lctrl while scrolling changes target size.
    int current_radius = app->mouse->target_radius;
    if (app->mouse->is_holding_lctrl)
    {
        app->mouse->target_radius = (vertical_scroll > 0) ? current_radius + 1 : current_radius - 1;
        app->mouse->target_radius = clamp(app->mouse->target_radius, 0, MAX_TARGET_RADIUS);
        return;
    }

    // Switch type when scrolling mouse.
    // Rollover type depending on scroll direction when result would exceed valid type bounds. 
    enum tile_type current_type = app->mouse->selected_type;
    enum tile_type new_type = SAND;
    if (vertical_scroll > 0)
    {
        new_type = current_type >= NUM_TILE_TYPES - 1 ? SAND : current_type + 1;
    }
    else
    {
        new_type = current_type <= SAND ? NUM_TILE_TYPES - 1 : current_type - 1;
    }

    switch_selected_type(app->mouse, new_type);
}


/**
 * Perform any application updates that need to occur as a result of any
 * keyboard keypress.
 *
 * @param app App to mutate as a result of keypress.
 * @param event Keyboard event containing data on what key was pressed.
 */
static void do_keyboard_press(struct Application *app, SDL_KeyboardEvent *event)
{
    struct Mouse *app_mouse = app->mouse;

    // Gather information about the key pressed.
    SDL_Keycode keycode = event->key;

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
        
        case SDLK_6:
            switch_selected_type(app_mouse, FUEL);
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
static void do_keyboard_release(struct Application *app, SDL_KeyboardEvent *event)
{
    struct Mouse *app_mouse = app->mouse;

    // Gather information about the key released.
    SDL_Keycode keycode = event->key;

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
 * size.
 * 
 * @param app App to mutate due to change in window state.
 */
static void do_window_resize(struct Application *app)
{
    // SDL handles window resizing automatically, and with the logical 
    // renderer size set, will handle resizing content automatically too.
    //
    // Cover window with black to prevent resizing causing ugly stretching 
    // of content at border. 
    set_black_background(app);
    SDL_UpdateWindowSurface(app->window);
}


/**
 * Compute the sidelength of a square target area in terms of sandbox tiles given
 * a radius.
 * 
 * @param radius Radius of target area.
 * @return Sidelength of target area in terms of sandbox tiles.
 */
static int compute_target_area_sidelength(int radius)
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
static int compute_target_area_size(int radius)
{
    // This should never happen. If it does, program is likely in invalid state.
    assert(radius >= 0 && "ERROR: Attempted to compute target area size of negative radius!\n");

    // Zero radius is a special case of 1-tile size draw area.
    if (radius == 0)
    {
        return 1;
    }

    // Compute square area of draw area.
    int sidelength = compute_target_area_sidelength(radius);
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
 * This array must be of size AT LEAST `compute_target_area(radius) + 1`, though 
 * the filled contents of the array are not guaranteed to be this long.
 * 
 * The output array is terminated by the (row, col) coordinates (-1, -1) to 
 * indicate the end of array.
 * 
 * @param sandbox Sandbox bounding points within computed target area.
 * @param origin_coords Origin of target area in terms of sandbox coordinates.
 * @param radius Radius of the target area computed.
 * @param target_area Output to place computed target area sandbox coordonates.
 */
static void get_sandbox_target_area(struct Sandbox *sandbox, 
                                    struct SandboxPoint origin_coords, 
                                    int radius,
                                    struct SandboxPoint *target_area)
{
    // This should never happen. If it does, program is definitely in invalid state.
    assert(radius >= 0 && "ERROR: Attempted to fill target area of negative radius!\n");

    struct SandboxPoint terminator = {.row = -1, .col = -1};

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
    int sidelength = compute_target_area_sidelength(radius);
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
static void draw_tile_highlight(struct Application *app, struct SandboxPoint coords)
{
    SDL_FPoint highlight_coords = scale_sandbox_coords(coords);

    // Do not show highlight ontop of non-empty tiles when placing.
    if (!is_tile_empty(get_tile(app->sandbox, coords)) 
     && app->mouse->mode == PLACE)
    {
        return;
    }

    // Draw square of highlight as big as a tile.
    SDL_FRect highlight_rect;
    highlight_rect.x = highlight_coords.x;
    highlight_rect.y = highlight_coords.y;
    highlight_rect.w = TILE_SCALE;
    highlight_rect.h = TILE_SCALE;

    // Show a red outline ontop of tiles about to be deleted, otherwise show
    // color of selected tile. Have highlight be half opaque.
    bool is_delete_mode = app->mouse->mode == DELETE;
    SDL_Color selected_color = TILE_COLORS[app->mouse->selected_type];
    SDL_Color highlight_color = is_delete_mode ? RED : selected_color;
    highlight_color.a = SDL_ALPHA_OPAQUE / 2;

    blit_rectangle(app, highlight_rect, highlight_color, !is_delete_mode);
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
static void draw_highlight(struct Application *app)
{
    // Snap mouse coordinate to nearest sandbox coordinates.
    struct SandboxPoint sandbox_coords = scale_mouse_coords(app->mouse, app->sandbox);

    // Get target area and draw a single tile highlight over all coordinates.
    int target_area_size = compute_target_area_size(MAX_TARGET_RADIUS);
    struct SandboxPoint target_area[target_area_size + 1];
    get_sandbox_target_area(app->sandbox, 
                             sandbox_coords, 
                             app->mouse->target_radius, 
                             target_area);
    
    for (int i = 0; target_area[i].row != -1; i++)
    {
        draw_tile_highlight(app, target_area[i]);
    }
}

/**
 * Loads into memory all textures used by tiles in the sandbox.
 * 
 * This function is idempotent, initializing textures several times does 
 * nothing.
 *
 * @param app Owning GUI application holding renderer to load textures onto.
 */
static void init_textures(struct Application *app)
{
    if (*TILE_TEXTURES != nullptr || *PANEL_TEXTURES != nullptr)
    {
        return;
    }

    // Load all tile, panel, and highlight textures.
    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        TILE_TEXTURES[i] = load_texture(app, TILE_TEXTURE_FILENAMES[i]);
        PANEL_TEXTURES[i] = load_texture(app, PANEL_TEXTURE_FILENAMES[i]);

        // Use nearest interpolation to scale resolution for pixel-perfect tiles.
        SDL_CHECK_BOOL(SDL_SetTextureScaleMode(TILE_TEXTURES[i], SDL_SCALEMODE_PIXELART));
        SDL_CHECK_BOOL(SDL_SetTextureScaleMode(PANEL_TEXTURES[i], SDL_SCALEMODE_PIXELART));
    }
}


/**
 * Initialize GUI application data relevant to blitting tile particles: colors,
 * pixel formats, and tile window dimensions.
 *
 * This function is idempotent, initializing several times does nothing.
 */
static void init_tile_colors(void)
{
    // If colors have been loaded already, do not reload.
    if (TILE_COLORS[1].a != NO_COLOR.a || ALPHA_PIXEL_FORMAT != nullptr)
    {
        return;
    }

    // Allocate the universal alpha pixel format and tile window dimensions.
    // Use size of first tile texture (AIR) as reprentative of all tiles.
    ALPHA_PIXEL_FORMAT = SDL_CHECK_PTR_TO_CONST(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32));
    SDL_Surface *reference_surface = SDL_CHECK_PTR(IMG_Load(TILE_TEXTURE_FILENAMES[0]));
    TILE_SCALE = (float) reference_surface->w;
    SDL_DestroySurface(reference_surface);

    // Initialize all colors used by tiles. 
    // Take pixel RGB at (0, 0) as representative of color of whole tile.
    SDL_Point topleft = {.x = 0, .y = 0};

    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        SDL_Surface *tile_surface = SDL_CHECK_PTR(IMG_Load(TILE_TEXTURE_FILENAMES[i]));
        TILE_COLORS[i] = get_pixel(tile_surface, topleft);
        SDL_DestroySurface(tile_surface);
    }
}


/**
 * Unload all tile textures from memory, destroying them and freeing the array
 * of tile_textures.
 */
static void destroy_textures(void)
{
    // Strictly speaking, it's not an error to destroy NULL, but not good either.
    if (*TILE_TEXTURES == nullptr || *PANEL_TEXTURES == nullptr)
    {
        SDL_Log("\nWARNING: %s:%d\nReason: Attempted to destroy textures without textures being initialized!\n", __FILE__, __LINE__);
    }

    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        SDL_DestroyTexture(TILE_TEXTURES[i]);
        SDL_DestroyTexture(PANEL_TEXTURES[i]);
    }
}


/**
 * Free any memory the passed GUI application takes up, shutting down any
 * libraries loading by init_gui().
 *
 * @param app Owning GUI application to free.
 */
static void cleanup_memory(struct Application *app)
{
    // Free memory taken up by app.
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    sandbox_free(app->sandbox);
    destroy_mouse(app->mouse);
    free(app);

    // Remove textures before exiting.
    destroy_textures();

    SDL_Quit();
}


// ----- PUBLIC FUNCTIONS -----


struct Application *init_gui(const char *title, struct Sandbox *sandbox)
{
    init_tile_colors();
    struct Application *app = SAFE_MALLOC(sizeof(*app));

    // Initialize window screen dimensions as a scale of the sandbox dimensions.
    app->sandbox = sandbox;
    app->min_window_width = sandbox->width * (int) TILE_SCALE;
    app->min_window_height = sandbox->height * (int) TILE_SCALE;

    unsigned int window_flags = SDL_WINDOW_RESIZABLE;

    // Init all SDL subsystems and library extensions.
    SDL_CHECK_BOOL(SDL_Init(SDL_INIT_VIDEO));

    app->window = SDL_CHECK_PTR(SDL_CreateWindow(title, 
                                                 app->min_window_width,
                                                 app->min_window_height,
                                                 window_flags));
    SDL_CHECK_BOOL(SDL_SetWindowMinimumSize(app->window, app->min_window_width, app->min_window_height));

    // Create renderer using the first graphics acceleration device found.
    // Set a logical drawing area for automatic resolution scaling of rendered contents.
    // Logical area is big enough to render sandbox at full resolution.
    app->renderer = SDL_CHECK_PTR(SDL_CreateRenderer(app->window, nullptr));
    SDL_CHECK_BOOL(SDL_SetRenderLogicalPresentation(app->renderer, app->min_window_width, app->min_window_height, SDL_LOGICAL_PRESENTATION_LETTERBOX));

    app->mouse = create_mouse();

    // Enable alpha blending for transparent textures on renderer and allocate
    // all textures.
    SDL_CHECK_BOOL(SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND));
    init_textures(app);
    return app;
}


void quit_gui(struct Application *app)
{
    cleanup_memory(app);
    exit(EXIT_SUCCESS);
}


struct Mouse *create_mouse(void)
{
    // Start new mouse as non-active and target radius size 0.
    struct Mouse *new_mouse = SAFE_CALLOC(1, sizeof(*new_mouse));
    new_mouse->selected_type = SAND;

    // Initialize cursors used by mouse.
    for (int i = 0; i < NUM_MOUSE_MODES; i++)
    {
        SDL_Surface *cursor_surface = SDL_CHECK_PTR(IMG_Load(CURSOR_TEXTURE_FILENAMES[i]));
        new_mouse->cursors[i] = SDL_CHECK_PTR(SDL_CreateColorCursor(cursor_surface, 0, 31));
        SDL_DestroySurface(cursor_surface);
    }

    update_mode(new_mouse, PLACE);
    return new_mouse;
}


void destroy_mouse(struct Mouse *mouse)
{
    for (int i = 0; i < NUM_MOUSE_MODES; i++)
    {
        SDL_DestroyCursor(mouse->cursors[i]);
    }
    free(mouse);
}


void update_mode(struct Mouse *mouse, enum mouse_mode mode)
{
    mouse->mode = mode;
    SDL_SetCursor(mouse->cursors[mouse->mode]);
}


SDL_Texture *load_texture(struct Application *app, const char *filename)
{
    // Call to SDL_image to load image.
    SDL_Texture *texture = SDL_CHECK_PTR(IMG_LoadTexture(app->renderer, filename));
    return texture;
}


// Currently unused, but maybe useful in future?
SDL_Texture *load_texture_alpha(struct Application *app, const char *filename, unsigned char alpha)
{
    // SDL_Image makes no guarantee on the image format of loaded textures.
    //
    // To guarantee alpha channel presence, temporarily load image as surface,
    // then convert surface to a portable alpha channel format. Finally, convert
    // to texture, enable + set alpha blending, and free the surfaces.
    SDL_Surface *raw_surface = SDL_CHECK_PTR(IMG_Load(filename));
    SDL_Surface *alpha_surface = SDL_CHECK_PTR(SDL_ConvertSurface(raw_surface, ALPHA_PIXEL_FORMAT->format));
    SDL_Texture *texture = SDL_CHECK_PTR(SDL_CreateTextureFromSurface(app->renderer, alpha_surface));
    SDL_CHECK_BOOL(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND));
    SDL_CHECK_BOOL(SDL_SetTextureAlphaMod(texture, alpha));

    SDL_DestroySurface(raw_surface);
    SDL_DestroySurface(alpha_surface);
    return texture;   
}


void blit_texture(struct Application *app, SDL_Texture *texture, SDL_FPoint window_coords)
{
    // Setup rectangle to draw texture.
    SDL_FRect dest;
    dest.x = window_coords.x;
    dest.y = window_coords.y;

    // Fill in rectangle dimension data by querying the texture.
    SDL_CHECK_BOOL(SDL_GetTextureSize(texture, &dest.w, &dest.h));

    // Draw texture, passing in NULL to copy whole texture.
    SDL_CHECK_BOOL(SDL_RenderTexture(app->renderer, texture, nullptr, &dest));
}


void blit_rectangle(struct Application *app, SDL_FRect rect, SDL_Color color, bool do_fill)
{
    // Draw a fill rectangle or rectangle outline, depending on parameter.
    SDL_CHECK_BOOL(SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a));
    bool (*rect_blitter)(SDL_Renderer *, const SDL_FRect *);
    rect_blitter = do_fill ? SDL_RenderFillRect : SDL_RenderRect;
    SDL_CHECK_BOOL(rect_blitter(app->renderer, &rect));
}


void set_black_background(struct Application *app)
{
    SDL_CHECK_BOOL(SDL_SetRenderDrawColor(app->renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a));
    SDL_CHECK_BOOL(SDL_RenderClear(app->renderer));
}


void draw_sandbox(struct Application *app)
{
    for (int row = 0; row < app->sandbox->height; row++)
    {
        for (int col = 0; col < app->sandbox->width; col++)
        {
            struct SandboxPoint sandbox_coords = {row, col};
            draw_tile(app, sandbox_coords);
        }
    }
}


void draw_tile(struct Application *app, struct SandboxPoint coords)
{
    Tile tile = get_tile(app->sandbox, coords);

    if (is_tile_empty(tile))
    {
        return;
    }

    // Compute the screen coordinates that a tile should be blitted at.
    SDL_FPoint window_coords = scale_sandbox_coords(coords);

    // Grab the associated tile texture, apply tile color variation, then blit.
    SDL_Texture *tile_texture = TILE_TEXTURES[get_tile_type(tile)];

    uint8_t color_mod = COLOR_MOD_FACTOR * get_tile_color(tile);
    SDL_Color variant = {
        .r = WHITE.r - color_mod, 
        .g = WHITE.g - color_mod, 
        .b = WHITE.b - color_mod,
        .a = WHITE.a,
    };
    SDL_CHECK_BOOL(SDL_SetTextureColorMod(tile_texture, variant.r, variant.g, variant.b));
    blit_texture(app, tile_texture, window_coords);
}


void draw_ui(struct Application *app)
{
    draw_highlight(app);

    // Draw panel texture and blit to topleft of screen.
    SDL_Texture *panel_texture = PANEL_TEXTURES[app->mouse->selected_type];
    SDL_FPoint topleft = {.x = 0, .y = 0};
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
            case SDL_EVENT_QUIT:
                quit_gui(app);
                break;

            // We obtain mouse coordinates in an event, as unlike SDL_GetMouseState(),
            // mouse coordinates captured this way can be auto-scaled to match
            // logical render size, transformed from window space.
            case SDL_EVENT_MOUSE_MOTION:
                SDL_CHECK_BOOL(SDL_ConvertEventToRenderCoordinates(app->renderer, &event));
                app->mouse->x = event.motion.x;
                app->mouse->y = event.motion.y;
                break;

            // Record player holding down mouse button by keeping track of
            // when it is pressed down and up.
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                do_mouse_button_down(app, &event.button);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                do_mouse_button_up(app, &event.button);
                break;

            // React to scrolling of mouse wheel.
            case SDL_EVENT_MOUSE_WHEEL:
                do_mouse_wheel_motion(app, &event.wheel);
                break;

            // When a key gets pressed/released, perform any keyboard updates.
            case SDL_EVENT_KEY_DOWN:
                do_keyboard_press(app, &event.key);
                break;
            case SDL_EVENT_KEY_UP:
                do_keyboard_release(app, &event.key);
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                do_window_resize(app);
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
    struct SandboxPoint sandbox_coords = scale_mouse_coords(mouse, sandbox);

    // Get target area and perform mouse mode operation for all tiles in target
    // area.
    int target_area_size = compute_target_area_size(MAX_TARGET_RADIUS);
    struct SandboxPoint target_area[target_area_size + 1];
    get_sandbox_target_area(sandbox, 
                            sandbox_coords, 
                            mouse->target_radius,
                            target_area);
    for (int i = 0; target_area[i].row != -1; ++i)
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
    }
}



void switch_selected_type(struct Mouse *mouse, enum tile_type new_type)
{
    // For invalid tile types, do nothing.
    if (new_type >= NUM_TILE_TYPES)
    {
        return;
    }
    
    mouse->selected_type = new_type;
}

