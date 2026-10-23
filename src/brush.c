#include "brush.h"
#include "sandbox.h"
#include "utils.h"
#include "tile.h"

#include <stdlib.h>
#include <assert.h>

/**
 * Compute the sidelength of a square target area in terms of sandbox tiles, 
 * given a radius.
 * 
 * @param radius Radius of target area.
 * @return Sidelength of target area in terms of sandbox tiles.
 */
static int compute_target_area_sidelength(int radius)
{
    return (radius * 2) + 1;
}


struct Brush *create_brush()
{
    // Start new brush as active and target radius size 0.
    struct Brush *new_brush = SAFE_CALLOC(1, sizeof(*new_brush));
    new_brush->is_active = true;
    new_brush->selected_type = SAND;
    
    return new_brush;
}


void destroy_brush(struct Brush *brush)
{
    free(brush);
}


void brush_switch_type(struct Brush *brush, enum TileType new_type)
{
    // For invalid tile types, do nothing.
    if (new_type >= NUM_TILE_TYPES)
    {
        return;
    }
    
    brush->selected_type = new_type;
}


void brush_increment_mode(struct Brush *brush)
{
    if (brush == nullptr)
    {
        return;
    }

    brush->mode = (brush->mode + 1) % NUM_BRUSH_MODES;
}


void brush_alter_tile(struct Brush *brush, struct Sandbox *sandbox)
{
    // Get target area and perform mouse mode operation for all tiles in target
    // area.
    int target_area_size = get_target_size(MAX_TARGET_RADIUS);
    struct SandboxPoint target_area[target_area_size + 1];
    get_brush_target_area(brush, sandbox, target_area);

    for (int i = 0; target_area[i].row != -1; ++i)
    {
        switch (brush->mode)
        {
            case PLACE:
                place_tile(sandbox, target_area[i], brush->selected_type);
                break;
            
            case DELETE:
                delete_tile(sandbox, target_area[i]);
                break;
            
            case REPLACE:
                replace_tile(sandbox, target_area[i], brush->selected_type);
                break;

            default:
                break;
        }
    }
}


int get_target_size(int radius)
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


void get_brush_target_area(struct Brush *brush, struct Sandbox *sandbox, struct SandboxPoint *target_area)
{

    int radius = brush->target_radius;
    struct SandboxPoint origin = brush->coords;

    // This should never happen. If it does, program is definitely in invalid state.
    assert(radius >= 0 && "ERROR: Attempted to fill target area of negative radius!\n");

    struct SandboxPoint terminator = {.row = -1, .col = -1};

    // Zero radius is a special case of 1-tile size draw area.
    if (radius == 0)
    {
        target_area[0] = origin;
        target_area[1] = terminator;
        return;
    }

    // Compute non-OOB square of target area by starting from topleft of square
    // and going through the tiles in row-major order.
    // Maintain an index into the array to terminate the end.
    int sidelength = compute_target_area_sidelength(radius);
    struct SandboxPoint topleft = {origin.row - radius, origin.col - radius};
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
