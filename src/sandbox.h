#ifndef SANDBOX_H
#define SANDBOX_H

/**
 * A collection of functions for defining and processing a sand simulation.
 *
 * Each tile shall be represented as a single byte.
 *
 * The first 4 significant bits are reserved for tile flags.
 * The last 4 bits represent a tile type ID number, from 0 to 15.
 */

 
#include <stdbool.h>


// All tile types a tile particle can be. AIR denotes the empty tile.
enum tile_type {AIR, 
                SAND, 
                WATER, 
                WOOD, 
                STEAM, 
                FIRE,
                NUM_TILE_TYPES};


/**
 * Type describing a sandbox simulation, containing particle grid data, grid
 * dimensions, and grid metadata.
 */
struct Sandbox
{
    // 2D array of bytes, each byte encoding 1 particle of the sandbox.
    unsigned char **grid;

    // Sandbox dimensions in particle tiles.
    int width;
    int height;

    // Amount of times sandbox has been simulated (one 'frame') since the
    // sandbox has been created.
    long long lifetime;
};


/**
 * Type packing together a (row, col) coordinate in some sandbox.
 */
struct SandboxPoint
{
    int row;
    int col;
};


/**
 * Generate and allocate memory for an empty 2D sandbox of tiles with dimension
 * width X height.
 *
 * The sandbox begins filled with air, equivalent to 0 in value.
 *
 * @param width Horizontal length of 2D sandbox in particle tiles.
 * @param height Vertical length of 2D sandbox in particles tiles.
 * @return Pointer to allocated sandbox.
 */
struct Sandbox *create_sandbox(int width, int height);


/**
 * Free all memory taken up by the given sandbox simulation.
 *
 * @param sandbox Sandbox to free.
 */
void sandbox_free(struct Sandbox *sandbox);


/**
 * Perform one full iteration of simulation on the given sandbox, applying 
 * any tile interactions, flow, gravity, flamability, etc.
 *
 * @param sandbox Sandbox to simulate.
 */
void process_sandbox(struct Sandbox *sandbox);


/**
 * Simulate gravity on the tile located at the given point by mutating the 
 * sandbox.
 * 
 * Gravity is simulated on a tile by having the tile either fall down 1 tile or
 * sink through a liquid tile.
 * If neither of these are possible, then the tile will attempt to slide or sink
 * diagonally down left or right, choosing at random if both are possible.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform gravity within.
 * @param coords Coordinates of tile to perform gravity on.
 */
void do_gravity(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Simulate flow on the tile at the given point as though it were a liquid.
 *
 * Flow is simulated on a tile by moving a left or right at random. 
 * There must be space at the left/right the tile must be on top of a 
 * solid floor or other liquids.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform flow within.
 * @param coords Coordinates of tile to perform flow on.
 */
void do_liquid_flow(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Simulate lift on the tile at the given point as though it were a gas.
 *
 * Lift is simulated on a tile by moving at random 1 tile left, right, up, 
 * upleft, or upright, wherever possible.
 *
 * A lifted tile is free to potentially move to any one of the spaces above if
 * the space is either empty or a gas. 
 * Lift through a liquid can only occur upwards.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform lift within.
 * @param coords Coordinates of tile to perform lift on.
 */
void do_lift(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Simulate extinguishing of fire at the given point.
 * 
 * Extinguishing is simulated on a tile by checking if water is directly
 * adjacent in any of the cardinal directions, and turning to smoke if so.
 * 
 * @param sandbox 2D Sandbox of tiles to simulate extinguishing within.
 * @param coords Coordinates of tile to simulate extinguishing on.
 */
void do_extinguish(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Determine whether the given sandbox coordinates are OOB for given sandbox.
 * 
 * @param sandbox Sandbox determining the bounds on which to enforce on coords.
 * @param coords (row, col) coordinates inside Sandbox packed into a point.
 * @return True if coords are OOB, false otherwise.
 */
bool is_coord_oob(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Create a new tile particle of the given tile type whose updated flag is 
 * synced to the parity of the given sandbox's lifetime as though put through
 * a call to set_tile_updated().
 * 
 * @param sandbox Sandbox with which the returned t
 * @param tile_type Type which the generated tile is given.
 * @return Tile particle, encoded as a single byte.
 */
unsigned char create_tile(struct Sandbox *sandbox, enum tile_type new_type);


/**
 * Place a tile of the given tile type inside the sandbox at given coordinates.
 * If a tile is already present at the given coordinates, this function does
 * nothing.
 * 
 * The new tile, if created, is set to being updated.
 *
 * @param sandbox Sandbox to mutate and place tile in.
 * @param coords Coordinate to place new tile.
 * @param type Type of the new tile.
 */
void place_tile(struct Sandbox *sandbox, struct SandboxPoint coords, enum tile_type type);


/**
 * Remove the tile inside the sandbox at the given coordinates, replacing the
 * tile already present with AIR.
 * 
 * If AIR is already present at the given coordinates, this function does
 * nothing.
 * 
 * @param sandbox Sandbox to mutate and remove tile in.
 * @param coords Coordinates at which to remove tile.
 */
void delete_tile(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Replace the tile inside the sandbox at the given coordinates with a new tile
 * of the given type. The new tile is set to being updated.
 * 
 * If a tile of identical type is already present at the given coordinates, this
 * function does nothing.
 * 
 * @param sandbox Sandbox to mutate and replace tile in.
 * @param coords Coordinate to replace old tile with new tile.
 * @param type Type of the new tile.
 */
void replace_tile(struct Sandbox *sandbox, struct SandboxPoint coords, enum tile_type type);


/**
 * Return the type of a tile, describing its properties in simulation.
 *
 * @param tile Tile, represented as a byte, to fetch type of.
 * @return Value from 0 to 15 representing the type of tile given.
 */
enum tile_type get_tile_type(unsigned char tile);


/**
 * Return whether the given tile is an empty space or not.
 * 
 * @param tile Tile to determine if is empty and replaceable or not.
 * @return True if tile is empty, false otherwise.
 */
bool is_tile_empty(unsigned char tile);


/**
 * Determine whether the given tile has already been updated or not in the
 * current pass of the sandbox, as determined by current_time.
 *
 * When a tile is updated, its updated flag is changed to match the parity of 
 * the time that has passed since the simulation began.
 *
 * @param tile Tile to determine whether it has been updated or not.
 * @param current_time Time that has passed in frames inside the simulation.
 * @return True if the tile has already been updated, false otherwise.
 */
bool is_tile_updated(unsigned char tile, long long current_time);


/**
 * Mutate the given tile's updated flag to show the tile has been updated.
 * This syncs the tile's flag to match the parity of the current simulation 
 * time.
 *
 * @param tile Pointer to tile to whose flag will be set.
 * @param current_time Time that has passed in frames inside the simulation.
 */
void set_tile_updated(unsigned char *tile, long long current_time);


/**
 * Determine the parity of the given current time.
 *
 * @param current_time Time that has passed in frames since the sim began.
 * @return 0 if the time is even, 1 if the time is odd.
 */
unsigned char get_time_parity(long long current_time);


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
bool get_updated_flag(unsigned char tile);


#endif // SANDBOX_H
