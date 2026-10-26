/**
 * Module which defines behavioral properties common to all tiles of any type,
 * and the interface for obtaining these properties from tiles.
 */

#ifndef TILE_PROPERTIES_H
#define TILE_PROPERTIES_H

#include "sim/tile.h"

/**
 * Container for all properties which determine the behavior of a tile for
 * any particular type in a sandbox simulation.
 * 
 * Floating point properties represent probabilities, where values other than
 * 0 or 1 have been chosen empirically based on what `feels` right.
 */
struct TileProperties
{
    bool is_subject_to_gravity;
    bool is_solid;
    bool is_liquid;
    bool is_gas;
    bool is_incendiary;
    bool dissolves;

    double burn_chance;
    double survival_chance;
};


/**
 * Lookup table of all properties of any given tile type.
 */
extern const struct TileProperties TILE_PROPERTIES[NUM_TILE_TYPES];


// ----- Utility functions for getting individual properties from tiles. -----


/**
 * Return whether the given tile is affected by gravity or not.
 *
 * @param tile Tile to determine if it has gravity or not.
 * @return True if tile is affected by gravity, false otherwise.
 */
bool tile_is_subject_to_gravity(Tile tile);

/**
 * Return whether or not a tile is "solid".
 * 
 * A solid blocks fluids from moving.
 *
 * @param tile Tile to determine if is solid or not.
 * @return True if tile is solid, false otherwise.
 */
bool tile_is_solid(Tile tile);

/**
 * Return whether the given tile is "liquid". 
 * 
 * A liquid flows ontop of solids and other liquids.
 *
 * @param tile Tile to determine if is liquid or not.
 * @return True if the tile type is liquid and there has flow, false otherwise.
 */
bool tile_is_liquid(Tile tile);

/**
 * Return whether the given tile is a "gas" 
 * 
 * A gas experiences lift and is able to pass through liquids and other gasses.
 *
 * @param tile Tile to determine if is gas and has lift or not.
 * @return True if tile is a gas and lifts into the air, false otherwise.
 */
bool tile_is_gas(Tile tile);

/**
 * Return whether tile is "incendiary" 
 * 
 * An incendiary lights burnable tiles on fire by being close to them.
 * 
 * @param tile Tile to determine if can light other tiles on fire or not.
 * @return True if tile can light others on fire, false otherwise.
 */
bool tile_is_incendiary(Tile tile);

/**
 * Return whether a tile flows laterally though liquids.
 * 
 * @param tile Tile to determine if dissolves in liquid.
 * @return True if tile dissolves in liquids like water, false otherwise.
 */
bool tile_dissolves(Tile tile);

/**
 * Get the flammability of a tile, the chance a tile has of being lit on fire
 * when adjacent to any incendiary tile in the cardinal directions.
 * 
 * @param tile Tile to get flammability chance for.
 * @return Chance of being lit on fire, 0.0 indicating 0%, 1.0 indicating 100%.
 */
double get_tile_burn_chance(Tile tile);

/**
 * Get chance a tile has of surviving to the next frame of simulation.
 * If a tile does not survive, it is replaced by AIR.
 * 
 * 0.0 indicates 0% chance of survival, 1.0 indicates 100% chance. 
 * A tile with 100% chance of survival will never disappear unless removed.
 * 
 * @param tile Tile to get likelihood of survival.
 * @return Double representing chance of survival to the next frame.
 */
double get_tile_survival_chance(Tile tile);

#endif // TILE_PROPERTIES_H