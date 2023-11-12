/*
 * Implementation of gui.h interface.
 *
 */

#include "gui.h"


// There are at most 16 unique tile IDs, and therefore 16 unique textures.
const static int NUM_UNIQUE_TILES = 16;

// Dimensions are chosen by user at the command line.
unsigned int SANDBOX_WIDTH = 80;
unsigned int SANDBOX_HEIGHT = 45;

unsigned int WINDOW_WIDTH;
unsigned int WINDOW_HEIGHT;


SDL_Texture **TILE_TEXTURES;

SDL_Texture **PANEL_TEXTURES;

// ----- PRIVATE FUNCTIONS -----

/*
 * Update mouse button pressed-down data in the given app by extracting mouse 
 * data from the mouse button event.
 *
 * @param app - App containing mouse click data to update.
 * @param event - Mouse event containing mouse data to extract.
 */
static void _do_mouse_button_down(struct Application *app, SDL_MouseButtonEvent *event)
{
    if (event -> button == SDL_BUTTON_LEFT)
    {
        app -> mouse -> is_left_clicking = true;
    }
}


/*
 * Update mouse button lift-up data in the given app by extracting mouse data
 * from the mouse button event.
 *
 * @param app - App containing mouse click data to update.
 * @param event - Mouse event containing mouse data to extract.
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


/*
 * Perform any application updates that need to occur as a result of any
 * keyboard keypress.
 *
 * @param app - App to mutate as a result of keypress.
 * @param event - Keyboard event containing data on what key was pressed.
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

        // In an unhandled keypress, do nothing.
        default:
            break;

    }
}


/*
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
    struct Application *app = (struct Application *) malloc(sizeof(struct Application));

    // Setup flags for window and renderer creation.
    int renderer_flags = SDL_RENDERER_ACCELERATED;
    int window_flags = 0;

    // Attempt to initialize SDL2 video subsystem.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("(ERROR) Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    // Init SDL_Image for use with PNGs and JPGs
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    // Initialize window dimensions as a scale of the sandbox dimensions.
    WINDOW_WIDTH = SANDBOX_WIDTH * PIXEL_SCALE;
    WINDOW_HEIGHT = SANDBOX_HEIGHT * PIXEL_SCALE;

    // Create app window once video is initialized.
    app -> window = SDL_CreateWindow(title, 
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            window_flags);

    // Check if window creation succeeded.
    if (app -> window == NULL)
    {
        printf("Failed to open a %d by %d window: %s\n", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_GetError());
        exit(1);
    }

    // Use linear interpolation to scale resolution.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    // Create renderer using the first graphics acceleration device found.
    app -> renderer = SDL_CreateRenderer(app -> window, -1, renderer_flags);

    if (app -> renderer == NULL)
    {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    // Initialize a new mouse with zero in every field, except for selected tile,
    // and assign it to the application.
    struct Mouse *new_mouse = (struct Mouse *) calloc(1, sizeof(struct Mouse));
    new_mouse -> selected_tile = SAND;
    app -> mouse = new_mouse;

    // Allocate memory for all textures used by the 16 possible tile types.
    init_textures(app);

    return app;
}


void init_textures(struct Application *app)
{
    // Allocate memory for array to hold pointers to all textures.
    TILE_TEXTURES = (SDL_Texture **) malloc(NUM_UNIQUE_TILES * sizeof(SDL_Texture *));
    PANEL_TEXTURES = (SDL_Texture **) malloc(NUM_UNIQUE_TILES * sizeof(SDL_Texture *));

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

    SDL_Quit();
}


SDL_Texture *load_texture(struct Application *app, char *filename)
{
    // Call SDL_image to load image.
    SDL_Texture *texture = IMG_LoadTexture(app -> renderer, filename);
    return texture;
}


void blit_texture(struct Application *app, SDL_Texture *texture, int x, int y)
{
    // Setup rectangle to draw texture.
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;

    // Fill in rectangle dimension data by querying the texture.
    SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);

    // Draw texture, passing in NULL to copy whole texture.
    SDL_RenderCopy(app -> renderer, texture, NULL, &dest);
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
    SDL_SetRenderDrawColor(app -> renderer, 0, 0, 0, 255);

    // Clear the window with the current set color.
    SDL_RenderClear(app -> renderer);
}


void draw_sandbox(struct Application *app, unsigned char **sandbox, unsigned int height, unsigned int width)
{
    for (unsigned int row = 0; row < height; row++)
    {
        for (unsigned int col = 0; col < width; col++)
        {
            unsigned char current_tile = sandbox[row][col];

            // Don't draw air.
            if (get_tile_id(current_tile) == AIR)
            {
                continue;
            }

            // Compute the coordinates that a tile should be blitted at by
            // scaling up their position as dictated by the scale factor.
            unsigned int tile_x = col * PIXEL_SCALE;
            unsigned int tile_y = row * PIXEL_SCALE;

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
    // Update mouse coordinates within app.
    SDL_GetMouseState(&(app -> mouse -> x), &(app -> mouse -> y));

    // Prevent mouse data from exceeding sandbox index boundaries if window
    // gets resized.
    if (app -> mouse -> x > WINDOW_WIDTH - PIXEL_SCALE)
    {
        app -> mouse -> x = WINDOW_WIDTH - PIXEL_SCALE;
    }

    if (app -> mouse -> y > WINDOW_HEIGHT - PIXEL_SCALE)
    {
        app -> mouse -> y = WINDOW_HEIGHT - PIXEL_SCALE;
    }

    // Take in an input event and react.
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // Free memory taken up by app, then shutdown.
            case SDL_QUIT:
                cleanup(app);
                exit(0);

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

            default:
                break;
        }
    }
}


void switch_selected_tile(struct Mouse *mouse, unsigned char tile_type)
{
    // For invalid tile types, do nothing.
    if (tile_type > 15)
    {
        return;
    }
    
    mouse -> selected_tile = tile_type;
}


void place_tile(struct Mouse *mouse,
        unsigned char **sandbox,
        unsigned int height,
        unsigned int width)
{
    // Downscale the mouse coordinates to floating sandbox coordinates.
    float row_coordinate = (float) mouse -> y / PIXEL_SCALE;
    float col_coordinate = (float) mouse -> x / PIXEL_SCALE;

    // Round to the nearest integer to obtain valid sandbox indices.
    unsigned int row_index = roundf(row_coordinate);
    unsigned int col_index = roundf(col_coordinate);

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


int main(int argc, char *argv[])
{
    // Initialize SDL, create an app, and load in textures.
    struct Application *app = init_gui("Sandbox");

    // Form a sandbox.
    unsigned char **sandbox = create_sandbox(SANDBOX_HEIGHT, SANDBOX_WIDTH);

    while (true)
    {
        // Render full black to the window.
        set_black_background(app);

        get_input(app);

        if (app -> mouse -> is_left_clicking)
        {
            place_tile(app -> mouse, sandbox, SANDBOX_HEIGHT, SANDBOX_WIDTH);
        }

        // Do 1 frame of sandbox processing and draw the result to the renderer.
        process_sandbox(sandbox, SANDBOX_HEIGHT, SANDBOX_WIDTH);
        draw_sandbox(app, sandbox, SANDBOX_HEIGHT, SANDBOX_WIDTH);

        // Draw UI elements above the sandbox so that they aren't covered.
        draw_ui(app);

        // Display all rendered graphics.
        SDL_RenderPresent(app -> renderer);

        // Run at ~30 FPS. (wait 33 milliseconds before proceeding to next frame)
        SDL_Delay(33);
    }

    return 0;
}
