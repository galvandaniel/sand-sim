#include "gui/cursor.h"
#include "gui/brush.h"
#include "gui/sdl_utils.h"
#include "utils.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3_image/SDL_image.h>

#include <stdlib.h>

const char *CURSOR_TEXTURE_FILENAMES[] = 
{
    [PLACE] = "assets/cursors/place.png",
    [DELETE] = "assets/cursors/delete.png",
    [REPLACE] = "assets/cursors/replace.png",

    // TODO: Load a proper default Cursor icon here, not the `PLACE` brush icon.
    [NUM_BRUSH_MODES] = "assets/cursors/place.png",
};


struct Cursor *create_cursor()
{
    // Cannot continue if SDL was not initialized first.
    if (!SDL_WasInit(SDL_INIT_VIDEO))
    {
        SDL_Log("\nWARNING: %s:%d\nReason: Attempted to create_cursor() without SDL_Init()!\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    struct Cursor *new_cursor = SAFE_CALLOC(1, sizeof(*new_cursor));

    // Initialize cursors icons used.
    for (int i = 0; i < NUM_CURSOR_ICONS; ++i)
    {
        SDL_Surface *cursor_surface = SDL_CHECK(IMG_Load(CURSOR_TEXTURE_FILENAMES[i]));
        new_cursor->icons[i] = SDL_CHECK(SDL_CreateColorCursor(cursor_surface, 0, 31));
        SDL_DestroySurface(cursor_surface);
    }

    SDL_CHECK(SDL_SetCursor(new_cursor->icons[PLACE]));
    return new_cursor;
}


void destroy_cursor(struct Cursor *cursor)
{
    if (cursor == nullptr)
    {
        return;
    }

    for (int i = 0; i < NUM_CURSOR_ICONS; ++i)
    {
        SDL_DestroyCursor(cursor->icons[i]);
    }
    free(cursor);
}


struct SandboxPoint scale_screen_coords(SDL_FPoint window_coords, struct Sandbox *sandbox)
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
 SDL_FPoint scale_sandbox_coords(struct SandboxPoint sandbox_coords)
{
    float window_x = (float) sandbox_coords.col * TILE_SCALE;
    float window_y = (float) sandbox_coords.row * TILE_SCALE;
    SDL_FPoint window_coords = {.x = window_x, .y = window_y};
    return window_coords;
}