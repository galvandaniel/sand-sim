#ifndef SANDBOX_H
#define SANDBOX_H

/**
 * A collection of functions for defining and processing a sand simulation.
 *
 * Each tile shall be represented as a single unsigned big-endian byte.
 *
 * The first 4 significant bits are reserved for tile flags.
 * The last 4 bits represent a tile ID number, from 0 to 15.
 */

 
#include <stdbool.h>


// Define the constant tile IDs 0 to 15.
enum tile_id {AIR, SAND, WATER, WOOD, STEAM, FIRE};


// Amount of time that has passed, in frames of simulation, since the sandbox
// has begun.
extern int SANDBOX_LIFETIME;


/**
 * Generate and allocate memory for an empty 2D sandbox of tiles with dimension
 * width X height.
 *
 * The sandbox begins filled with air, equivalent to 0 in value.
 *
 * @param height Vertical length of 2D sandbox.
 * @param width Horizontal length of 2D sandbox.
 *
 * @return Pointer to allocated 2D array of bytes representing a sandbox.
 */
unsigned char **create_sandbox(int height, int width);


/**
 * Free all memory taken up by the given sandbox simulation.
 *
 * @param sandbox Sandbox to free.
 * @param height Height of sandbox to free.
 */
void sandbox_free(unsigned char **sandbox, int height);


/**
 * Perform one full iteration of simulation on the given sandbox, applying 
 * any tile interactions, flow, gravity, flamability, etc.
 *
 * @param sandbox Sandbox to simulate.
 * @param height, width Dimensions of sandbox.
 */
void process_sandbox(unsigned char **sandbox, int height, int width);


/**
 * Return the ID number of a tile, ranging from 0 to 15.
 *
 * @param tile Tile number, represented as a byte, to fetch ID number of.
 *
 * @return Value from 0 to 15 representing the type of tile given.
 */
unsigned char get_tile_id(unsigned char tile);


/**
 * Determine whether the given tile has already been updated or not in the
 * current pass of the sandbox, as determined by current_time.
 *
 * When a tile is updated, its updated flag is changed to match the parity of 
 * the time that has passed since the simulation began.
 *
 * @param tile Tile to determine whether it has been updated or not.
 * @param current_time Time that has passed in frames inside the simulation.
 *
 * @return True if the tile has already been updated, false otherwise.
 */
bool is_tile_updated(unsigned char tile, int current_time);


/**
 * Mutate the given tile's updated flag to show the tile has been updated.
 * This syncs the tile's flag to match the parity of the current simulation 
 * time.
 *
 * @param tile Pointer to tile to whose flag will be set.
 * @param current_time Time that has passed in frames inside the simulation.
 */
void set_tile_updated(unsigned char *tile, int current_time);


/**
 * Simulate gravity on the tile located at the given indices, within the
 * sandbox of the given dimensions, by mutating the sandbox.
 * 
 * Gravity is simulated on a tile by having the tile either fall down 1 tile or
 * sink through a liquid tile.
 * If neither of these are possible, then the tile will attempt to slide or sink
 * diagonally down left or right, choosing at random if both are possible.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform gravity within.
 * @param height, width Dimensions of given sandbox.
 * @param row_index, column_index Coordinates of tile to perform gravity on.
 */
void do_gravity(unsigned char **sandbox, int height, int width, int row_index, int column_index);


/**
 * Simulate flow on the tile at the given indices as though it were a liquid.
 *
 * Flow is simulated on a tile by moving a left or right at random. 
 * There must be space at the left/right the tile must be on top of a 
 * solid floor or other liquids.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform flow within.
 * @param width, height Dimensions of given sandbox.
 * @param row_index, column_index Coordinates of tile to perform flow on.
 */
void do_liquid_flow(unsigned char **sandbox, int height, int width, int row_index, int column_index);


/**
 * Simulate lift on the tile at the given indices as though it were a gas.
 *
 * Lift is simulated on a tile by moving at random 1 tile left, right, up, 
 * upleft, or upright, wherever possible.
 *
 * A lifetd tile is free to potentially move to any one of the spaces above if
 * the space is either empty, a liquid, or a gas.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform lift within.
 * @param width, height Dimensions of given sandbox.
 * @param row_index, column_index Coordinates of tile to perform lift on.
 */
void do_lift(unsigned char **sandbox, int height, int width, int row_index, int column_index);


/**
 * Determine the parity of the given current time.
 *
 * @param current_time Time that has passed in frames since the sim began.
 *
 * @return 0 if the time is even, 1 if the time is odd.
 */
unsigned char get_time_parity(int current_time);


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
 * 
 * @return True if updated flag is set, false otherwise.
 */
bool get_updated_flag(unsigned char tile);


#endif // SANDBOX_H
