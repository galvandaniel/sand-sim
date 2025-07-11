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
 * 3 - Wood
 * 4 - Steam
 * 5 - Fire
 * 6-15 - (UNUSED)
 *
 */

#include "sandbox.h"


// Lifetime sandbox has existed for begins at 0 frames, 0 seconds.
unsigned int SANDBOX_LIFETIME = 0;


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
 * Flip a coin pseudo-randomly, generating either heads or tails.
 *
 * This function is not seeded, and will always generate the same sequence.
 *
 * @return - 1 for heads, 0 for tails.
 */
static bool _flip_coin(void)
{
    // Generate a random number between 0 and 1.
    float random_value = rand() / (float) RAND_MAX;

    if (random_value > 0.5)
    {
        return 1;
    }

    return 0;
}


/*
 * Return whether the given tile is affected by gravity or not.
 *
 * @param tile - Tile to determine if it has gravity or not.
 *
 * @return - True if tile is affected by gravity, false otherwise.
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


/*
 * Return whether the given tile is affected by liquid flow or not.
 *
 * @param tile - Tile to determine if it has flow or not.
 *
 * @return - True if the tile type flows, false otherwise.
 *
 */
static bool _tile_has_flow(unsigned char tile)
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


/*
 * Return whether the given tile floats into the air naturally or not.
 *
 * Intuitively, lift is the opposite of gravity.
 *
 * @param tile - Tile to determine if it has lift or not.
 *
 * @return - True if tile has lift and floats, false otherwise.
 */
static bool _tile_has_lift(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);

    switch (tile_type)
    {
        // Air refers to the empty tile, it does not receive lift.
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


/*
 * Return whether or not a tile is "solid".
 *
 * A tile is considered solid if it capable of acting as a "floor".
 * Intuitively, this means you could sensibly stand on the tile.
 *
 * For example, air is not solid, but sand is. Fire is not solid, but wood is.
 *
 */
static bool _is_solid(unsigned char tile)
{
    unsigned char tile_type = get_tile_id(tile);

    switch (tile_type)
    {
        case AIR:
            return false;

        case SAND:
            return true;

        case WATER:
            return true;

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


/*
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
 * @param sandbox - Sandbox to potentialy mutate by moving tiles for sliding.
 * @param height, width - Dimensions of given sandbox.
 * @param row_index, column_index - Coordinates of tile to slide.
 */
void _slide_left_or_right(unsigned char **sandbox,
        unsigned int height,
        unsigned int width,
        unsigned int row_index,
        unsigned int column_index)
{
    // Gather info about what's to the left and right.
    unsigned int left_column = column_index - 1;
    unsigned int right_column = column_index + 1;

    // Test to see where we can move...
    // Only slide in a direction if there's an empty space, and it's not OOB.
    bool can_slide_left = false;
    bool can_slide_right = false;

    if (left_column != -1 && get_tile_id(sandbox[row_index][left_column]) == AIR)
    {
        can_slide_left = true;
    }

    if (right_column != width && get_tile_id(sandbox[row_index][right_column]) == AIR)
    {
        can_slide_right = true;
    }

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


void process_sandbox(unsigned char **sandbox, unsigned int height, unsigned int width)
{
    // Iterate through whole sandbox, applying updates where necessary.
    for (unsigned int row = 0; row < height; row++)
    {
        for (unsigned int col = 0; col < width; col++)
        {
            unsigned char current_tile = sandbox[row][col];
            unsigned char tile_type = get_tile_id(current_tile);
            bool is_static = is_tile_static(current_tile);
            bool is_updated = is_tile_updated(current_tile, SANDBOX_LIFETIME);

            // Do not simulate an empty tile.
            if (tile_type == AIR)
            {
                continue;
            }

            // Do not simulate a static tile.
            if (is_static)
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

            // Perform flow on tiles that need it.
            if (_tile_has_flow(current_tile))
            {
                do_liquid_flow(sandbox, height, width, row, col);
            }

            // Perform lift on tiles that need it.
            if (_tile_has_lift(current_tile))
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


bool is_tile_updated(unsigned char tile, unsigned int current_time)
{
    // Shift the updated bit flag to the front, and compare.
    unsigned char time_parity = get_time_parity(current_time);
    unsigned char updated_flag = get_updated_flag(tile);

    return updated_flag == time_parity;
}


void set_tile_updated(unsigned char *tile, unsigned int current_time)
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

    unsigned char current_tile = sandbox[row_index][column_index];
    unsigned char tile_below = sandbox[next_row][column_index];

    // If the tile directly beneath us is empty, fall by one tile.
    if (get_tile_id(tile_below) == AIR)
    {
        _swap_tiles(row_index, column_index, next_row, column_index, sandbox);
        return;
    }

    // If the tile directly beneath us is water, sink through it.
    // However, don't let water sink through itself.
    if (get_tile_id(tile_below) == WATER && get_tile_id(current_tile) != WATER)
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

    // If we have the option of sliding down left or right, choose which one
    // to go down at random by flipping a coin.
    if (can_slide_left && can_slide_right)
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


void do_liquid_flow(unsigned char **sandbox,
        unsigned int height,
        unsigned int width,
        unsigned int row_index,
        unsigned int column_index)
{
    unsigned int next_row = row_index + 1;

    // Liquid can't flow if it's not on solid footing.
    // Being on the botton of sandbox counts as being on solid footing.
    // Short-circuiting prevents OOB memory access.
    if (next_row != height && !_is_solid(sandbox[next_row][column_index]))
    {
        return;
    }

    _slide_left_or_right(sandbox, height, width, row_index, column_index);
}


void do_lift(unsigned char **sandbox,
        unsigned int height,
        unsigned int width,
        unsigned int row_index,
        unsigned int column_index)
{
    bool can_ascend = false;
    bool can_slide = false;

    unsigned int next_row = row_index - 1;

    // We can only potentially ascend if doing so wouldn't take us out of bounds.
    if (next_row != -1)
    {
        can_ascend = true;
    }

    // We can only slide if we're on the ceiling of the sandbox, or if the
    // tile above us is solid and can serve as a ceiling.
    if (next_row == -1 || _is_solid(sandbox[next_row][column_index]))
    {
        can_slide = true;
    }

    // Anti-gravity logic.
    if (can_ascend)
    {
        unsigned char tile_above = sandbox[next_row][column_index];

        // If the tile directly above us is air or is water, rise through it.
        if (get_tile_id(tile_above) == AIR || get_tile_id(tile_above) == WATER)
        {
            _swap_tiles(row_index, column_index, next_row, column_index, sandbox);
            return;
        }

        // Get the tiles to the top left and topright and their tile types.
        unsigned int left_column = column_index - 1;
        unsigned int right_column = column_index + 1;

        // Assume ascending to the left or right isn't possible.
        bool can_ascend_left = false;
        bool can_ascend_right = false;

        // We can only ascend sideways through air, and only if we don't end up OOB.
        if (left_column != -1 && get_tile_id(sandbox[next_row][left_column]) == AIR)
        {
            can_ascend_left = true;
        }

        if (right_column != width && get_tile_id(sandbox[next_row][right_column]) == AIR)
        {
            can_ascend_right = true;
        }

        // If we have a choice of left or right ascension, pick one at random.
        if (can_ascend_left && can_ascend_right)
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

        // If there is only one choice of ascension, do that.
        if (can_ascend_left)
        {
            _swap_tiles(row_index, column_index, next_row, left_column, sandbox);
            return;
        }

        if (can_ascend_right)
        {
            _swap_tiles(row_index, column_index, next_row, right_column, sandbox);
            return;
        }
    }

    if (can_slide)
    {
        _slide_left_or_right(sandbox, height, width, row_index, column_index);
    }

}


unsigned char get_time_parity(unsigned int current_time)
{
    // Use a mask of (0000 ... 0001) to extract the first bit, granting parity.
    return current_time & 1;
}

bool get_updated_flag(unsigned char tile)
{
    // The updated flag is the last bit of a tile.
    return tile >> 7;
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

