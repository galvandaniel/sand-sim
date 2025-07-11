#ifndef SANDBOX_H
#define SANDBOX_H

/*
 * A collection of functions for working with a sand simulation as a 2D array
 * of tiles.
 *
 * Each tile shall be represented as a single unsigned big-endian byte.
 *
 * The first 4 significant bits are reserved for tile flags.
 * The last 4 bits represent a tile ID number, from 0 to 15.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>


// Define the constant tile IDs 0 to 15.
enum tile_id {AIR, SAND, WATER, WOOD, STEAM, FIRE};


// Amount of time that has passed, in frames of simulation, since the sandbox
// has begun.
extern unsigned int SANDBOX_LIFETIME;


/*
 * Generate and allocate memory for an empty 2D sandbox of tiles with dimension
 * width X height.
 *
 * The sandbox begins filled with non-static air, equivalent to 0 in value.
 *
 * @param height - Vertical length of 2D sandbox.
 * @param width - Horizontal length of 2D sandbox.
 *
 * @return - Pointer to allocated 2D array of bytes representing a sandbox.
 */
unsigned char **create_sandbox(unsigned int height, unsigned int width);


/*
 * Free all memory taken up by the given sandbox simulation.
 *
 * @param sandbox - Sandbox to free.
 * @param height, width - Dimensions of sandbox to free.
 */
void sandbox_free(unsigned char **sandbox, unsigned int height, unsigned int width);


/*
 * Perform one full iteration of simulation on the given sandbox, applying 
 * any tile interations, flow, gravity, etc.
 *
 * @param sandbox - Sandbox to simulate.
 * @param height, width - Dimensions of sandbox.
 */
void process_sandbox(unsigned char **sandbox, unsigned int height, unsigned int width);


/*
 * Return the ID number of a tile, ranging from 0 to 15.
 *
 * @param tile - Tile number, represented as a byte, to fetch ID number of.
 *
 * @return - Value from 0 to 15 representing the type of tile given.
 */
unsigned char get_tile_id(unsigned char tile);


/*
 * Determine whether the given tile has already been updated or not in the
 * current pass of the sandbox, as determined by current_time.
 *
 * When a tile is updated, its updated flag is changed to match the parity of 
 * the time that has passed since the simulation began.
 *
 * @param tile - Tile to determine whether it has been updated or not.
 * @param current_time - Time that has passed in frames inside the simulation.
 *
 * @return - 1 if the tile has already been updated, 0 otherwise.
 */
bool is_tile_updated(unsigned char tile, unsigned int current_time);


/*
 * Mutate the given tile's updated flag to show the tile has been updated.
 * This syncs the tile's flag to match the parity of the current simulation 
 * time.
 *
 * @param tile - Pointer to tile to whose flag will be set.
 * @param current_time - Time that has passed in frames inside the simulation.
 */
void set_tile_updated(unsigned char *tile, unsigned int current_time);


/*
 * Return whether the given tile is static in the sandbox or not.
 *
 * A tile is static if it has no foreesable updates to perform to the sandbox.
 *
 * For example, a tile of sand that cannot move downwards any further is
 * considered static.
 *
 * If a tile is static, it will not be updated and will not affect the sandbox.
 *
 * @param tile - Tile to determine if is static or not.
 * 
 * @return - 1 if the tile is static, 0 otherwise.
 *
 */
bool is_tile_static(unsigned char tile);


/*
 * Mutate the give tile's static flag to static or non-static, depending on
 * the given bool.
 *
 * @param tile - Pointer to tile whose static flag will be set.
 * @param should_set_static - Whether tile is set to static or non-static.
 */
void set_tile_static(unsigned char *tile, bool should_set_static);


/*
 * Simulate gravity on the tile located at the given indices, within the
 * sandbox of the given dimensions, by mutating the sandbox.
 *
 * This function does NOT check whether gravity should be performed on the tile
 * at the given coordinates.
 *
 * @param sandbox - 2D Sandbox of tiles to mutate and perform gravity within.
 * @param height, width - Dimensions of given sandbox.
 * @param row_index, column_index - Coordinates of tile to perform gravity on.
 */
void do_gravity(unsigned char **sandbox,
        unsigned int height,
        unsigned int width,
        unsigned int row_index,
        unsigned int column_index);


/*
 * Simulate flow on the tile at the given indices as though it were a liquid.
 *
 * Intuitively, this means moving a tile left or right at random, if there is
 * space and the tile has a floor.
 *
 * This function does NOT check whether flow should be performed on the tile
 * at the given coordinates.
 *
 * @param sandbox - 2D Sandbox of tiles to mutate and perform flow within.
 * @param width, height - Dimensions of given sandbox.
 * @param row_index, column_index - Coordinates of tile to perform flow on.
 */
void do_liquid_flow(unsigned char **sandbox,
        unsigned int height,
        unsigned int width,
        unsigned int row_index,
        unsigned int column_index);


/*
 * Simulate lift on the tile at the given indices as though it were a gas.
 *
 * Intuitively, lift is the opposite of gravity and liquid flow, combined as one. 
 *
 * Moving up if possible, ascending to the top left or topright otherwise.
 * If neither of those are possible, move left or right at random if a ceiling
 * is present.
 *
 * This function does NOT check whether the tile at the given coordinates should
 * have lift.
 *
 * @param sandbox - 2D Sandbox of tiles to mutate and perform lift within.
 * @param width, height - Dimensions of given sandbox.
 * @param row_index, column_index - Coordinates of tile to perform lift on.
 */
void do_lift(unsigned char **sandbox,
        unsigned int height,
        unsigned int width,
        unsigned int row_index,
        unsigned int column_index);


/*
 * Determine the parity of the given current time.
 *
 * @param current_time - Time that has passed in frames since the sim began.
 *
 * @return - 0 if the time is even, 1 if the time is odd.
 */
unsigned char get_time_parity(unsigned int current_time);


/*
 * Obtain the updated flag from a tile, synced to the parity of the time from
 * when it was last updated.
 *
 * The updated flag on its own does NOT say whether the tile is currently
 * updated or not. The flag represents a parity, it is NOT a boolean.
 *
 * Use is_tile_updated() to determine whether a tile is updated or not.
 *
 * @param tile - Tile to get updated flag from.
 */
bool get_updated_flag(unsigned char tile);


/*
 * Print a string representation of a 2D sandbox to stdout.
 *
 * A sandbox is represented as a string by a '-' denoting air, 'O' sand, 
 * '_' water.
 *
 * @param sandbox - Sandbox to print to stdout.
 * @param height, width - Dimensions of the given sandbox.
 */
void print_sandbox(unsigned char **sandbox, unsigned int height, unsigned int width);


#endif // SANDBOX_H
