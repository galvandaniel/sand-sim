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
 * 6 - Fuel
 * 6-15 - (UNUSED)
 *
 */

#include "sandbox.h"
#include "utils.h"

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


// ----- STATIC FUNCTIONS -----


/**
 * Swap the tiles located at the two coordinates within sandbox grid.
 *
 * @param coords Coordinates of first tile.
 * @param other_coords Coordinates of second tile.
 * @param grid Sandbox 2D grid of bytes to mutate by swapping first and second 
 * tile.
 */
static void _swap_tiles(struct SandboxPoint coords, struct SandboxPoint other_coords, unsigned char **grid)
{
    unsigned char temp = grid[coords.row][coords.col];
    grid[coords.row][coords.col] = grid[other_coords.row][other_coords.col];
    grid[other_coords.row][other_coords.col] = temp;
}


/**
 * Determine whether the two given tiles have the type tile type.
 * 
 * @param tile, other_tile Tiles to determine if they have same type.
 * @return True if tiles have same tile type, false otherwise.
 */
static bool _are_tiles_same_type(unsigned char tile, unsigned char other_tile)
{
    return get_tile_type(tile) == get_tile_type(other_tile);
}


/**
 * Get the chance a tile particle has of surviving to the next frame of
 * simulation.
 * 
 * If a tile does not survive, it is replaced by AIR.
 * 
 * 0.0 indicates 0% chance of survival, 1.0 indicates 100% chance. 
 * A tile with 100% chance of survival will never disappear unless removed by
 * the user.
 * 
 * @param tile Tile to get likelihood of survival.
 * @return Double representing chance of survival to the next frame.
 */
static double _tile_survival_chance(unsigned char tile)
{
    enum tile_type current_type = get_tile_type(tile);

    // Tile values that are not 0 or 1 have been chosen empirically based on
    // what 'feels' right.
    switch (current_type)
    {
        // AIR represents empty tile so 'surviving' has no significance for it.
        case AIR:
            return 1.0;

        case SAND:
            return 1.0;

        case WATER:
            return 1.0;

        case WOOD:
            return 1.0;

        case STEAM:
            return 0.95;

        case FIRE:
            return 0.87;
        
        case FUEL:
            return 1.0;

        default:
            return 1.0;
    }
}


/**
 * Get the flammability of a tile, the chance a tile has of being lit on fire in
 * next frame when adjacent to any incendiary tile in the cardinal directions.
 * 
 * @param tile Tile to get flammability chance for.
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
            return 0.50;

        case STEAM:
            return 0.0;

        case FIRE:
            return 0.0;            

        case FUEL:
            return 0.75;

        default:
            return 0.0;
    }
}


/**
 * Return whether the given tile is affected by gravity or not.
 *
 * @param tile Tile to determine if it has gravity or not.
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
        
        case FUEL:
            return true;

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
        
        case FUEL:
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
        
        case FUEL:
            return true;

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
 * @param tile Tile to determine if is gas and has lift or not.
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
        
        // Fuel is gasoline ('gas'), but it is not a gaseous fluid.
        case FUEL:
            return false;

        default:
            return false;
    }
}


/**
 * Return whether or not a tile dissolves in fluids therefore has a chance to
 * flow laterally through liquids.
 * 
 * @param tile Tile to determine if dissolves in liquid.
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
        
        case FUEL:
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
        
        case FUEL:
            return false;

        default:
            return false;
    }
}


/**
 * Perform a random roll on whether the given tile should survive to the next
 * frame of simulation or not. This depends on the tile's survival odds.
 * 
 * @param tile Tile to determine if should survive to next frame or not.
 * @return True if tile survives, false if tile dies and is replaced by air.
 */
static bool _roll_should_tile_survive(unsigned char tile)
{
    double chance_of_survival = _tile_survival_chance(tile);

    // No need to check if a tile which always survives does so.
    if (approx_equal(chance_of_survival, 1.0))
    {
        return true;
    }

    double random_value = random();
    return random_value <= chance_of_survival;
}


/**
 * Perform a random roll on whether or not the tile located at the given 
 * coordinates in the sandbox should light on fire and be replaced by a fire tile.
 * 
 * If the given tile cannot burn at all, no randomness or roll is performed.
 * 
 * @param sandbox Sandbox containing tile to test for burn condition.
 * @param coords Coordinates of tile to determine if should convert to fire on 
 * next frame or not.
 * @return True if roll succeeds and tile should become fire, false otherwise.
 */
static bool _roll_should_tile_burn(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    double burn_chance = _tile_flammability(sandbox->grid[coords.row][coords.col]);

    if (approx_equal(burn_chance, 0.0))
    {
        return false;
    }

    struct SandboxPoint up = {coords.row - 1, coords.col};
    struct SandboxPoint right = {coords.row, coords.col + 1};
    struct SandboxPoint down = {coords.row + 1, coords.col};
    struct SandboxPoint left = {coords.row, coords.col - 1};
    // Flammable tiles roll for burn if an incendiary tile is directly NSEW.
    struct SandboxPoint search_area[4] = {up,
                                          right,
                                          down,
                                          left};
    // Search for incendiary tile. 
    // If search area goes OOB, default to not incendiary.
    bool is_next_to_incendiary = false;
    for (int i = 0; i < 4; i++)
    {
        int search_row = search_area[i].row;
        int search_col = search_area[i].col;

        if (is_coord_oob(sandbox, search_area[i]))
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
    double random_value = random();
    return random_value <= burn_chance;
}


/**
 * Determine if the given sandbox coordinates can flow like liquid to the
 * target coordinates
 *
 * Liquid can flow to some target location if the tile at that location is empty
 * or another liquid not of the same tile type.
 *
 * @param sandbox Sandbox to determine if target coordinates can flow to.
 * @param coords Source coordinates of tile to perform flow.
 * @param target Destination coordinates tile is trying to flow to.
 * @return True if the tile at coords can flow to target, false otherwise.
 */
static bool _can_flow(struct Sandbox *sandbox, struct SandboxPoint source, struct SandboxPoint target)
{
    if (is_coord_oob(sandbox, target))
    {
        return false;
    }

    unsigned char source_tile = sandbox->grid[source.row][source.col];
    unsigned char target_tile = sandbox->grid[target.row][target.col];
    
    return (is_tile_empty(target_tile) ||
           (_tile_is_liquid(target_tile) && !_are_tiles_same_type(source_tile, target_tile)));
}


/**
 * Determine if the given sandbox coordinates can be lifted to the target coordinates.
 * 
 * It is assumed that the target coordinates are valid coordinates which can be
 * lifted, coordinates are not checked for consistency with anti-gravity logic.
 *
 * @param sandbox Sandbox to determine if target coordinates can be lifted to.
 * @param source Source coordinates of tile to perform lift.
 * @param target Destination coordinates a tile is trying to lift to.
 * @return True if the source tile can be lifted to the target location, 
 * False otherwise.
 */
static bool _can_lift(struct Sandbox *sandbox, struct SandboxPoint source, struct SandboxPoint target)
{
    // If attempting to lift OOB, reject.
    if (is_coord_oob(sandbox, target))
    {
        return false;
    }

    unsigned char source_tile = sandbox->grid[source.row][source.col];
    unsigned char target_tile = sandbox->grid[target.row][target.col];

    // A tile can only lift through liquid by going upwards, not sideways.
    bool is_left_or_right = (target.col == source.col - 1) || (target.col == source.col + 1);
    bool is_parallel_horizontal = target.row == source.row && is_left_or_right;
    if (_tile_is_liquid(target_tile) && is_parallel_horizontal)
    {
        return false;
    }

    // Gases lift through liquids and other gases, but only if not passes
    // through own gas type.
    return (is_tile_empty(target_tile)
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
 * @param source Source coordinates of tile to perform sink.
 * @param target Destination coordinates a tile is trying to sink to.
 * @return True if the source tile can sink to the target location
 * False otherwise.
 */
static bool _can_sink(struct Sandbox *sandbox, struct SandboxPoint source, struct SandboxPoint target)
{
    // Cannot sink if doing so goes OOB.
    if (is_coord_oob(sandbox, target))
    {
        return false;
    }

    unsigned char source_tile = sandbox->grid[source.row][source.col];
    unsigned char target_tile = sandbox->grid[target.row][target.col];

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
    struct Sandbox *new_sandbox = SAFE_MALLOC(sizeof(*new_sandbox));

    new_sandbox->width = width;
    new_sandbox->height = height;
    new_sandbox->lifetime = 0;

    // Allocate grid, starting by allocating memory for each row of grid.
    unsigned char **new_grid = SAFE_MALLOC((size_t) height * sizeof(*new_grid));
    
    // Then allocate memory for each tile within each row, setting each tile to
    // 0, which corresponds to air.
    for (int row = 0; row < height; row++)
    {
        new_grid[row] = SAFE_CALLOC((size_t) width, sizeof(**new_grid));
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
            // Any mutation made to the current tile stops all other updates.
            // For 1 simulation step, a tile may only move 1 space xor convert 
            // into another tile exactly once
            unsigned char current_tile = grid[row][col];
            struct SandboxPoint coords = {row, col};

            bool is_updated = is_tile_updated(current_tile, sandbox->lifetime);

            // Do not simulate an empty tile.
            if (is_tile_empty(current_tile))
            {
                continue;
            }

            // Do not simulate a tile that has already been updated.
            if (is_updated)
            {
                continue;
            }

            // Perform survival check.
            if (!_roll_should_tile_survive(current_tile))
            {
                delete_tile(sandbox, coords);
                continue;
            }

            // Perform burn check.
            if (_roll_should_tile_burn(sandbox, coords))
            {
                replace_tile(sandbox, coords, FIRE);
                continue;
            }

            // Mark the tile as updated before checking for any movement.
            // Take care to mutate the array element, NOT the stack-variable.
            set_tile_updated(&grid[row][col], sandbox->lifetime);

            // Perform extinguish check. Only fire extinguishes. 
            if (get_tile_type(current_tile) == FIRE && do_extinguish(sandbox, coords))
            {
                continue;
            }

            // Perform gravity on the tiles that need it.
            if (_tile_has_gravity(current_tile) && do_gravity(sandbox, coords))
            {
                continue;
            }

            // Perform flow on liquid tiles.
            if (_tile_is_liquid(current_tile) && do_flow(sandbox,coords))
            {
                continue;
            }

            // Perform lift on gasses
            if (_tile_is_gas(current_tile) && do_lift(sandbox, coords))
            {
                continue;
            }
        }
    }

    // For every frame of processing, the sandbox grows older.
    sandbox->lifetime++;
}


bool do_gravity(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    struct SandboxPoint down = {coords.row + 1, coords.col};

    // Don't simulate gravity if doing so would take us out of bounds.
    if (is_coord_oob(sandbox, down))
    {
        return false;
    }

    // First check if tile can sink through a liquid/fall directly down.
    bool can_sink_below = _can_sink(sandbox, coords, down);

    if (is_tile_empty(sandbox->grid[down.row][down.col]) || can_sink_below)
    {
        _swap_tiles(coords, down, sandbox->grid);
        return true;
    }

    struct SandboxPoint left = {coords.row, coords.col - 1};
    struct SandboxPoint right = {coords.row, coords.col + 1};
    struct SandboxPoint downleft = {down.row, left.col};
    struct SandboxPoint downright = {down.row, right.col};

    // The left and right borders of the sandbox are considered walls.
    bool is_wall_left = (left.col == -1 
                      || _tile_is_solid(sandbox->grid[left.row][left.col]));
    bool is_wall_right = (right.col == sandbox->width 
                       || _tile_is_solid(sandbox->grid[right.row][right.col]));

    // Cannot slide or sink at all if there are walls both sides and below is
    // not empty or liquid.
    if (is_wall_left && is_wall_right)
    {
        return false;
    }

    // Now check if tile can sink/slide diagonally instead.
    bool can_sink_downleft = _can_sink(sandbox, coords, downleft);
    bool can_sink_downright = _can_sink(sandbox, coords, downright);

    bool can_slide_downleft = (!is_wall_left 
                            && is_tile_empty(sandbox->grid[downleft.row][downleft.col]));
    bool can_slide_downright = (!is_wall_right 
                             && is_tile_empty(sandbox->grid[downright.row][downright.col]));

    // If we can both slide/sink down left and right, choose one at random.
    struct SandboxPoint target = {.row = coords.row, .col = coords.col};

    if ((can_slide_downleft && can_slide_downright)
        || (can_sink_downleft && can_sink_downright))
    {
        target = flip_coin() ? downleft : downright;
        _swap_tiles(coords, target, sandbox->grid);
        return true;
    }

    // If there is no choice in direction, do whichever is possible.
    target = (can_slide_downleft || can_sink_downleft) ? downleft : target;
    target = (can_slide_downright || can_sink_downright) ? downright : target;

    if (target.row != coords.row || target.col != coords.col)
    {
        _swap_tiles(coords, target, sandbox->grid);
        return true;
    }

    return false;
}


bool do_flow(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    struct SandboxPoint below = {coords.row + 1, coords.col};

    // Liquid can't flow if not on solid footing or not ontop of another liquid.
    // Being on the botton of sandbox counts as being on solid footing.
    bool on_solid_ground = below.row == sandbox->height || _tile_is_solid(sandbox->grid[below.row][below.col]);
    if (!on_solid_ground && !_tile_is_liquid(sandbox->grid[below.row][below.col]))
    {
        return false;
    }

    struct SandboxPoint left = {coords.row, coords.col - 1};
    struct SandboxPoint right = {coords.row, coords.col + 1};

    bool can_flow_left = _can_flow(sandbox, coords, left);
    bool can_flow_right = _can_flow(sandbox, coords, right);

    // If we can flow both directions, choose one at random on a coin flip.
    struct SandboxPoint target = {.row = coords.row, .col = coords.col};
    if (can_flow_left && can_flow_right)
    {
        target = flip_coin() ? left : right;
        _swap_tiles(coords, target, sandbox->grid);
        return true;
    }

    // If only one option is available, do that.
    target = can_flow_left ? left : target;
    target = can_flow_right ? right : target;

    if (target.col != coords.col)
    {
        _swap_tiles(coords, target, sandbox->grid);
        return true;
    }

    return false;
}


bool do_lift(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    struct SandboxPoint left = {coords.row, coords.col - 1};
    struct SandboxPoint right = {coords.row, coords.col + 1};
    struct SandboxPoint up = {coords.row - 1, coords.col};
    struct SandboxPoint upleft = {up.row, left.col};
    struct SandboxPoint upright = {up.row, right.col};

    // Capture possible coordinates to lift to and determine whether tile
    // at each coordinates can be lifted to any of them.
    struct SandboxPoint options[5] = {upleft, up, upright, left, right};
    bool possibilities[5];

    for (int i = 0; i < 5; i++)
    {
        possibilities[i] = _can_lift(sandbox, coords, options[i]);
    }

    // Collect the valid movement options into an array then pick one at random.
    struct SandboxPoint targets[5];
    int num_true = 0;
    for (int i = 0; i < 5; i++)
    {
        if (possibilities[i])
        {
            targets[num_true] = options[i];
            num_true++;
        }
    }

    // No lift is possible if cannot move any way upwards.
    if (num_true == 0)
    {
        return false;
    }

    int rand_index = randint(0, num_true - 1);
    _swap_tiles(coords, targets[rand_index], sandbox->grid);
    return true;
}


bool do_extinguish(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    struct SandboxPoint up = {coords.row - 1, coords.col};
    struct SandboxPoint left = {coords.row, coords.col - 1};
    struct SandboxPoint right = {coords.row, coords.col + 1};
    struct SandboxPoint down = {coords.row + 1, coords.col};

    // Check for water in cardinal directions.
    struct SandboxPoint search_area[4] = {up, right, down, left};
    bool is_next_to_water = false;
    for (int i = 0; i < 4; i++)
    {
        int search_row = search_area[i].row;
        int search_col = search_area[i].col;

        // If search area goes OOB, default to not water.
        if (is_coord_oob(sandbox, search_area[i]))
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
        return false;
    }

    replace_tile(sandbox, coords, STEAM);
    return true;
}


bool is_coord_oob(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    return (coords.row < 0 
         || coords.row >= sandbox->height
         || coords.col < 0
         || coords.col >= sandbox->width);
}


unsigned char create_tile(struct Sandbox *sandbox, enum tile_type new_type)
{
    // New tiles are synced to time to prevent update until next frame.
    unsigned char new_tile = (unsigned char) new_type;
    set_tile_updated(&new_tile, sandbox->lifetime);

    // TODO: Randomization of tile color will go here.

    return new_tile;
}


void place_tile(struct Sandbox *sandbox, struct SandboxPoint coords, enum tile_type type)
{
    // Only place new tiles ontop of air.
    if (!is_tile_empty(sandbox->grid[coords.row][coords.col]))
    {
        return;
    }
    sandbox->grid[coords.row][coords.col] = create_tile(sandbox, type);
}


void delete_tile(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    // Don't delete air tile, this would be redundant.
    if (is_tile_empty(sandbox->grid[coords.row][coords.col]))
    {
        return;
    }
    sandbox->grid[coords.row][coords.col] = AIR;
}


void replace_tile(struct Sandbox *sandbox, struct SandboxPoint coords, enum tile_type type)
{
    // Don't replace a tile with its own type, this would be redundant.
    enum tile_type source_type = get_tile_type(sandbox->grid[coords.row][coords.col]);
    if (source_type == type)
    {
        return;
    }
    sandbox->grid[coords.row][coords.col] = create_tile(sandbox, type);
}


enum tile_type get_tile_type(unsigned char tile)
{
    // Use a mask of (0000 1111) to extract the first 4 bits.
    unsigned char tile_mask = 0x0f;
    enum tile_type type_id = tile_mask & tile;

    return type_id;
}


bool is_tile_empty(unsigned char tile)
{
    return get_tile_type(tile) == AIR;
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



