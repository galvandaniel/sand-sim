/**
 * Tiles are encoding as unsigned bytes in the following bit layout: uccUtttt
 * 
 * u - Update flag. 1 If the tile has already been updated in the current 
 * simulation step, 0 otherwise.
 * c - Color code. A value from 0 to 3 representing a unique color variation
 * on the tile's color as determined by tile type.
 * U - (UNUSED)
 * t - Tile type identifier, a value from 0 to 15.
 */

#include "sandbox.h"
#include "utils.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

/**
 * Various bit fields associated with getting/removing Tile bit fields, along
 * with their distances from least significant bit (called `shift`).
 */
constexpr uint8_t GET_UPDATE_MASK = 0b1000'0000;
constexpr uint8_t REMOVE_UPDATE_MASK = 0b0111'1111;
constexpr int UPDATE_FLAG_SHIFT = 7;

constexpr uint8_t GET_TILE_MASK = 0b0000'1111;

constexpr uint8_t REMOVE_COLOR_MASK = 0b1001'1111;
constexpr uint8_t GET_COLOR_MASK = 0b0110'0000;
constexpr int COLOR_SHIFT = 5;


// ----- STATIC FUNCTIONS -----


/**
 * From given 2D sandbox coordinates, compute a flattened integer index for
 * accessing the underlying tile grid of `sandbox`.
 * 
 * @param sandbox Sandbox to determine flattened index for.
 * @param coords (row, col) coordinates to flatten with respect to `sandbox`.
 * @return Flattened integer index for accessing tile grid of `sandbox`.
 *  */
static int get_flat_idx(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    return (coords.row * sandbox->width) + coords.col;
}


/**
 * From a flat index into the underlying grid of `sandbox`, compute 2D sandbox
 * coordinates within the bounds of the given sandbox `width` and `height`.
 * 
 * @param sandbox Sandbox to determine packed (row, col) coordinates for.
 * @param flat_idx Index into underlying grod of `sandbox`.
 * @return (row, col) packed coordinates into `sandbox`.
 */
static struct SandboxPoint get_packed_idx(struct Sandbox *sandbox, int flat_idx)
{
    // Flattened index is always a multiple of width + some remainder (col).
    int col = flat_idx % sandbox->width;
    int row = flat_idx / sandbox->width;
    return (struct SandboxPoint) {row, col};
}


/**
 * Return a pointer to the tile particle in the sandbox located at the given
 * packed (row, col) sandbox coordinates.
 * 
 * If the requested coordinates exist outside the bounds of the passed sandbox,
 * this function asserts.
 * 
 * @param sandbox Sandbox from which the returned tile is obtained.
 * @param coords (row, col) coordinates of tile to read from sandbox.
 * @return Pointer to tile particle from `sandbox` located at `coords`.
 */
static Tile *get_tile_ref(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    assert(!is_coord_oob(sandbox, coords) 
        && "ERROR: Attempt to access sandbox tile OOB!\n");

    int flat_idx = get_flat_idx(sandbox, coords);
    return sandbox->grid + flat_idx;
}


/**
 * Swap the tiles located at the two coordinates within sandbox grid.
 *
 * @param sandbox Sandbox to mutate by swapping tiles located and given coords.
 * @param coords Coordinates of first tile.
 * @param other_coords Coordinates of second tile.
 */
static void swap_tiles(struct Sandbox *sandbox, struct SandboxPoint coords, struct SandboxPoint other_coords)
{
    Tile temp = get_tile(sandbox, coords);
    *get_tile_ref(sandbox, coords) = *get_tile_ref(sandbox, other_coords);
    *get_tile_ref(sandbox, other_coords) = temp;
}


/**
 * Determine whether the two given tiles have the type tile type.
 * 
 * @param tile, other_tile Tiles to determine if they have same type.
 * @return True if tiles have same tile type, false otherwise.
 */
static bool are_tiles_same_type(Tile tile, Tile other_tile)
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
static double get_tile_survival_chance(Tile tile)
{
    enum tile_type current_type = get_tile_type(tile);

    // Tile values that are not 0 or 1 have been chosen empirically based on
    // what 'feels' right.
    switch (current_type)
    {
    // AIR represents empty tile so 'surviving' has no significance for it.
    case AIR:
    case SAND:
    case WATER:
    case WOOD:
    case FUEL:
        return 1.0;

    case STEAM:
        return 0.95;

    case FIRE:
        return 0.87;

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
static double tile_flammability(Tile tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
    case AIR:
    case SAND:
    case WATER:
    case STEAM:
    case FIRE:
        return 0.0;

    case WOOD:
        return 0.50;         

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
static bool tile_has_gravity(Tile tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
    case AIR:
    case WOOD:
    case STEAM:
    case FIRE:
        return false;

    case SAND:
    case WATER:
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
 * For example, you cannot stand on air and fire.
 * You can, however, stand on sand and wood.
 *
 * @param tile Tile to determine if is solid or not.
 * @return True if tile is solid, false otherwise.
 */
static bool tile_is_solid(Tile tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
    case AIR:
    case WATER:
    case STEAM:
    case FIRE:
    case FUEL:
        return false;

    case SAND:
    case WOOD:
        return true;

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
static bool tile_is_liquid(Tile tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
    case AIR:
    case SAND:
    case WOOD:
    case STEAM:
    case FIRE:
        return false;

    case WATER:
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
static bool tile_is_gas(Tile tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
    // Air refers to the empty tile, it is not a gas.
    case AIR:
    case SAND:
    case WATER:
    case WOOD:
    // Fuel is a gasoline ('gas'), but it is not a gaseous substance.
    case FUEL:
        return false;

    case STEAM:
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
 * @return True if tile dissolves in liquids like water, false otherwise.
 */
static bool tile_dissolves(Tile tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
    case AIR:
    case SAND:
    case WATER:
    case WOOD:
    case STEAM:
    case FIRE:
    case FUEL:
    case NUM_TILE_TYPES:
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
static bool tile_is_incendiary(Tile tile)
{
    enum tile_type current_type = get_tile_type(tile);

    switch (current_type)
    {
    case AIR:
    case SAND:
    case WATER:
    case WOOD:
    case STEAM:
    case FUEL:
        return false;

    case FIRE:
        return true;

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
static bool roll_should_tile_survive(Tile tile)
{
    double chance_of_survival = get_tile_survival_chance(tile);

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
static bool roll_should_tile_burn(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    Tile tile = get_tile(sandbox, coords);
    double burn_chance = tile_flammability(tile);

    if (approx_equal(burn_chance, 0.0))
    {
        return false;
    }

    struct SandboxPoint top = {coords.row - 1, coords.col};
    struct SandboxPoint right = {coords.row, coords.col + 1};
    struct SandboxPoint bottom = {coords.row + 1, coords.col};
    struct SandboxPoint left = {coords.row, coords.col - 1};

    // Flammable tiles roll for burn if an incendiary tile is directly NSEW.
    struct SandboxPoint search_area[4] = {
        top,
        right,
        bottom,
        left,
    };

    // Search for incendiary tile. 
    // If search area goes fully OOB, default to not incendiary.
    bool is_next_to_incendiary = false;
    for (int i = 0; i < 4; i++)
    {
        if (is_coord_oob(sandbox, search_area[i]))
         {
            continue;
         }

         if (tile_is_incendiary(get_tile(sandbox, search_area[i])))
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
static bool can_flow(struct Sandbox *sandbox, struct SandboxPoint source, struct SandboxPoint target)
{
    if (is_coord_oob(sandbox, target))
    {
        return false;
    }

    Tile source_tile = get_tile(sandbox, source);
    Tile target_tile = get_tile(sandbox, target);
    
    return (is_tile_empty(target_tile) ||
           (tile_is_liquid(target_tile) && !are_tiles_same_type(source_tile, target_tile)));
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
static bool can_lift(struct Sandbox *sandbox, struct SandboxPoint source, struct SandboxPoint target)
{
    // If attempting to lift OOB, reject.
    if (is_coord_oob(sandbox, target))
    {
        return false;
    }

    Tile source_tile = get_tile(sandbox, source);
    Tile target_tile = get_tile(sandbox, target);

    // A tile can only lift through liquid by going upwards, not sideways.
    bool is_left_or_right = (target.col == source.col - 1) || (target.col == source.col + 1);
    bool is_parallel_horizontal = target.row == source.row && is_left_or_right;
    if (tile_is_liquid(target_tile) && is_parallel_horizontal)
    {
        return false;
    }

    // Gases lift through liquids and other gases, but only if not passing
    // through own gas type.
    return (is_tile_empty(target_tile)
         || tile_is_liquid(target_tile)
         || (tile_is_gas(target_tile) && !are_tiles_same_type(source_tile, target_tile)));
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
static bool can_sink(struct Sandbox *sandbox, struct SandboxPoint source, struct SandboxPoint target)
{
    // Cannot sink if doing so goes OOB.
    if (is_coord_oob(sandbox, target))
    {
        return false;
    }

    Tile source_tile = get_tile(sandbox, source);
    Tile target_tile = get_tile(sandbox, target);

    // Can only sink through a liquid tile, and a liquid cannot sink through 
    // its own type.
    return (tile_is_liquid(target_tile)
         && !are_tiles_same_type(source_tile, target_tile)
         && !tile_dissolves(source_tile));
}


// ----- PUBLIC FUNCTIONS -----


struct Sandbox *create_sandbox(int width, int height)
{
    // Lifetime sandbox has existed for begins at 0 frames, 0 seconds.
    struct Sandbox *new_sandbox = SAFE_MALLOC(sizeof(*new_sandbox));
    new_sandbox->width = width;
    new_sandbox->height = height;
    new_sandbox->lifetime = 0;

    // Allocate flattened 2D grid of tile particles, setting each tile to AIR.
    Tile *new_grid = SAFE_CALLOC((size_t) (height * width), sizeof(*new_grid));
    new_sandbox->grid = new_grid;

    return new_sandbox;
}


void sandbox_free(struct Sandbox *sandbox)
{
    free(sandbox->grid);
    free(sandbox);
}


void process_sandbox(struct Sandbox *sandbox)
{
    int sandbox_area = sandbox->width * sandbox->height;

    // Iterate through whole sandbox, applying updates where necessary.
    for (int i = 0; i < sandbox_area; ++i)
    {
        // Any mutation made to the current tile stops all other updates.
        // For 1 simulation step, a tile may only move 1 space xor convert 
        // into another tile exactly once
        struct SandboxPoint coords = get_packed_idx(sandbox, i);
        Tile current_tile = get_tile(sandbox, coords);

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
        if (!roll_should_tile_survive(current_tile))
        {
            delete_tile(sandbox, coords);
            continue;
        }

        // Perform burn check.
        if (roll_should_tile_burn(sandbox, coords))
        {
            replace_tile(sandbox, coords, FIRE);
            continue;
        }

        // Mark the tile as updated before checking for any movement.
        // Take care to mutate the array element, NOT the stack-variable.
        set_tile_updated(get_tile_ref(sandbox, coords), sandbox->lifetime);

        // Perform extinguish check. Only fire extinguishes. 
        if (get_tile_type(current_tile) == FIRE && do_extinguish(sandbox, coords))
        {
            continue;
        }

        // Perform gravity on the tiles that need it.
        if (tile_has_gravity(current_tile) && do_gravity(sandbox, coords))
        {
            continue;
        }

        // Perform flow on liquid tiles.
        if (tile_is_liquid(current_tile) && do_flow(sandbox,coords))
        {
            continue;
        }

        // Perform lift on gasses
        if (tile_is_gas(current_tile) && do_lift(sandbox, coords))
        {
            continue;
        }
    }

    // For every frame of processing, the sandbox grows older.
    ++sandbox->lifetime;
}


bool do_gravity(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    struct SandboxPoint bottom = {coords.row + 1, coords.col};

    // Don't simulate gravity if doing so would take us out of bounds.
    if (is_coord_oob(sandbox, bottom))
    {
        return false;
    }

    // First check if tile can sink through a liquid/fall directly bottom.
    bool can_sink_below = can_sink(sandbox, coords, bottom);

    if (is_tile_empty(get_tile(sandbox, bottom)) || can_sink_below)
    {
        swap_tiles(sandbox, coords, bottom);
        return true;
    }

    struct SandboxPoint left = {coords.row, coords.col - 1};
    struct SandboxPoint right = {coords.row, coords.col + 1};
    struct SandboxPoint bottomleft = {bottom.row, left.col};
    struct SandboxPoint bottomright = {bottom.row, right.col};

    // The left and right borders of the sandbox are considered walls.
    bool is_wall_left = (left.col == -1 
                      || tile_is_solid(get_tile(sandbox, left)));
    bool is_wall_right = (right.col == sandbox->width 
                       || tile_is_solid(get_tile(sandbox, right)));

    // Cannot slide or sink at all if there are walls both sides and below is
    // not empty or liquid.
    if (is_wall_left && is_wall_right)
    {
        return false;
    }

    // Now check if tile can sink/slide diagonally instead.
    bool can_sink_bottomleft = can_sink(sandbox, coords, bottomleft);
    bool can_sink_bottomright = can_sink(sandbox, coords, bottomright);

    bool can_slide_bottomleft = (!is_wall_left && is_tile_empty(get_tile(sandbox, bottomleft)));
    bool can_slide_bottomright = (!is_wall_right && is_tile_empty(get_tile(sandbox, bottomright)));

    // If we can both slide/sink bottom left and right, choose one at random.
    struct SandboxPoint target = {.row = coords.row, .col = coords.col};

    if ((can_slide_bottomleft && can_slide_bottomright)
        || (can_sink_bottomleft && can_sink_bottomright))
    {
        target = flip_coin() ? bottomleft : bottomright;
        swap_tiles(sandbox, coords, target);
        return true;
    }

    // If there is no choice in direction, do whichever is possible.
    target = (can_slide_bottomleft || can_sink_bottomleft) ? bottomleft : target;
    target = (can_slide_bottomright || can_sink_bottomright) ? bottomright : target;

    if (target.row != coords.row || target.col != coords.col)
    {
        swap_tiles(sandbox, coords, target);
        return true;
    }

    return false;
}


bool do_flow(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    struct SandboxPoint bottom = {coords.row + 1, coords.col};

    // TODO: Move this logic into can_flow.
    // Liquid can't flow if not on solid footing or not ontop of another liquid.
    // Being on the botton of sandbox counts as being on solid footing.
    bool on_solid_ground = bottom.row == sandbox->height || tile_is_solid(get_tile(sandbox, bottom));
    if (!on_solid_ground && !tile_is_liquid(get_tile(sandbox, bottom)))
    {
        return false;
    }

    struct SandboxPoint left = {coords.row, coords.col - 1};
    struct SandboxPoint right = {coords.row, coords.col + 1};

    bool can_flow_left = can_flow(sandbox, coords, left);
    bool can_flow_right = can_flow(sandbox, coords, right);

    // If we can flow both directions, choose one at random on a coin flip.
    struct SandboxPoint target = {.row = coords.row, .col = coords.col};
    if (can_flow_left && can_flow_right)
    {
        target = flip_coin() ? left : right;
        swap_tiles(sandbox, coords, target);
        return true;
    }

    // If only one option is available, do that.
    target = can_flow_left ? left : target;
    target = can_flow_right ? right : target;

    if (target.col != coords.col)
    {
        swap_tiles(sandbox, coords, target);
        return true;
    }

    return false;
}


bool do_lift(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    struct SandboxPoint left = {coords.row, coords.col - 1};
    struct SandboxPoint right = {coords.row, coords.col + 1};
    struct SandboxPoint top = {coords.row - 1, coords.col};
    struct SandboxPoint topleft = {top.row, left.col};
    struct SandboxPoint topright = {top.row, right.col};

    // Capture possible coordinates to lift to and determine whether tile
    // at each coordinates can be lifted to any of them.
    struct SandboxPoint options[] = {topleft, top, topright, left, right};
    const size_t num_options = sizeof(options) / sizeof(options[0]);
    bool possibilities[num_options];

    for (size_t i = 0; i < num_options; i++)
    {
        possibilities[i] = can_lift(sandbox, coords, options[i]);
    }

    // Collect the valid movement options into an array then pick one at random.
    struct SandboxPoint targets[num_options];
    int num_true = 0;
    for (size_t i = 0; i < num_options; i++)
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

    int rand_idx = randint(0, num_true - 1);
    swap_tiles(sandbox, coords, targets[rand_idx]);
    return true;
}


bool do_extinguish(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    struct SandboxPoint top = {coords.row - 1, coords.col};
    struct SandboxPoint left = {coords.row, coords.col - 1};
    struct SandboxPoint right = {coords.row, coords.col + 1};
    struct SandboxPoint bottom = {coords.row + 1, coords.col};

    // Check for water in cardinal directions.
    struct SandboxPoint search_area[4] = {top, right, bottom, left};
    bool is_next_to_water = false;
    for (int i = 0; i < 4; i++)
    {
        // If search area goes fully OOB, default to not next to water water.
        if (is_coord_oob(sandbox, search_area[i]))
         {
            continue;
         }

         if (get_tile_type(get_tile(sandbox, search_area[i])) == WATER)
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


Tile get_tile(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    // Idea: Consider notion of returning 'INVALID' tile particle instead?
    assert(!is_coord_oob(sandbox, coords) 
        && "ERROR: Attempt to access sandbox tile OOB!\n");

    int flattened_idx = get_flat_idx(sandbox, coords);
    return sandbox->grid[flattened_idx];
}



Tile create_tile(struct Sandbox *sandbox, enum tile_type new_type)
{
    // New tiles are synced to time to prevent update until next frame.
    Tile new_tile = (Tile) new_type;
    set_tile_updated(&new_tile, sandbox->lifetime);

    // Give newly created tile a random color variation code within [0, 3].
    uint8_t color_variant = (uint8_t) randint(0, 3);
    set_tile_color(&new_tile, color_variant);
    return new_tile;
}


void place_tile(struct Sandbox *sandbox, struct SandboxPoint coords, enum tile_type type)
{
    // Only place new tiles ontop of air.
    if (!is_tile_empty(get_tile(sandbox, coords)))
    {
        return;
    }
    *get_tile_ref(sandbox, coords) = create_tile(sandbox, type);
}


void delete_tile(struct Sandbox *sandbox, struct SandboxPoint coords)
{
    // Don't delete air tile, this would be redundant.
    if (is_tile_empty(get_tile(sandbox, coords)))
    {
        return;
    }
    *get_tile_ref(sandbox, coords) = AIR;
}


void replace_tile(struct Sandbox *sandbox, struct SandboxPoint coords, enum tile_type type)
{
    // Don't replace a tile with its own type, this would be redundant.
    enum tile_type source_type = get_tile_type(get_tile(sandbox, coords));
    if (source_type == type)
    {
        return;
    }
    *get_tile_ref(sandbox, coords) = create_tile(sandbox, type);
}


enum tile_type get_tile_type(Tile tile)
{
    return GET_TILE_MASK & tile;
}


uint8_t get_tile_color(Tile tile)
{
    uint8_t color_code = GET_COLOR_MASK & tile;
    return color_code >> COLOR_SHIFT;
}


bool get_updated_flag(Tile tile)
{
    // The updated flag is the last bit of a tile.
    return (bool) (tile >> UPDATE_FLAG_SHIFT);
}


bool is_tile_empty(Tile tile)
{
    return get_tile_type(tile) == AIR;
}


bool is_tile_updated(Tile tile, uint64_t current_time)
{
    uint8_t time_parity = get_time_parity(current_time);
    bool updated_flag = get_updated_flag(tile);
    return updated_flag == (bool) time_parity;
}


void set_tile_updated(Tile *tile, uint64_t current_time)
{
    uint8_t time_parity = get_time_parity(current_time);

    // In the case of 0, we're updating a tile bit flag of 1 to 0, so we AND.
    // In the case of 1, we're updating a tile bit flag of 0 to 1, so we OR.
    if (time_parity == 0)
    {
        *tile &= REMOVE_UPDATE_MASK;
    }
    else
    {
        *tile |= GET_UPDATE_MASK;
    }
}


void set_tile_color(Tile *tile, uint8_t color)
{
    // Bring 2-bit color value into format (0cc0 0000) (chopping off int bits), 
    // erase old 2-bit color value using mask of (1001 1111) and copy new one.
    uint8_t new_GET_COLOR_MASK = (uint8_t) (color << COLOR_SHIFT);

    *tile &= REMOVE_COLOR_MASK;
    *tile |= new_GET_COLOR_MASK;
}


uint8_t get_time_parity(uint64_t current_time)
{
    // Use a mask of (0000 ... 0001) to extract the first bit, granting parity.
    return current_time & 1;
}

