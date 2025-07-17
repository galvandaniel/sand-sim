/**
 * Tiles have the following 4 bit flags, starting from most significant bit:
 *
 * 1. Update - If the tile has already been updated in the current update pass.
 * 2. (UNUSED)
 * 3. (UNUSED)
 * 4. (UNUSED)
 *
 * The remaining 4 bits are used for tile ID numbers, going as follow:
 *
 * 0 - Air (empty tile)
 * 1 - Sand
 * 2 - Water
 * 3 - Wood
 * 4 - Steam
 * 5 - Fire
 * 6-15 - (UNUSED)
 *
 */

#include "sandbox.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


// Lifetime sandbox has existed for begins at 0 frames, 0 seconds.
int SANDBOX_LIFETIME = 0;


// ----- STATIC FUNCTIONS -----


/**
 * Swap the tiles located at the two coordinates within sandbox.
 *
 * @param row_one, column_one Coordinates of first tile.
 * @param row_two, column_two Coordinates of second tile.
 * @param sandbox Sandbox to mutate by swapping first and second tile.
 */
static void _swap_tiles(int row_one, int column_one, int row_two, int column_two, unsigned char **sandbox)
{
    unsigned char temp = sandbox[row_one][column_one];
    sandbox[row_one][column_one] = sandbox[row_two][column_two];
    sandbox[row_two][column_two] = temp;
}


/**
 * Generate a integer between the given ranges, inclusive.
 * 
 * This function is not needed and will always generate the same sequence.
 * 
 * @param min, max Inclusive bounds of the random integer to be generated.
 * 
 * @return Random integer in the interval [min, max].
 */
static int randint(int min, int max)
{
    return min + (rand() % (max + 1 - min));
}

/**
 * Flip a coin, generating either heads or tails.
 *
 * This function is not seeded and will always generate the same sequence.
 *
 * @return True for heads, false for tails.
 */
static bool _flip_coin(void)
{
    // Generate a random number between 0 and 1.
    float random_value = (float) rand() / (float) RAND_MAX;

    if (random_value > 0.5)
    {
        return 1;
    }

    return 0;
}


/**
 * Return whether the given tile is an empty space or not.
 * 
 * @param tile Tile to determine if is empty and replaceable or not.
 * @return True if tile is empty, false otherwise.
 */
static bool _tile_is_empty(unsigned char tile)
{
    return get_tile_id(tile) == AIR;
}


/**
 * Determine wheter the two given tiles have the type tile type.
 * 
 * @param tile, other_tile Tiles to determine if they have same type.
 * 
 * @return True if tiles have same tile type, false otherwise.
 */
static bool _are_tiles_same_type(unsigned char tile, unsigned char other_tile)
{
    return get_tile_id(tile) == get_tile_id(other_tile);
}


/**
 * Return whether the given tile is affected by gravity or not.
 *
 * @param tile Tile to determine if it has gravity or not.
 *
 * @return True if tile is affected by gravity, false otherwise.
 */
static bool _tile_has_gravity(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);

    // We return, so no break is needed to prevent fall-through.
    switch (tile_type)
    {
        case AIR:
            return false;

        case SAND:
            return true;

        case WATER:
            return true;

        case WOOD:
            return false;

        case STEAM:
            return false;

        case FIRE:
            return false;

        default:
            return false;
    }
}


/**
 * Return whether or not a tile is "solid".
 *
 * A tile is considered solid if it capable of acting as a "floor" and can block
 * fluid.
 * Intuitively, this means you could sensibly stand on the tile.
 *
 * For example, air and fire are not solid. Sand and wood are solid.
 *
 * @param tile Tile to determine if is solid or not.
 * 
 * @return True if tile is solid, false otherwise.
 */
static bool _tile_is_solid(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);

    switch (tile_type)
    {
        case AIR:
            return false;

        case SAND:
            return true;

        case WATER:
            return false;

        case WOOD:
            return true;

        case STEAM:
            return false;

        case FIRE:
            return false;

        default:
            return false;
    }
}


/**
 * Return whether the given tile is "liquid". 
 * 
 * A tile is considered liquid if it is able to flow ontop of solids and other 
 * liquids.
 * 
 * For example, water is a liquid. Wood and steam are not liquids.
 *
 * @param tile Tile to determine if is liquid or not.
 *
 * @return True if the tile type is liquid and there has flow, false otherwise.
 */
static bool _tile_is_liquid(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);

    switch (tile_type)
    {
        case AIR:
            return false;

        case SAND:
            return false;

        case WATER:
            return true;

        case WOOD:
            return false;

        case STEAM:
            return false;

        case FIRE:
            return false;

        default:
            return false;
    }
}


/**
 * Return whether the given tile is a "gas" 
 * 
 * A tile is considered a gas if lifts into the air and is able to permeate
 * through liquids and other gasses.
 *
 *
 * @param tile Tile to determine if is gas and has lift or not.
 *
 * @return True if tile is a gas and lifts into the air, false otherwise.
 */
static bool _tile_is_gas(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);

    switch (tile_type)
    {
        // Air refers to the empty tile, it is not a gas.
        case AIR:
            return false;

        case SAND:
            return false;

        case WATER:
            return false;

        case WOOD:
            return false;

        case STEAM:
            return true;

        case FIRE:
            return true;

        default:
            return false;
    }
}


/**
 * Return whether or not a tile dissolves in fluids therefore has a chance to
 * flow laterally through liquids.
 * 
 * @param tile Tile to determine if dissolves in liquid.
 * 
 * @return True if tile dissolves in liquids like water, false otherwise.
 */
static bool _tile_dissolves(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);

    switch (tile_type)
    {
        case AIR:
            return false;

        case SAND:
            return false;

        case WATER:
            return false;

        case WOOD:
            return false;

        case STEAM:
            return false;

        case FIRE:
            return false;

        default:
            return false;
    }
}


/**
 * Perform a slide on the given tile coordinates within the sandbox, if possible.
 *
 * A slide is possible if the tile in question has room to move to the left
 * or to the right one tile, without going out of bounds.
 *
 * If both are possible, perform one at random.
 *
 * This function does NOT check under what conditions a tile should slide.
 *
 * Helper function to do_liquid_flow and do_lift.
 *
 * @param sandbox Sandbox to potentialy mutate by moving tiles for sliding.
 * @param width Width of given sandbox.
 * @param row_index, column_index Coordinates of tile to slide.
 */
void _slide_left_or_right(unsigned char **sandbox, int width, int row_index, int column_index)
{
    int left_column = column_index - 1;
    int right_column = column_index + 1;

    // Only slide in a direction if there's an empty space, and it's not OOB.
    bool can_slide_left = left_column != -1 && _tile_is_empty(sandbox[row_index][left_column]);
    bool can_slide_right = right_column != width && _tile_is_empty(sandbox[row_index][right_column]);

    // If we can flow both directions, choose one at random on a coin flip.
    if (can_slide_left && can_slide_right)
    {
        bool heads = _flip_coin();

        if (heads)
        {
            _swap_tiles(row_index, column_index, row_index, left_column, sandbox);
        }
        else
        {
            _swap_tiles(row_index, column_index, row_index, right_column, sandbox);
        }
        return;
    }

    // If only one option is available, do that.
    if (can_slide_left)
    {
        _swap_tiles(row_index, column_index, row_index, left_column, sandbox);
        return;
    }
    if (can_slide_right)
    {
        _swap_tiles(row_index, column_index, row_index, right_column, sandbox);
        return;
    }
}


/**
 * Determine if the given sandbox coordinates can be lifted to the target coordinates.
 * 
 * It is assumed that the target coordinates are valid coordinates which can be
 * lifted, coordinates are not checked for consistency with anti-gravity logic.
 */
static bool _can_lift(unsigned char **sandbox, 
                      int width, 
                      int row_index, 
                      int column_index, 
                      int target_row, 
                      int target_column)
{
    // If attempting to lift OOB, reject.
    if (target_row == -1 || target_column == -1 || target_column == width)
    {
        return false;
    }

    unsigned char source_tile = sandbox[row_index][column_index];
    unsigned char target_tile = sandbox[target_row][target_column];

    return (_tile_is_empty(target_tile)
         || _tile_is_liquid(target_tile)
         || (_tile_is_gas(target_tile) && !_are_tiles_same_type(source_tile, target_tile)));
}


/**
 * Determine if the given sandbox coordinates can sink to the target coordaintes.
 * 
 * It is assumed that the target coordinates are valid coordinates which can be
 * sunk to, coordinates are not checked for consistency with gravity logic.
 */
static bool _can_sink(unsigned char **sandbox,
                      int height,
                      int width, 
                      int row_index,
                      int column_index,
                      int target_row,
                      int target_column)
{
    // Cannot sink if doing so goes OOB.
    if (target_row == height || target_column == -1 || target_column == width)
    {
        return false;
    }

    unsigned char source_tile = sandbox[row_index][column_index];
    unsigned char target_tile = sandbox[target_row][target_column];

    return (_tile_is_liquid(target_tile)
         && !_are_tiles_same_type(source_tile, target_tile)
         && !_tile_dissolves(source_tile));
}


// ----- PUBLIC FUNCTIONS -----


unsigned char **create_sandbox(int height, int width)
{
    // Allocate memory for each row.
    unsigned char **new_sandbox = malloc((unsigned long) height * sizeof(*new_sandbox));

    // Then allocate memory for each tile within each row, setting each tile to
    // 0, which corresponds to air.
    for (int row_index = 0; row_index < height; row_index++)
    {
        new_sandbox[row_index] = calloc((unsigned long) width, sizeof(**new_sandbox));
    }

    return new_sandbox;
}


void sandbox_free(unsigned char **sandbox, int height)
{
    // First, free each row as an array of bytes.
    // Then, free the array of pointers that pointed to each array of bytes.
    for (int row_index = 0; row_index < height; row_index++)
    {
        free(sandbox[row_index]);
    }

    free(sandbox);
}


void process_sandbox(unsigned char **sandbox, int height, int width)
{
    // Iterate through whole sandbox, applying updates where necessary.
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            unsigned char current_tile = sandbox[row][col];
            bool is_updated = is_tile_updated(current_tile, SANDBOX_LIFETIME);

            // Do not simulate an empty tile.
            if (_tile_is_empty(current_tile))
            {
                continue;
            }

            // Do not simulate a tile that has already been updated.
            if (is_updated)
            {
                continue;
            }

            // Reaching this point means an update-check MUST occur, even if
            // it results in nothing changing, so we mark the tile as updated.
            // Take care to mutate the array element, NOT the stack-variable.
            set_tile_updated(&sandbox[row][col], SANDBOX_LIFETIME);
           

            // Perform gravity on the tiles that need it.
            if (_tile_has_gravity(current_tile))
            {
                do_gravity(sandbox, height, width, row, col);
            }

            // Perform flow on liquid tiles.
            if (_tile_is_liquid(current_tile))
            {
                do_liquid_flow(sandbox, height, width, row, col);
            }

            // Perform lift on gasses
            if (_tile_is_gas(current_tile))
            {
                do_lift(sandbox, height, width, row, col);
            }

            // A swap could have just happened, so update the tile again to
            // prevent updating a swapped tile.
            set_tile_updated(&sandbox[row][col], SANDBOX_LIFETIME);
        }
    }

    // For every frame of processing, the sandbox grows older.
    SANDBOX_LIFETIME++;
}


unsigned char get_tile_id(unsigned char tile)
{
    // Use a mask of (0000 1111) to extract the first 4 bits.
    unsigned char tile_mask = 0x0f;
    unsigned char tile_id = tile_mask & tile;

    return tile_id;
}


bool is_tile_updated(unsigned char tile, int current_time)
{
    unsigned char time_parity = get_time_parity(current_time);
    unsigned char updated_flag = get_updated_flag(tile);

    return updated_flag == time_parity;
}


void set_tile_updated(unsigned char *tile, int current_time)
{
    unsigned char time_parity = get_time_parity(current_time);

    // Masks of (0111 1111) and (1000 0000) for off/on switching of first bit.
    unsigned char turn_off = 127;
    unsigned char turn_on = 128;

    // In the case of 0, we're updating a tile bit flag of 1 to 0, so we AND.
    // In the case of 1, we're updating a tile bit flag of 0 to 1, so we OR.
    if (time_parity == 0)
    {
        *tile &= turn_off;
    }
    else
    {
        *tile |= turn_on;
    }
}


void do_gravity(unsigned char **sandbox, int height, int width, int row_index, int column_index)
{
    int next_row = row_index + 1;

    // Don't simulate gravity if doing so would take us out of bounds.
    if (next_row == height)
    {
        return;
    }

    // First check if tile can sink/fall directly down.
    bool can_sink_below = _can_sink(sandbox, height, width, row_index, column_index, next_row, column_index);
    unsigned char tile_below = sandbox[next_row][column_index];

    if (_tile_is_empty(tile_below) || can_sink_below)
    {
        _swap_tiles(row_index, column_index, next_row, column_index, sandbox);
        return;
    }

    int left_column = column_index - 1;
    int right_column = column_index + 1;

    // The left and right borders of the sandbox are considered walls.
    bool is_wall_on_left = left_column == -1 || _tile_is_solid(sandbox[row_index][left_column]);
    bool is_wall_on_right = right_column == width || _tile_is_solid(sandbox[row_index][right_column]);

    // Cannot slide or sink downward if there are blocking walls on left and right.
    if (is_wall_on_left && is_wall_on_right)
    {
        return;
    }

    // Now check if tile can sink/slide diagonally instead.
    bool can_sink_bottomleft = _can_sink(sandbox, height, width, row_index, column_index, next_row, left_column);
    bool can_sink_bottomright = _can_sink(sandbox, height, width, row_index, column_index, next_row, right_column);

    bool can_slide_bottomleft = !is_wall_on_left && _tile_is_empty(sandbox[next_row][left_column]);
    bool can_slide_bottomright = !is_wall_on_right && _tile_is_empty(sandbox[next_row][right_column]);

    // If we can both slide/sink down left and right, choose one at random.
    if ((can_slide_bottomleft && can_slide_bottomright)
        || (can_sink_bottomleft && can_sink_bottomright))
    {
        bool heads = _flip_coin();

        if (heads)
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
    if (can_slide_bottomleft || can_sink_bottomleft)
    {
        _swap_tiles(row_index, column_index, next_row, left_column, sandbox);
        return;
    }
    if (can_slide_bottomright || can_sink_bottomright)
    {
        _swap_tiles(row_index, column_index, next_row, right_column, sandbox);
        return;
    }
}


void do_liquid_flow(unsigned char **sandbox, int height, int width, int row_index, int column_index)
{
    int next_row = row_index + 1;

    // Liquid can't flow if not on solid footing or not ontop of another liquid.
    // Being on the botton of sandbox counts as being on solid footing.
    bool on_solid_ground = next_row == height || _tile_is_solid(sandbox[next_row][column_index]);
    if (!on_solid_ground && !_tile_is_liquid(sandbox[next_row][column_index]))
    {
        return;
    }

    _slide_left_or_right(sandbox, width, row_index, column_index);
}


void do_lift(unsigned char **sandbox, int height, int width, int row_index, int column_index)
{
    int next_row = row_index - 1;
    int left_column = column_index - 1;
    int right_column = column_index + 1;

    // Determine the directions tile can life to.
    bool can_ascend_up = _can_lift(sandbox, width, row_index, column_index, next_row, column_index);
    bool can_ascend_left = _can_lift(sandbox, width, row_index, column_index, next_row, left_column);
    bool can_ascend_right = _can_lift(sandbox, width, row_index, column_index, next_row, right_column);
    bool can_slide_left = _can_lift(sandbox, width, row_index, column_index, row_index, left_column);
    bool can_slide_right = _can_lift(sandbox, width, row_index, column_index, row_index, right_column);

    // Pick one movement option at random by collecting the possible target
    // indices into an array.
    bool movement_options[] = {can_ascend_up, 
                               can_ascend_left, 
                               can_ascend_right,
                               can_slide_left,
                               can_slide_right};
    int index_options[5][2] = {{next_row, column_index},
                               {next_row, left_column},
                               {next_row, right_column},
                               {row_index, left_column},
                               {row_index, right_column}};
    int target_indices[5][2];
    int num_true = 0;
    for (int i = 0; i < 5; i++)
    {
        if (movement_options[i])
        {
            target_indices[num_true][0] = index_options[i][0];
            target_indices[num_true][1] = index_options[i][1];
            num_true++;
        }
    }

    // No ascension is possible if cannot move any way upwards.
    if (num_true == 0)
    {
        return;
    }

    int random_index = randint(0, num_true - 1);
    int target_row = target_indices[random_index][0];
    int target_column = target_indices[random_index][1];
    _swap_tiles(row_index, column_index, target_row, target_column, sandbox);
}


unsigned char get_time_parity(int current_time)
{
    // Use a mask of (0000 ... 0001) to extract the first bit, granting parity.
    return current_time & 1;
}

bool get_updated_flag(unsigned char tile)
{
    // The updated flag is the last bit of a tile.
    return tile >> 7;
}



