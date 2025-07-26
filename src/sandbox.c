/**
 * Tiles have the following 4 bit flags, starting from most significant bit:
 *
 * 1. Update - If the tile has already been updated in the current update pass.
 * 2. (UNUSED)
 * 3. (UNUSED)
 * 4. (UNUSED)
 *
 * The remaining 4 bits are used for tile type identifiers, going as follow:
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

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


const double survival_chances[] = {0.50, 0.87, 0.95, 1.0};


// ----- STATIC FUNCTIONS -----


/**
 * Swap the tiles located at the two coordinates within sandbox grid.
 *
 * @param row_one, col_one Coordinates of first tile.
 * @param row_two, col_two Coordinates of second tile.
 * @param grid Sandbox 2D grid of bytes to mutate by swapping first and second 
 * tile.
 */
static void _swap_tiles(int row_one, int col_one, int row_two, int col_two, unsigned char **grid)
{
    unsigned char temp = grid[row_one][col_one];
    grid[row_one][col_one] = grid[row_two][col_two];
    grid[row_two][col_two] = temp;
}


/**
 * Generate a integer between the given ranges, inclusive.
 * 
 * This function is not seeded and will always generate the same sequence.
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
    return get_tile_type(tile) == AIR;
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
    return get_tile_type(tile) == get_tile_type(other_tile);
}


/**
 * Get the flammability of a tile, the chance a tile has of being lit on fire in
 * next frame when adjacent to any incendiary tile in the cardinal directions.
 * 
 * @param tile Tile to get flammability chance for.
 * 
 * @return Chance of being lit on fire, 0.0 indicating 0%, 1.0 indicating 100%.
 */
static double _tile_flammability(unsigned char tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
        case AIR:
            return 0.0;

        case SAND:
            return 0.0;

        case WATER:
            return 0.0;

        case WOOD:
            return 0.75;

        case STEAM:
            return 0.0;

        case FIRE:
            return 0.0;            

        default:
            return 0.0;
    }
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
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
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
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
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
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
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
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
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
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
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
 * Return whether or not a tile is able to light other flammable tiles on fire 
 * by being close to them.
 * 
 * @param tile Tile to determine if can light other tiles on fire or not.
 * 
 * @return True if tile can light others on fire, false otherwise.
 */
static bool _tile_is_incendiary(unsigned char tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
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
            return true;

        default:
            return false;
    }
}


/**
 * Perform a random roll on whether the given tile should survive to the next
 * frame of simulation or not. This depends on the tile's classification in
 * survival odds.
 * 
 * @param tile Tile to determine if should survive to next frame or not
 * @return True if tile survives, false if tile dies and is replaced by air.
 */
static bool _roll_should_tile_survive(unsigned char tile)
{
    enum lifespan_duration lifespan = get_tile_lifespan(tile);

    // Tile with permanent duration always survives.
    if (lifespan == PERMANENT)
    {
        return true;
    }

    double random_value = (double) rand() / (double) RAND_MAX;
    double chance_of_survival = survival_chances[lifespan];

    return random_value <= chance_of_survival;
}


/**
 * Perform a random roll on whether or not the tile located at the given indices
 * in the sandbox should light on fire and be replaced by a fire tile.
 * 
 * If the given tile cannot burn at all, no randomness or roll is performed.
 * 
 * @param sandbox Sandbox containing tile to test for burn condition.
 * @param row, col Indices of tile to determine if should convert to fire on 
 * next frame or not.
 * @return True if roll succeeds and tile should become fire, false otherwise.
 */
static bool _roll_should_tile_burn(struct Sandbox *sandbox, int row, int col)
{
    double burn_chance = _tile_flammability(sandbox->grid[row][col]);

    // FP comparison is inexact, but here inflammability is assigned the 
    // literal value of 0.0, which is exactly representable in IEEE 754
    if (burn_chance == 0.0)
    {
        return false;
    }

    int row_below = row + 1;
    int row_above = row - 1;
    int left_col = col - 1;
    int right_col = col + 1;
    // Flammable tiles roll for burn if an incendiary tile is directly NSEW.
    int search_area_indices[4][2] = {{row_above, col},
                                     {row, right_col},
                                     {row_below, col},
                                     {row, left_col}};
    // Search for incendiary tile. 
    // If search area goes OOB, default to not incendiary.
    bool is_next_to_incendiary = false;
    for (int i = 0; i < 4; i++)
    {
        int search_row = search_area_indices[i][0];
        int search_col = search_area_indices[i][1];

        if (search_row < 0 || search_row == sandbox->height 
         || search_col < 0 || search_col == sandbox->width)
         {
            continue;
         }

         if (_tile_is_incendiary(sandbox->grid[search_row][search_col]))
         {
            is_next_to_incendiary = true;
            break;
         }
    }

    if (!is_next_to_incendiary)
    {
        return false;
    }
    double random_value = (double) rand() / (double) RAND_MAX;
    return random_value <= burn_chance;
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
 * Helper function to do_liquid_flow.
 *
 * @param sandbox Sandbox to potentialy mutate by moving tiles for sliding.
 * @param row, col Coordinates of tile to slide.
 */
void _slide_left_or_right(struct Sandbox *sandbox, int row, int col)
{
    int left_col = col - 1;
    int right_col = col + 1;

    // Only slide in a direction if there's an empty space, and it's not OOB.
    bool can_slide_left = left_col != -1 && _tile_is_empty(sandbox->grid[row][left_col]);
    bool can_slide_right = right_col != sandbox->width && _tile_is_empty(sandbox->grid[row][right_col]);

    // If we can flow both directions, choose one at random on a coin flip.
    if (can_slide_left && can_slide_right)
    {
        bool heads = _flip_coin();

        if (heads)
        {
            _swap_tiles(row, col, row, left_col, sandbox->grid);
        }
        else
        {
            _swap_tiles(row, col, row, right_col, sandbox->grid);
        }
        return;
    }

    // If only one option is available, do that.
    if (can_slide_left)
    {
        _swap_tiles(row, col, row, left_col, sandbox->grid);
        return;
    }
    if (can_slide_right)
    {
        _swap_tiles(row, col, row, right_col, sandbox->grid);
        return;
    }
}


/**
 * Determine if the given sandbox coordinates can be lifted to the target coordinates.
 * 
 * It is assumed that the target coordinates are valid coordinates which can be
 * lifted, coordinates are not checked for consistency with anti-gravity logic.
 *
 * @param sandbox Sandbox to determine if target coordinates can be lifted to.
 * @param row, col Source coordinates of tile to perform lift.
 * @param target_row, target_col Destination coordinates a tile is trying to
 * lift to.
 * 
 * @return True if the tile at (row, col) can be lifted to 
 * (target_row, target_col), False otherwise.
 */
static bool _can_lift(struct Sandbox *sandbox, int row, int col, int target_row, int target_col)
{
    // If attempting to lift OOB, reject.
    if (target_row == -1 || target_col == -1 || target_col == sandbox->width)
    {
        return false;
    }

    unsigned char source_tile = sandbox->grid[row][col];
    unsigned char target_tile = sandbox->grid[target_row][target_col];

    // A tile can only lift through liquid by going upwards, not sideways.
    bool is_left_or_right = (target_col == col - 1) || (target_col == col + 1);
    bool is_parallel_horizontal = target_row == row && is_left_or_right;
    if (_tile_is_liquid(target_tile) && is_parallel_horizontal)
    {
        return false;
    }

    // Gases lift through liquids and other gases, but only if not passes
    // through own gas type.
    return (_tile_is_empty(target_tile)
         || _tile_is_liquid(target_tile)
         || (_tile_is_gas(target_tile) && !_are_tiles_same_type(source_tile, target_tile)));
}


/**
 * Determine if the given sandbox coordinates can sink to the target coordaintes.
 * 
 * It is assumed that the target coordinates are valid coordinates which can be
 * sunk to, coordinates are not checked for consistency with gravity logic.
 * 
 * @param sandbox Sandbox to determine if target coordinates can be sunk to.
 * @param row, col Source coordinates of tile to perform sink.
 * @param target_row, target_col Destination coordinates a tile is trying to
 * sink to.
 * 
 * @return True if the tile at (row, col) can sink to (target_row, target_col),
 * False otherwise.
 */
static bool _can_sink(struct Sandbox *sandbox, int row, int col, int target_row, int target_col)
{
    // Cannot sink if doing so goes OOB.
    if (target_row == sandbox->height || target_col == -1 || target_col == sandbox->width)
    {
        return false;
    }

    unsigned char source_tile = sandbox->grid[row][col];
    unsigned char target_tile = sandbox->grid[target_row][target_col];

    // Can only sink through a liquid tile, and a liquid cannot sink through 
    // its own type.
    return (_tile_is_liquid(target_tile)
         && !_are_tiles_same_type(source_tile, target_tile)
         && !_tile_dissolves(source_tile));
}


// ----- PUBLIC FUNCTIONS -----


struct Sandbox *create_sandbox(int width, int height)
{
    // Lifetime sandbox has existed for begins at 0 frames, 0 seconds.
    struct Sandbox *new_sandbox = malloc(sizeof(*new_sandbox));
    new_sandbox->width = width;
    new_sandbox->height = height;
    new_sandbox->lifetime = 0;

    // Allocate grid, starting by allocating memory for each row of grid.
    unsigned char **new_grid = malloc((size_t) height * sizeof(*new_grid));

    // Then allocate memory for each tile within each row, setting each tile to
    // 0, which corresponds to air.
    for (int row = 0; row < height; row++)
    {
        new_grid[row] = calloc((size_t) width, sizeof(**new_grid));
    }

    new_sandbox->grid = new_grid;

    return new_sandbox;
}


void sandbox_free(struct Sandbox *sandbox)
{
    // First, free each row as an array of bytes.
    // Then, free the array of pointers that pointed to each array of bytes.
    for (int row = 0; row < sandbox->height; row++)
    {
        free(sandbox->grid[row]);
    }

    free(sandbox->grid);
    free(sandbox);
}


void process_sandbox(struct Sandbox *sandbox)
{
    unsigned char **grid = sandbox->grid;

    // Iterate through whole sandbox, applying updates where necessary.
    for (int row = 0; row < sandbox->height; row++)
    {
        for (int col = 0; col < sandbox->width; col++)
        {
            unsigned char current_tile = grid[row][col];
            bool is_updated = is_tile_updated(current_tile, sandbox->lifetime);

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

            // Perform survival check.
            bool should_survive = _roll_should_tile_survive(current_tile);
            if (!should_survive)
            {
                grid[row][col] = AIR;
                continue;
            }

            // Perform burn check. Resulting fire tile should not be simulated.
            bool should_burn = _roll_should_tile_burn(sandbox, row, col);
            if (should_burn)
            {
                grid[row][col] = create_tile(sandbox, FIRE);
                continue;
            }

            // Reaching this point means an update-check MUST occur, even if
            // it results in nothing changing, so we mark the tile as updated.
            // Take care to mutate the array element, NOT the stack-variable.
            set_tile_updated(&grid[row][col], sandbox->lifetime);

            // Perform extinguish check. Only fire extinguishes. 
            if (get_tile_type(current_tile) == FIRE)
            {
                do_extinguish(sandbox, row, col);
            }

            // Perform gravity on the tiles that need it.
            if (_tile_has_gravity(current_tile))
            {
                do_gravity(sandbox, row, col);
            }

            // Perform flow on liquid tiles.
            if (_tile_is_liquid(current_tile))
            {
                do_liquid_flow(sandbox, row, col);
            }

            // Perform lift on gasses
            if (_tile_is_gas(current_tile))
            {
                do_lift(sandbox, row, col);
            }

            // A swap could have just happened, so update the tile again to
            // prevent updating a swapped tile.
            set_tile_updated(&grid[row][col], sandbox->lifetime);
        }
    }

    // For every frame of processing, the sandbox grows older.
    sandbox->lifetime++;
}


unsigned char create_tile(struct Sandbox *sandbox, enum tile_type new_type)
{
    // New tiles are synced to time to prevent update until next frame.
    unsigned char new_tile = (unsigned char) new_type;
    set_tile_updated(&new_tile, sandbox->lifetime);

    // TODO: Randomization of tile color will go here.

    return new_tile;
}


enum tile_type get_tile_type(unsigned char tile)
{
    // Use a mask of (0000 1111) to extract the first 4 bits.
    unsigned char tile_mask = 0x0f;
    enum tile_type type_id = tile_mask & tile;

    return type_id;
}


enum lifespan_duration get_tile_lifespan(unsigned char tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
        // AIR represents empty tile so lifespan has no significance for it.
        case AIR:
            return PERMANENT;

        case SAND:
            return PERMANENT;

        case WATER:
            return PERMANENT;

        case WOOD:
            return PERMANENT;

        case STEAM:
            return LONG;

        case FIRE:
            return AVERAGE;

        default:
            return PERMANENT;
    }
}


bool is_tile_updated(unsigned char tile, long long current_time)
{
    unsigned char time_parity = get_time_parity(current_time);
    unsigned char updated_flag = get_updated_flag(tile);

    return updated_flag == time_parity;
}


void set_tile_updated(unsigned char *tile, long long current_time)
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


void do_gravity(struct Sandbox *sandbox, int row, int col)
{
    int next_row = row + 1;

    // Don't simulate gravity if doing so would take us out of bounds.
    if (next_row == sandbox->height)
    {
        return;
    }

    // First check if tile can sink through a liquid/fall directly down.
    bool can_sink_below = _can_sink(sandbox, row, col, next_row, col);
    unsigned char tile_below = sandbox->grid[next_row][col];

    if (_tile_is_empty(tile_below) || can_sink_below)
    {
        _swap_tiles(row, col, next_row, col, sandbox->grid);
        return;
    }

    int left_col = col - 1;
    int right_col = col + 1;

    // The left and right borders of the sandbox are considered walls.
    bool is_wall_on_left = left_col == -1 || _tile_is_solid(sandbox->grid[row][left_col]);
    bool is_wall_on_right = right_col == sandbox->width || _tile_is_solid(sandbox->grid[row][right_col]);

    // Cannot slide or sink downward if there are blocking walls on left and right.
    if (is_wall_on_left && is_wall_on_right)
    {
        return;
    }

    // Now check if tile can sink/slide diagonally instead.
    bool can_sink_bottomleft = _can_sink(sandbox, row, col, next_row, left_col);
    bool can_sink_bottomright = _can_sink(sandbox, row, col, next_row, right_col);

    bool can_slide_bottomleft = !is_wall_on_left && _tile_is_empty(sandbox->grid[next_row][left_col]);
    bool can_slide_bottomright = !is_wall_on_right && _tile_is_empty(sandbox->grid[next_row][right_col]);

    // If we can both slide/sink down left and right, choose one at random.
    if ((can_slide_bottomleft && can_slide_bottomright)
        || (can_sink_bottomleft && can_sink_bottomright))
    {
        bool heads = _flip_coin();

        if (heads)
        {
            _swap_tiles(row, col, next_row, left_col, sandbox->grid);
        }
        else
        {
            _swap_tiles(row, col, next_row, right_col, sandbox->grid);
        }
        return;
    }

    // If there is no choice in direction, do whichever is possible.
    if (can_slide_bottomleft || can_sink_bottomleft)
    {
        _swap_tiles(row, col, next_row, left_col, sandbox->grid);
        return;
    }
    if (can_slide_bottomright || can_sink_bottomright)
    {
        _swap_tiles(row, col, next_row, right_col, sandbox->grid);
        return;
    }
}


void do_liquid_flow(struct Sandbox *sandbox, int row, int col)
{
    int next_row = row + 1;

    // Liquid can't flow if not on solid footing or not ontop of another liquid.
    // Being on the botton of sandbox counts as being on solid footing.
    bool on_solid_ground = next_row == sandbox->height || _tile_is_solid(sandbox->grid[next_row][col]);
    if (!on_solid_ground && !_tile_is_liquid(sandbox->grid[next_row][col]))
    {
        return;
    }

    _slide_left_or_right(sandbox, row, col);
}


void do_lift(struct Sandbox *sandbox, int row, int col)
{
    int next_row = row - 1;
    int left_col = col - 1;
    int right_col = col + 1;

    // Capture possible coordinates to lift to and determine whether tile
    // at (row, col) can be lifted to any of them.
    int movement_options[5][2] = {{next_row, col},
                                  {next_row, left_col},
                                  {next_row, right_col},
                                  {row, left_col},
                                  {row, right_col}};
    bool movement_possibilities[5];

    for (int i = 0; i < 5; i++)
    {
        movement_possibilities[i] = _can_lift(sandbox, 
                                              row, 
                                              col, 
                                              movement_options[i][0], 
                                              movement_options[i][1]);
    }

    // Collect the valid movement options into an array then pick one at random.
    int target_indices[5][2];
    int num_true = 0;
    for (int i = 0; i < 5; i++)
    {
        if (movement_possibilities[i])
        {
            target_indices[num_true][0] = movement_options[i][0];
            target_indices[num_true][1] = movement_options[i][1];
            num_true++;
        }
    }

    // No lift is possible if cannot move any way upwards.
    if (num_true == 0)
    {
        return;
    }

    int random_index = randint(0, num_true - 1);
    int target_row = target_indices[random_index][0];
    int target_col = target_indices[random_index][1];
    _swap_tiles(row, col, target_row, target_col, sandbox->grid);
}


void do_extinguish(struct Sandbox *sandbox, int row, int col)
{
    int row_below = row + 1;
    int row_above = row - 1;
    int left_col = col - 1;
    int right_col = col + 1;
    // Check for water in cardinal directions.
    int search_area_indices[4][2] = {{row_above, col},
                                     {row, right_col},
                                     {row_below, col},
                                     {row, left_col}}; 
    bool is_next_to_water = false;
    for (int i = 0; i < 4; i++)
    {
        int search_row = search_area_indices[i][0];
        int search_col = search_area_indices[i][1];

        // If search area goes OOB, default to not water.
        if (search_row < 0 || search_row == sandbox->height 
         || search_col < 0 || search_col == sandbox->width)
         {
            continue;
         }

         if (get_tile_type(sandbox->grid[search_row][search_col]) == WATER)
         {
            is_next_to_water = true;
            break;
         }
    }

    if (!is_next_to_water)
    {
        return;
    }

    sandbox->grid[row][col] = create_tile(sandbox, STEAM);
}


unsigned char get_time_parity(long long current_time)
{
    // Use a mask of (0000 ... 0001) to extract the first bit, granting parity.
    return current_time & 1;
}

bool get_updated_flag(unsigned char tile)
{
    // The updated flag is the last bit of a tile.
    return tile >> 7;
}



