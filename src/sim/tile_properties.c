/**
 * Implementation of interface defined by tile_properties.h
 * 
 * If the properties of any tile type must be modified, or properties for a new
 * tile type is to be defined, it must be done here.
 */

#include "sim/tile_properties.h"
#include "sim/tile.h"

// ----- Define behavioral properties of all tile types -----

/**
 * Air refers to the empty tile, not oxygen, so it is not a gas.
 */
constexpr struct TileProperties AIR_PROPERTIES = 
{
    .is_subject_to_gravity = false,
    .is_solid = false,
    .is_liquid = false,
    .is_gas = false,
    .is_incendiary = false,
    .dissolves = false,
    .burn_chance = 0.0,
    // Air represents empty tile so 'surviving' has no significance for it.
    .survival_chance = 1.0,  
};

/**
 * Sand falls and piles up in liquids.
 */
constexpr struct TileProperties SAND_PROPERTIES =
{
    .is_subject_to_gravity = true,
    .is_solid = true,
    .is_liquid = false,
    .is_gas = false,
    .is_incendiary = false,
    .dissolves = false,
    .burn_chance = 0.0,
    .survival_chance = 1.0,
};

/**
 * Water behaves like sand, except it flows ontop of solids.
 */
constexpr struct TileProperties WATER_PROPERTIES =
{
    .is_subject_to_gravity = true,
    .is_solid = false,
    .is_liquid = true,
    .is_gas = false,
    .is_incendiary = false,
    .dissolves = false,
    .burn_chance = 0.0,
    .survival_chance = 1.0,
};

/**
 * Wood behaves like solid floating ground, except it burns.
 */
constexpr struct TileProperties WOOD_PROPERTIES =
{
    .is_subject_to_gravity = false,
    .is_solid = true,
    .is_liquid = false,
    .is_gas = false,
    .is_incendiary = false,
    .dissolves = false,
    .burn_chance = 0.50,
    .survival_chance = 1.0,
};


/**
 * Steam rises, then disappears.
 */
constexpr struct TileProperties STEAM_PROPERTIES =
{
    .is_subject_to_gravity = false,
    .is_solid = false,
    .is_liquid = false,
    .is_gas = true,
    .is_incendiary = false,
    .dissolves = false,
    .burn_chance = 0.0,
    .survival_chance = 0.95,
};

/**
 * Fire rises, then disappears, but more quickly than steam.
 */
constexpr struct TileProperties FIRE_PROPERTIES =
{
    .is_subject_to_gravity = false,
    .is_solid = false,
    .is_liquid = false,
    // Fuel is basically gasoline ('gas'), but it is not a gaseous substance.
    .is_gas = true,
    .is_incendiary = true,
    .dissolves = false,
    .burn_chance = 0.0,
    .survival_chance = 0.87,
};

/**
 * Fuel behaves like water, except it burns very quickly.
 */
constexpr struct TileProperties FUEL_PROPERTIES =
{
    .is_subject_to_gravity = true,
    .is_solid = false,
    .is_liquid = true,
    .is_gas = false,
    .is_incendiary = false,
    .dissolves = false,
    .burn_chance = 0.75,
    .survival_chance = 1.0, 
};

/**
 * Lookup table of all properties of any given tile type.
 */
const struct TileProperties TILE_PROPERTIES[NUM_TILE_TYPES] =
{
    [AIR] = AIR_PROPERTIES,
    [SAND] = SAND_PROPERTIES,
    [WATER] = WATER_PROPERTIES,
    [WOOD] = WOOD_PROPERTIES,
    [STEAM] = STEAM_PROPERTIES,
    [FIRE] = FIRE_PROPERTIES,
    [FUEL] = FUEL_PROPERTIES,
};


// ----- Utility functions -----


bool tile_is_subject_to_gravity(Tile tile)
{
    return TILE_PROPERTIES[get_tile_type(tile)].is_subject_to_gravity;
}

bool tile_is_solid(Tile tile)
{
    return TILE_PROPERTIES[get_tile_type(tile)].is_solid;
}

bool tile_is_liquid(Tile tile)
{
    return TILE_PROPERTIES[get_tile_type(tile)].is_liquid;
}

bool tile_is_gas(Tile tile)
{
    return TILE_PROPERTIES[get_tile_type(tile)].is_gas;
}

bool tile_is_incendiary(Tile tile)
{
    return TILE_PROPERTIES[get_tile_type(tile)].is_incendiary;
}

bool tile_dissolves(Tile tile)
{
    return TILE_PROPERTIES[get_tile_type(tile)].dissolves;
}

double get_tile_burn_chance(Tile tile)
{
    return TILE_PROPERTIES[get_tile_type(tile)].burn_chance;
}

double get_tile_survival_chance(Tile tile)
{
    return TILE_PROPERTIES[get_tile_type(tile)].survival_chance;
}