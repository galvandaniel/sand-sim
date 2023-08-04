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
    if (event -> button == SDL_BUTTON_LEFT)
    {
        app -> mouse -> is_left_clicking = false;
    }
}


/*
 * Unload all tile textures from memory, destroying them and freeing the array
 * of tile_textures.
 */
static void _destroy_tile_textures(void)
{
    for (int i = 0; i < NUM_UNIQUE_TILES; i++)
    {
        SDL_DestroyTexture(TILE_TEXTURES[i]);
    }

    free(TILE_TEXTURES);
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

    app -> mouse = (struct Mouse *) malloc(sizeof(struct Mouse));

    return app;
}


void init_tile_textures(struct Application *app)
{
    // Allocate memory for array to hold pointers to all textures.
    TILE_TEXTURES = (SDL_Texture **) malloc(NUM_UNIQUE_TILES * sizeof(SDL_Texture *));

    // Load all tile textures.
    TILE_TEXTURES[AIR] = load_texture(app, "assets/air.png");
    TILE_TEXTURES[SAND] = load_texture(app, "assets/sand.png");
    TILE_TEXTURES[WATER] = load_texture(app, "assets/water.png");

    // Placeholder textures while other tiles are being implemented.
    TILE_TEXTURES[WOOD] = load_texture(app, "assets/air.png");
    TILE_TEXTURES[STEAM] = load_texture(app, "assets/air.png");
    TILE_TEXTURES[FIRE] = load_texture(app, "assets/air.png");

    for (int i = 6; i < NUM_UNIQUE_TILES; i++)
    {
        TILE_TEXTURES[i] = load_texture(app, "assets/air.png");
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
    _destroy_tile_textures();

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

    switch (tile_type)
    {
        case AIR:
            return TILE_TEXTURES[AIR];

        case SAND:
            return TILE_TEXTURES[SAND];

        case WATER:
            return TILE_TEXTURES[WATER];

        case WOOD:
            return TILE_TEXTURES[WOOD];

        default:
            return TILE_TEXTURES[AIR];
    }
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


void get_input(struct Application *app)
{
    // Update mouse coordinates within app.
    SDL_GetMouseState(&(app -> mouse -> x), &(app -> mouse -> y));

    // Prevent mouse data from exceeding sandbox index boundaries if window
    // gets resized.
    if (app -> mouse -> x > WINDOW_WIDTH)
    {
        app -> mouse -> x = WINDOW_WIDTH - 1;
    }

    if (app -> mouse -> y > WINDOW_HEIGHT)
    {
        app -> mouse -> y = WINDOW_HEIGHT - 1;
    }

    // Take in an input event and react.
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
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

            default:
                break;
        }
    }
}


int main(void)
{
    // Initialize SDL, create an app, and load in textures.
    struct Application *app = init_gui("Sandbox");
    init_tile_textures(app);

    // Form a sandbox.
    unsigned char **sandbox = create_sandbox(SANDBOX_HEIGHT, SANDBOX_WIDTH);
    sandbox[2][0] = 1;
    sandbox[2][1] = 2;
    sandbox[2][2] = 2;

    while (true)
    {
        // Render full black to the window.
        set_black_background(app);

        get_input(app);
        process_sandbox(sandbox, SANDBOX_HEIGHT, SANDBOX_WIDTH);
        draw_sandbox(app, sandbox, SANDBOX_HEIGHT, SANDBOX_WIDTH);

        SDL_RenderPresent(app -> renderer);
        SDL_Delay(16);
    }

    return 0;
}
