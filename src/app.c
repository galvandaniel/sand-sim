/**
 * Implementation of gui.h interface.
 */

#include "app.h"
#include "cursor.h"
#include "brush.h"
#include "sdl_utils.h"
#include "sandbox.h"
#include "tile.h"
#include "utils.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

const char *TILE_TEXTURE_FILENAMES[] = {
    [AIR] = "assets/tiles/air.png",
    [SAND] = "assets/tiles/sand.png",
    [WATER] = "assets/tiles/water.png",
    [WOOD] = "assets/tiles/wood.png",
    [STEAM] = "assets/tiles/steam.png",
    [FIRE] = "assets/tiles/fire.png",
    [FUEL] = "assets/tiles/fuel.png",
};
const char *PANEL_TEXTURE_FILENAMES[] = {
    [AIR] = "assets/panels/air_panel.png",
    [SAND] = "assets/panels/sand_panel.png",
    [WATER] = "assets/panels/water_panel.png",
    [WOOD] = "assets/panels/wood_panel.png",
    [STEAM] = "assets/panels/steam_panel.png",
    [FIRE] = "assets/panels/fire_panel.png",
    [FUEL] = "assets/panels/fuel_panel.png",
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


// ----- PRIVATE FUNCTIONS -----


/**
 * Update app state in response to mouse button press events.
 *
 * @param app App with state to update.
 * @param event Mouse event containing mouse data to extract.
 */
static void do_mouse_button_down(struct Application *app, SDL_MouseButtonEvent *event)
{
    unsigned char mouse_button = event->button;

    switch (mouse_button)
    {
        case SDL_BUTTON_LEFT:
            app->cursor->is_holding_interact = true;
            break;
        
        // Do nothing on unhandled mouse press.
        default:
            break;
    }
}


/**
 * Update app state in response to mouse button lift-up event.
 *
 * @param app App with state to update.
 * @param event Mouse event containing mouse data to extract.
 */
static void do_mouse_button_up(struct Application *app, SDL_MouseButtonEvent *event)
{
    unsigned char mouse_button = event->button;

    switch (mouse_button)
    {
        case SDL_BUTTON_LEFT:
            app->cursor->is_holding_interact = false;
            break;
        
        // Loop through brush placement modes on RMB press.
        case SDL_BUTTON_RIGHT:
            brush_increment_mode(app->brush);
            SDL_CHECK(SDL_SetCursor(app->cursor->icons[app->brush->mode]));
            break;

        // Do nothing on unhandled mouse press.
        default:
            break;
    }
}


/**
 * Update app state in response to pressing or scrolling mouse wheel.
 * 
 * @param app App with state to update as a result of mousewheel motion.
 * @param event Mouse wheel event containing data on wheel motion.
 */
static void do_mouse_wheel_motion(struct Application *app, SDL_MouseWheelEvent *event)
{
    float vertical_scroll = event->y;

    // Holding lctrl while scrolling changes brush target size.
    int current_radius = app->brush->target_radius;
    if (app->cursor->is_holding_lctrl)
    {
        app->brush->target_radius = (vertical_scroll > 0) ? current_radius + 1 : current_radius - 1;
        app->brush->target_radius = clamp(app->brush->target_radius, 0, MAX_TARGET_RADIUS);
        return;
    }

    // Switch type when scrolling mouse.
    // Rollover type depending on scroll direction when result would exceed valid type bounds. 
    enum TileType current_type = app->brush->selected_type;
    enum TileType new_type = SAND;
    if (vertical_scroll > 0)
    {
        new_type = current_type >= NUM_TILE_TYPES - 1 ? SAND : current_type + 1;
    }
    else
    {
        new_type = current_type <= SAND ? NUM_TILE_TYPES - 1 : current_type - 1;
    }

    brush_switch_type(app->brush, new_type);
}


/**
 * Update app state in response to keyboard key press.
 *
 * @param app App with state to update as a result of keypress.
 * @param event Keyboard event containing data on what key was pressed.
 */
static void do_keyboard_press(struct Application *app, SDL_KeyboardEvent *event)
{
    // Gather information about the key pressed.
    SDL_Keycode keycode = event->key;

    switch (keycode)
    {
        // In the event of keys 0 - 9, switch mouse tile to appropriate type.
        case SDLK_1:
            brush_switch_type(app->brush, SAND);
            break;

        case SDLK_2:
            brush_switch_type(app->brush, WATER);
            break;

        case SDLK_3:
            brush_switch_type(app->brush, WOOD);
            break;

        case SDLK_4:
            brush_switch_type(app->brush, STEAM);
            break;

        case SDLK_5:
            brush_switch_type(app->brush, FIRE);
            break;
        
        case SDLK_6:
            brush_switch_type(app->brush, FUEL);
            break;

        case SDLK_LCTRL:
            app->cursor->is_holding_lctrl = true;
            break;        

        // In the case of pressing ESC, the app will quit.
        case SDLK_ESCAPE:
            quit_app(app);

        // In an unhandled keypress, do nothing.
        default:
            break;
    }
}


/**
 * Update app state in response to keyboard key release.
 *
 * @param app App with state to update as a result of key release.
 * @param event Keyboard event containing data on what key was released.
 */
static void do_keyboard_release(struct Application *app, SDL_KeyboardEvent *event)
{
    // Gather information about the key released.
    SDL_Keycode keycode = event->key;

    switch (keycode)
    {
        case SDLK_LCTRL:
            app->cursor->is_holding_lctrl = false;
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
     && app->brush->mode == PLACE)
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
    bool is_delete_mode = app->brush->mode == DELETE;
    SDL_Color selected_color = TILE_COLORS[app->brush->selected_type];
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
    // Get target area and draw a single tile highlight over all coordinates.
    int target_area_size = get_target_size(MAX_TARGET_RADIUS);
    struct SandboxPoint target_area[target_area_size + 1];
    get_brush_target_area(app->brush, app->sandbox, target_area);
    
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
        SDL_CHECK(SDL_SetTextureScaleMode(TILE_TEXTURES[i], SDL_SCALEMODE_PIXELART));
        SDL_CHECK(SDL_SetTextureScaleMode(PANEL_TEXTURES[i], SDL_SCALEMODE_PIXELART));
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

    // Initialize the universal alpha pixel format
    ALPHA_PIXEL_FORMAT = SDL_CHECK(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32));

    // Initialize all colors used by tiles. 
    // Take pixel RGB at (0, 0) as representative of color of whole tile.
    SDL_Point topleft = {.x = 0, .y = 0};

    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        SDL_Surface *tile_surface = SDL_CHECK(IMG_Load(TILE_TEXTURE_FILENAMES[i]));
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
        SDL_Log("\nWARNING: %s:%d\nReason: Attempted to destroy NULL textures!\n", __FILE__, __LINE__);
    }

    for (int i = 0; i < NUM_TILE_TYPES; i++)
    {
        SDL_DestroyTexture(TILE_TEXTURES[i]);
        SDL_DestroyTexture(PANEL_TEXTURES[i]);

        TILE_TEXTURES[i] = nullptr;
        PANEL_TEXTURES[i] = nullptr;
    }
}


/**
 * Free any memory the passed GUI application takes up.
 * 
 * Passing NULL to this function causes nothing to happen.
 *
 * @param app Owning GUI application to free.
 */
static void destroy_app(struct Application *app)
{
    if (app == nullptr)
    {
        return;
    }

    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    destroy_sandbox(app->sandbox);
    destroy_brush(app->brush);
    destroy_cursor(app->cursor);
    free(app);
    destroy_textures();
}


// ----- PUBLIC FUNCTIONS -----


struct Application *create_app(const char *title, struct Sandbox *sandbox)
{
    init_tile_colors();
    struct Application *app = SAFE_MALLOC(sizeof(*app));

    // Initialize window screen dimensions as a scale of the sandbox dimensions.
    app->sandbox = sandbox;
    app->min_window_width = sandbox->width * (int) TILE_SCALE;
    app->min_window_height = sandbox->height * (int) TILE_SCALE;

    unsigned int window_flags = SDL_WINDOW_RESIZABLE;

    // Init all SDL subsystems and library extensions.
    SDL_CHECK(SDL_Init(SDL_INIT_VIDEO));

    app->window = SDL_CHECK(SDL_CreateWindow(title, 
                                             app->min_window_width,
                                             app->min_window_height,
                                             window_flags
                                            ));
    SDL_CHECK(SDL_SetWindowMinimumSize(app->window, app->min_window_width, app->min_window_height));

    // Create renderer using the first graphics acceleration device found.
    // Set a logical drawing area for automatic resolution scaling of rendered contents.
    // Logical area is big enough to render sandbox at full resolution.
    app->renderer = SDL_CHECK(SDL_CreateRenderer(app->window, nullptr));
    SDL_CHECK(SDL_SetRenderLogicalPresentation(app->renderer, app->min_window_width, app->min_window_height, SDL_LOGICAL_PRESENTATION_LETTERBOX));

    // Initialize members which track GUI user interaction.
    app->cursor = create_cursor();
    app->brush = create_brush();

    // Enable alpha blending for transparent textures on renderer and allocate
    // all textures.
    SDL_CHECK(SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND));
    init_textures(app);
    return app;
}


void quit_app(struct Application *app)
{
    destroy_app(app);
    SDL_Quit();
    exit(EXIT_SUCCESS);
}


SDL_Texture *load_texture(struct Application *app, const char *filename)
{
    // Call to SDL_image to load image.
    SDL_Texture *texture = SDL_CHECK(IMG_LoadTexture(app->renderer, filename));
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
    SDL_Surface *raw_surface = SDL_CHECK(IMG_Load(filename));
    SDL_Surface *alpha_surface = SDL_CHECK(SDL_ConvertSurface(raw_surface, ALPHA_PIXEL_FORMAT->format));
    SDL_Texture *texture = SDL_CHECK(SDL_CreateTextureFromSurface(app->renderer, alpha_surface));
    SDL_CHECK(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND));
    SDL_CHECK(SDL_SetTextureAlphaMod(texture, alpha));

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
    SDL_CHECK(SDL_GetTextureSize(texture, &dest.w, &dest.h));

    // Draw texture, passing in NULL to copy whole texture.
    SDL_CHECK(SDL_RenderTexture(app->renderer, texture, nullptr, &dest));
}


void blit_rectangle(struct Application *app, SDL_FRect rect, SDL_Color color, bool do_fill)
{
    // Draw a fill rectangle or rectangle outline, depending on parameter.
    SDL_CHECK(SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a));
    bool (*rect_blitter)(SDL_Renderer *, const SDL_FRect *);
    rect_blitter = do_fill ? SDL_RenderFillRect : SDL_RenderRect;
    SDL_CHECK(rect_blitter(app->renderer, &rect));
}


void set_black_background(struct Application *app)
{
    SDL_CHECK(SDL_SetRenderDrawColor(app->renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a));
    SDL_CHECK(SDL_RenderClear(app->renderer));
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
    SDL_CHECK(SDL_SetTextureColorMod(tile_texture, variant.r, variant.g, variant.b));
    blit_texture(app, tile_texture, window_coords);
}


void draw_ui(struct Application *app)
{
    draw_highlight(app);

    // Draw panel texture and blit to topleft of screen.
    SDL_Texture *panel_texture = PANEL_TEXTURES[app->brush->selected_type];
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
                quit_app(app);
                break;

            // We obtain mouse coordinates in an event, as unlike SDL_GetMouseState(),
            // mouse coordinates captured this way can be auto-scaled to match
            // logical render size, transformed from window space.
            case SDL_EVENT_MOUSE_MOTION:
                // TODO: Move this logic into a do_mouse_motion() function.
                SDL_CHECK(SDL_ConvertEventToRenderCoordinates(app->renderer, &event));
                app->cursor->coords = (SDL_FPoint) {.x = event.motion.x, .y = event.motion.y};
                app->brush->coords = scale_screen_coords(app->cursor->coords, app->sandbox);
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
    // Interacting places/deletes tiles on the owned sandbox.
    if (app->cursor->is_holding_interact)
    {
        brush_alter_tile(app->brush, app->sandbox);
    }
}
