#ifndef SANDBOX_H
#define SANDBOX_H

/**
 * A collection of functions for defining and processing a sand simulation.
 *
 * Each tile shall be represented as a single unsigned big-endian byte.
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
                FIRE};

/**
 * Classifications for how likely it is a tile particle will survive to next 
 * step in the sandbox.
 *  
 * SHORT: Particle is very likely to disappear on next frame.
 * AVERAGE: Particle is equally likely to disappear or survive on next frame. 
 * LONG: Particle is unlikely to disappear on next frame.
 * 
 * PERMANENT: Particle will never disappear unless deleted by user.
 */
enum lifespan_duration {SHORT, AVERAGE, LONG, PERMANENT};

/**
 * Odds of survival with length equal in size to lifespan_duration enum,
 * corresponding element-wise.
 * 
 * Example: survival_chances[SHORT] is equal to odds for survival of tile w/
 * a SHORT lifespan.
 * 
 * 0.0 indicates 0% chance of survival, 1.0 indicates 100% chance.
 */
extern const double survival_chances[];


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
 * Generate and allocate memory for an empty 2D sandbox of tiles with dimension
 * width X height.
 *
 * The sandbox begins filled with air, equivalent to 0 in value.
 *
 * @param width Horizontal length of 2D sandbox in particle tiles.
 * @param height Vertical length of 2D sandbox in particles tiles.
 *
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
 * Create a new tile particle of the given tile type whose updated flag is 
 * synced to the parity of the given sandbox's lifetime as though put through
 * a call to set_tile_updated().
 * 
 * @param sandbox Sandbox with which the returned t
 * @param tile_type Type which the generated tile is given.
 * 
 * @return Tile particle, encoded as a single byte.
 */
unsigned char create_tile(struct Sandbox *sandbox, enum tile_type new_type);


/**
 * Return the type of a tile, describing its properties in simulation.
 *
 * @param tile Tile, represented as a byte, to fetch type of.
 *
 * @return Value from 0 to 15 representing the type of tile given.
 */
enum tile_type get_tile_type(unsigned char tile);


/**
 * Get the classification a tile has for surviving to the next frame.
 * 
 * @param tile Tile to get classification for likelihood of survival.
 */
enum lifespan_duration get_tile_lifespan(unsigned char tile);


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
 * Simulate gravity on the tile located at the given indices by mutating the 
 * sandbox.
 * 
 * Gravity is simulated on a tile by having the tile either fall down 1 tile or
 * sink through a liquid tile.
 * If neither of these are possible, then the tile will attempt to slide or sink
 * diagonally down left or right, choosing at random if both are possible.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform gravity within.
 * @param row, col Coordinates of tile to perform gravity on.
 */
void do_gravity(struct Sandbox *sandbox, int row, int col);


/**
 * Simulate flow on the tile at the given indices as though it were a liquid.
 *
 * Flow is simulated on a tile by moving a left or right at random. 
 * There must be space at the left/right the tile must be on top of a 
 * solid floor or other liquids.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform flow within.
 * @param row, col Coordinates of tile to perform flow on.
 */
void do_liquid_flow(struct Sandbox *sandbox, int row, int col);


/**
 * Simulate lift on the tile at the given indices as though it were a gas.
 *
 * Lift is simulated on a tile by moving at random 1 tile left, right, up, 
 * upleft, or upright, wherever possible.
 *
 * A lifted tile is free to potentially move to any one of the spaces above if
 * the space is either empty or a gas. 
 * Lift through a liquid can only occur upwards.
 *
 * @param sandbox 2D Sandbox of tiles to mutate and perform lift within.
 * @param row, col Coordinates of tile to perform lift on.
 */
void do_lift(struct Sandbox *sandbox, int row, int col);


/**
 * Simulate extinguishing of fire at the given indices.
 * 
 * Extinguishing is simulated on a tile by checking if water is directly
 * adjacent in any of the cardinal directions, and turning to smoke if so.
 * 
 * @param sandbox 2D Sandbox of tiles to simulate extinguishing within.
 * @param row, col Coordinate sof tile to simulate extinguishing on.
 */
void do_extinguish(struct Sandbox *sandbox, int row, int col);


/**
 * Determine the parity of the given current time.
 *
 * @param current_time Time that has passed in frames since the sim began.
 *
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
 * 
 * @return True if updated flag is set, false otherwise.
 */
bool get_updated_flag(unsigned char tile);


#endif // SANDBOX_H
