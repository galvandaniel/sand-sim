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


/*
 * Generate and allocate memory for an empty 2D sandbox of tiles with dimension
 * width X height.
 *
 * The sandbox begins filled with non-static air.
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
 * @param height - Height of sandbox to free.
 * @param width - Width of sandbox to free.
 */
void sandbox_free(unsigned char **sandbox, unsigned int height, unsigned int width);


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
 * @param tile - Pointer to tile to mutate.
 * @param current_time - Time that has passed in frames inside the simulation.
 */
void set_tile_updated(unsigned char *tile, unsigned int current_time);


/*
 * Determine the parity of the given current time.
 *
 * @param current_time - Time that has passed in frames since the sim began.
 *
 * @return - 0 if the time is even, 1 if the time is odd.
 */
unsigned char get_time_parity(unsigned int current_time);


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


