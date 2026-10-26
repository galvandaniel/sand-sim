#ifndef CURSOR_H
#define CURSOR_H

#include "gui/brush.h"
#include "sim/sandbox.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>

/**
 * Scale-factor representing how many pixels in SDL screen space it takes to 
 * draw 1 side of 1 particle tile in the sandbox.
 * Ex: TILE_SCALE = 8 indicates 1 tile particle takes up 64 screen pixels.
 * 
 * The value of this constant is consistent with the (n x n) dimensions of
 * assets/tiles/$(tile_type).png for the sandbox to display as intended.
 * Ex: If sand.png is 8x8, TILE_SCALE is 8.
 */
constexpr float TILE_SCALE = 8;

/**
 * Number of different icons supported by cursor: 
 * As many brush modes that exist, along with a default cursor.
 */
constexpr int NUM_CURSOR_ICONS = NUM_BRUSH_MODES + 1;

extern const char *CURSOR_TEXTURE_FILENAMES[];

/**
 * Type which captures data from an input device and constitutes main method of 
 * interacting with sandbox GUI application.
 * 
 * Captured data is most intuitively controlled by keyboard + mouse, but can
 * capture other input data. Ex: controller input.
 */
struct Cursor
{
    // Subpixel coordinates in terms of GUI screen coordinate space.
    SDL_FPoint coords;

    // Whether user is holding down main interaction button. Typically, LMB.
    bool is_holding_interact;

    // TODO: Move this data into dedicated ButtonInput data type?
    bool is_holding_lctrl;

    // Collection of all different cursor icons that can cursor can be.
    SDL_Cursor *icons[NUM_CURSOR_ICONS];
};


/**
 * Create GUI cursor representation, with cursor icons appropriately initialized
 * as SDL Cursors.
 * 
 * All other fields are initialized to 0.
 * 
 * This function depends on SDL having been initialized to enable cursor icon 
 * creation, and so will quit the calling program if this is not the case.
 * 
 * @return Cursor which maintains state of primary GUI interaction input device.
 */
struct Cursor *create_cursor();


/**
 * Free all memory taken up by the passed GUI cursor. 
 * 
 * If `cursor` is NULL, this function does nothing.
 * 
 * @param cursor GUI cursor to free.
 */
void destroy_cursor(struct Cursor *cursor);


/**
 * Scale the given (x, y) screeen space coordinates down to (row, col) sandbox
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
struct SandboxPoint scale_screen_coords(SDL_FPoint window_coords, struct Sandbox *sandbox);


/**
 * Scale the given (row, col) sandbox coordinates located in some sandbox up to 
 * (x, y) SDL screen space coordinates.
 * 
 * row is scaled by `TILE_SCALE` up to a y screen coordinate.
 * col is scaled by `TILE_SCALE` up to a x screen coordinate.
 * 
 * This function assumes the given coordinates are valid for whatever sandbox
 * they came from.
 * 
 * @param sandbox_coords Coordinates into some sandbox packed as a point.
 * @return Screen coordinates packed into an SDL point.
 */
SDL_FPoint scale_sandbox_coords(struct SandboxPoint sandbox_coords);

#endif // CURSOR_H