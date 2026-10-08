/**
 * Module for defining and processing a sand simulation.
 */

#ifndef SANDBOX_H
#define SANDBOX_H

#include "tile.h"

/**
 * Type describing a sandbox simulation, containing 2D grid data, grid
 * dimensions, and grid metadata.
 */
struct Sandbox
{
    // Flattened 2D array of Tiles.
    Tile *grid;

    // Sandbox dimensions in particle tiles.
    int width;
    int height;

    // Amount of times sandbox has been simulated (one 'frame') since the
    // sandbox has been created.
    uint64_t lifetime;
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
 * @return True if the tile at the given coordinates moved to a different 
 * location in the sandbox due to gravity, false otherwise
 */
bool do_gravity(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Simulate flow on the tile at the given point as though it were a liquid.
 *
 * Flow is simulated on a tile by moving a left or right at random. 
 * There must be space at the left/right the tile must be on top of a 
 * solid floor or other liquids.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform flow within.
 * @param coords Coordinates of tile to perform flow on.
 * @return True if the tile at the given coordinates moved to a different 
 * location in the sandbox due to flow, false otherwise
 */
bool do_flow(struct Sandbox *sandbox, struct SandboxPoint coords);


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
 * @return True if the tile at the given coordinates moved to a different 
 * location in the sandbox due to lift, false otherwise
 */
bool do_lift(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Simulate extinguishing of fire at the given point.
 * 
 * Extinguishing is simulated on a tile by checking if water is directly
 * adjacent in any of the cardinal directions, and turning to smoke if so.
 * 
 * @param sandbox 2D Sandbox of tiles to simulate extinguishing within.
 * @param coords Coordinates of tile to simulate extinguishing on. 
 * @return True if the tile at the given coordinates extinguished and turned to
 * smoke, false otherwise.
 */
bool do_extinguish(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Determine whether the given sandbox coordinates are OOB for given sandbox.
 * 
 * @param sandbox Sandbox determining the bounds on which to enforce on coords.
 * @param coords (row, col) coordinates inside Sandbox packed into a point.
 * @return True if coords are OOB, false otherwise.
 */
bool is_coord_oob(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Return a copy of the tile particle in the sandbox located at the given
 * packed (row, col) sandbox coordinates.
 * 
 * If the requested coordinates exist outside the bounds of the passed sandbox,
 * this function asserts.
 * 
 * @param sandbox Sandbox from which the returned tile is obtained.
 * @param coords (row, col) coordinates of tile to read from sandbox.
 * @return Tile particle from `sandbox` located at `coords`.
 */
Tile get_tile(struct Sandbox *sandbox, struct SandboxPoint coords);


/**
 * Create a new tile particle of the given tile type whose updated flag is 
 * synced to the parity of the given sandbox's lifetime as though put through
 * a call to set_tile_updated().
 * 
 * @param sandbox Sandbox with which the returned tile is synced to.
 * @param tile_type Type which the generated tile is given.
 * @return Tile particle of the given type.
 */
Tile create_tile(struct Sandbox *sandbox, enum tile_type new_type);


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


#endif // SANDBOX_H
