/**
 * Module which defines the interface for creating and manipulating
 * `Tile` particles in a sandbox simulation.
 * 
 * Each tile shall be represented as a single byte.
 */

#ifndef TILE_H
#define TILE_H

#include <stdint.h>

/**
 * One tile particle of a sandbox is represented by exactly 1 byte.
 */
typedef uint8_t Tile;

/*
 * All types a tile particle can be. AIR denotes the empty tile.
 */
enum tile_type {
    AIR, 
    SAND, 
    WATER, 
    WOOD, 
    STEAM, 
    FIRE,
    FUEL,
    NUM_TILE_TYPES,
};


/**
 * Return the type of a tile, describing its properties in simulation.
 *
 * @param tile Tile to fetch type of.
 * @return Value from 0 to 15 representing the type of tile given.
 */
enum tile_type get_tile_type(Tile tile);


/**
 * Obtain the updated flag from a tile, synced to the parity of the time from
 * when it was last updated.
 *
 * The updated flag on its own does NOT say whether the tile is currently
 * updated or not. The flag represents a parity, it is NOT a boolean.
 *
 * Use is_tile_updated() to determine whether a tile is updated or not.
 *
 * @param tile Tile to get updated flag from.
 * @return True if updated flag is set, false otherwise.
 */
bool get_updated_flag(Tile tile);


/**
 * Return the color code of the given tile, a value from 0 to 3 encoding a
 * color variation of the tile type color.
 * 
 * @param tile Tile to get color code from.
 * @return A value from 0 to 3, each representing a unique color variation.
 */
unsigned char get_tile_color(Tile tile);



/**
 * Return whether the given tile is an empty space or not.
 * 
 * @param tile Tile to determine if is empty and replaceable or not.
 * @return True if tile is empty, false otherwise.
 */
bool is_tile_empty(Tile tile);


/**
 * Determine whether the given tile has already been updated or not with
 * respect to the passed `current_time`.
 *
 * When a tile is updated, its updated flag is changed to match the parity of 
 * the time that has passed since the simulation began.
 *
 * @param tile Tile to determine whether it has been updated or not.
 * @param current_time Time that has passed in frames inside the simulation.
 * @return True if the tile has already been updated, false otherwise.
 */
bool is_tile_updated(Tile tile, uint64_t current_time);


/**
 * Mutate the given tile's updated flag to show the tile has been updated.
 * This syncs the tile's flag to match the parity of the current simulation 
 * time.
 *
 * @param tile Tile whose updated flag will be set.
 * @param current_time Time that has passed in frames inside the simulation.
 */
void set_tile_updated(Tile *tile, uint64_t current_time);


/**
 * Mutate the given tile's color code value to the given color code.
 * 
 * @param tile Tile whose color code will be mutated.
 * @param color Value from 0 to 3 encoding a color variation on the tile which
 * will be set on the tile given.
 */
void set_tile_color(Tile *tile, uint8_t color);

#endif // TILE_H