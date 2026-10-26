/**
 * Module which defines a container of a sandbox GUI's 'brush' draw state.
 * 
 * The 'brush' of a sandbox GUI constitutes user ability to interact directly
 * with a sandbox simulation via some input device.
 */

#ifndef BRUSH_H
#define BRUSH_H

#include "sim/tile.h"
#include "sim/sandbox.h"

/**
 * Maximum allowed size of brush target area.
 */
constexpr int MAX_TARGET_RADIUS = 5;

/**
 * Unique modes a sandbox brush can be in.
 * 
 * PLACE: Place tile only on empty spaces.
 * DELETE: Place empty (air) tile on any other tile.
 * REPLACE: Place tile on any other tile.
 */
enum BrushMode {PLACE, DELETE, REPLACE, NUM_BRUSH_MODES};

/**
 * Type maintaining data related to interactive drawing within a sandbox GUI.
 */
struct Brush
{
    // Current centerpoint of brush in terms of some sandbox coordinate space.
    // These need not be always in sync with GUI screen space coordinates.
    struct SandboxPoint coords;

    // Whether sandbox Brush has any affect on simulation at all.
    bool is_active;

    // Radius of square target area in sandbox.
    // The target area determines where tiles are placed.
    int target_radius;

    // Selected tile type to place down.
    enum TileType selected_type;

    // Mode controlling whether brush deletes or places tiles into sandbox,
    // and corresponding cursors for each mode.
    enum BrushMode mode;
};


/**
 * Create and initialize GUI brush draw state container, with brush initialized
 * to be active and PLACE-ing tiles of type SAND.
 * 
 * All other fields are initialized to 0.
 * 
 * @return Cursor which maintains state of primary GUI interaction input device.
 */
struct Brush *create_brush();


/**
 * Free all memory taken up by the given brush. 
 * 
 * If `brush` is NULL, this function does nothing.
 * 
 * @param cursor Brush to free.
 */
void destroy_brush(struct Brush *brush);


/**
 * Switch the brush's selected tile type to the given new_type.
 *
 * @param brush Brush to switch contained tile type of.
 * @param new_type New selected tile type.
 */
void brush_switch_type(struct Brush *brush, enum TileType new_type);


/**
 * Increment brush placement mode of given brush to next mode from current, 
 * rolling over to PLACE if given brush is in mode REPLACE. 
 * 
 * @param brush Brush whose mode to advance.
 */
void brush_increment_mode(struct Brush *brush);


/**
 * Alter the sandbox by changing the tile located at the brush's target
 * coordinates scaled down sandbox coordinates via TILE_SCALE, according to the
 * brush mode. 
 * 
 * @param brush Brush whose coordiantes and selected tile type will be used
 * to mutate a tile in the given sandbox.
 * @param sandbox Sandbox to mutate and alter tile in according to brush data.
 */
void brush_alter_tile(struct Brush *brush, struct Sandbox *sandbox);


/**
 * Compute the size of a brush target area in terms of sandbox tiles.
 * 
 * This function will assert if the given radius is negative.
 * 
 * @param radius Radius of target area, determining size.
 * @return Size of target area in terms of sandbox tiles.
 */
int get_target_size(int radius);


/**
 * Compute a target area inside a given sandbox which surrounds and includes the 
 * given Brush's center point coordinates in terms of sandbox (row, col) coordinates.
 * 
 * Any target area points which would go OOB the given sandbox are not included
 * in the output array of points.
 * 
 * The space for array of coordinates is allocated by the caller as the output
 * parameter target_area. 
 * This array must be of size AT LEAST `get_brush_target_size(brush) + 1`, though 
 * the filled contents of the array are not guaranteed to be this long.
 * 
 * The output array is terminated by the (row, col) coordinates (-1, -1) to 
 * indicate the end of array.
 * 
 * @param brush Brush to use as origin and radius of target area.
 * @param sandbox Sandbox bounding points within computed target area.
 * @param target_area Output to place computed target area sandbox coordonates.
 */
void get_brush_target_area(struct Brush *brush, struct Sandbox *sandbox, struct SandboxPoint *target_area);


#endif // BRUSH_H