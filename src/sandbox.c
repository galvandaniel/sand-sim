/*
 * Tiles have the following 4 bit flags, starting from most significant bit:
 *
 * 1. Update - If the tile has already been updated in the current update pass.
 * 2. Static - If the tile is set to not be updated due to reaching rest.
 * 3. (UNUSED)
 * 4. (UNUSED)
 *
 * The remaining 4 bits are used for tile ID numbers, going as follow:
 *
 * 0 - Air (empty tile)
 * 1 - Sand
 * 2 - Water
 * 3-15 - (UNUSED)
 *
 */

#include "sandbox.h"


// ----- STATIC/PRIVATE FUNCTIONS -----


/*
 * Swap the tiles located at the two coordinates within sandbox.
 *
 * @param row_one, column_one - Coordinates of first tile.
 * @param row_two, column_two - Coordinates of second tile.
 * @param sandbox - Sandbox to mutate by swapping first and second tile.
 */
static void _swap_tiles(unsigned int row_one,
        unsigned int column_one,
        unsigned int row_two,
        unsigned int column_two,
        unsigned char **sandbox)
{
    // Use a temp variable to prevent destroying data before swap is complete.
    unsigned char temp = sandbox[row_one][column_one];
    sandbox[row_one][column_one] = sandbox[row_two][column_two];
    sandbox[row_two][column_two] = temp;
}


/*
 * Obtain the updated flag from a tile, synced to the parity of the time from
 * when it was last updated.
 *
 * This is information is not useful on its own without current time, and so
 * it is a private function.
 *
 * @param tile - Tile to get updated flag from.
 */
static bool _get_updated_flag(unsigned char tile)
{
    return tile >> 7;
}


// ----- PUBLIC FUNCTIONS -----


unsigned char **create_sandbox(unsigned int height, unsigned int width)
{
    // Allocate memory for each row.
    unsigned char **new_sandbox = (unsigned char **) malloc(height * sizeof(unsigned char *));

    // Then allocate memory for each tile within each row, setting each tile to
    // 0, which corresponds to non-static air.
    for (unsigned int row_index = 0; row_index < height; row_index++)
    {
        new_sandbox[row_index] = (unsigned char *) calloc(width, sizeof(unsigned char));
    }

    return new_sandbox;
}


void sandbox_free(unsigned char **sandbox, unsigned int height, unsigned int width)
{
    // First, free each row as an array of bytes.
    // Then, free the array of pointers that pointed to each array of bytes.
    for (unsigned int row_index = 0; row_index < height; row_index++)
    {
        free(sandbox[row_index]);
    }

    free(sandbox);
}


unsigned char get_tile_id(unsigned char tile)
{
    // Use a mask of (0000 1111) to extract the first 4 bits.
    unsigned char tile_mask = 0x0f;
    unsigned char tile_id = tile_mask & tile;

    return tile_id;
}


bool is_tile_updated(unsigned char tile, unsigned int current_time)
{
    // Shift the updated bit flag to the front, and compare.
    unsigned char time_parity = get_time_parity(current_time);
    unsigned char updated_flag = _get_updated_flag(tile);

    return updated_flag == time_parity;
}


void set_tile_updated(unsigned char *tile, unsigned int current_time)
{
    unsigned char time_parity = get_time_parity(current_time);

    // Masks of (0111 1111) and (1000 0000) for off/on switching of first bit.
    unsigned char turn_off = 127;
    unsigned char turn_on = 128;

    // In the case of 0, we're updating a bit flag of 1 to 0, so we AND.
    // In the case of 1, we're updating a bit flag of 0 to 1, so we OR.
    if (time_parity == 0)
    {
        *tile &= turn_off;
    }
    else
    {
        *tile |= turn_on;
    }
}


bool is_tile_static(unsigned char tile)
{
    // Bring static flag to the front, extract first bit.
    unsigned char front_static = tile >> 6;
    return front_static & 1;
}


void set_tile_static(unsigned char *tile, bool should_set_static)
{
    // Masks of (0100 0000) and (1011 1111) for on/off switching of second bit.
    unsigned char turn_off = 191;
    unsigned char turn_on = 64;

    // Depending on what data we seek to copy, AND or OR to avoid destroying data.
    if (should_set_static)
    {
        *tile |= turn_on;
    }
    else
    {
        *tile &= turn_off;
    }
}


void do_gravity(unsigned char **sandbox,
        unsigned int height,
        unsigned int width,
        unsigned int row_index,
        unsigned int column_index)
{
    unsigned int next_row = row_index + 1;

    // Don't simulate gravity if doing so would take us out of bounds.
    if (next_row == height)
    {
        return;
    }

    unsigned char tile_below = sandbox[next_row][column_index];

    // If the tile directly beneath us is empty, fall by one tile.
    if (get_tile_id(tile_below) == AIR)
    {
        _swap_tiles(row_index, column_index, next_row, column_index, sandbox);
        return;
    }

    // Get column indices of tiles to the bottom left and bottom right.
    unsigned int left_column = column_index - 1;
    unsigned int right_column = column_index + 1;

    // Assume that a slide isn't possible.
    bool can_slide_left = false;
    bool can_slide_right = false;

    // Determine if its feasible to slide left, right, or neither.
    // Short-circuiting has us avoid accessing out of bounds index values.
    if (left_column != -1 && get_tile_id(sandbox[next_row][left_column]) == AIR)
    {
        can_slide_left = true;
    }

    if (right_column != width && get_tile_id(sandbox[next_row][right_column]) == AIR)
    {
        can_slide_right = true;
    }

    unsigned char current_tile = sandbox[row_index][column_index];
    // If we have the option of sliding down left or right, choose which one
    // to go down depending on whether we were last updated on an even or odd
    // frame.
    //
    // This simulates randomness and saves having to recompute a random number
    // every frame that sliding gravity occurs.
    if (can_slide_left && can_slide_right)
    {
        bool updated_flag = _get_updated_flag(current_tile);

        if (updated_flag == 0)
        {
            _swap_tiles(row_index, column_index, next_row, left_column, sandbox);
        }
        else
        {
            _swap_tiles(row_index, column_index, next_row, right_column, sandbox);
        }
        return;
    }

    // If there is no choice in direction, do whichever is possible.
    if (can_slide_left)
    {
        _swap_tiles(row_index, column_index, next_row, left_column, sandbox);
        return;
    }

    if (can_slide_right)
    {
        _swap_tiles(row_index, column_index, next_row, right_column, sandbox);
        return;
    }

    // At this point, the tile is in a location where it cannot fall any more.
    // In this situation, it is now considered a static tile.
    // set_static(tile);
}


unsigned char get_time_parity(unsigned int current_time)
{
    // Use a mask of (0000 ... 0001) to extract the first bit, granting parity.
    return current_time & 1;
}


void print_sandbox(unsigned char **sandbox, unsigned int height, unsigned int width)
{
    if (sandbox == NULL)
    {
        return;
    }

    // Iterate through the sandbox, putting newlines at the end of each row
    // and printing each tile as an appropriate character.
    for (unsigned int row = 0; row < height; row++)
    {
        for (unsigned int col = 0; col < width; col++)
        {
            unsigned char current_tile = sandbox[row][col];

            if (get_tile_id(current_tile) == AIR)
            {
                putchar('-');
            }

            else if (get_tile_id(current_tile) == SAND)
            {
                putchar('O');
            }

            else if (get_tile_id(current_tile) == WATER)
            {
                putchar('_');
            }

            if (col == width - 1)
            {
                putchar('\n');
            }
        }
    }
}

