/**
 * Tiles are encoded as bytes in the following bit layout: uccUtttt
 * 
 * The 4 most significant bits are reserved for tile flags.
 * The 4 least significant bits represent a tile type ID number, from 0 to 15.
 * 
 * u - Update flag. 1 If the tile has already been updated in the current 
 * simulation step, 0 otherwise.
 * c - Color code. A value from 0 to 3 representing a unique color variation
 * on the tile's color as determined by tile type.
 * U - (UNUSED)
 * t - Tile type identifier, a value from 0 to 15.
 */

#include "tile.h"
#include "utils.h"

/**
 * Bit fields associated with getting/removing Tile bit fields, along
 * with their distances from least significant bit (called `shift`).
 */
static constexpr uint8_t GET_UPDATE_MASK = 0b1000'0000;
static constexpr uint8_t REMOVE_UPDATE_MASK = 0b0111'1111;
static constexpr int UPDATE_FLAG_SHIFT = 7;

static constexpr uint8_t GET_TILE_MASK = 0b0000'1111;

static constexpr uint8_t REMOVE_COLOR_MASK = 0b1001'1111;
static constexpr uint8_t GET_COLOR_MASK = 0b0110'0000;
static constexpr int COLOR_SHIFT = 5;


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
    bool time_parity = is_odd(current_time);
    bool updated_flag = get_updated_flag(tile);
    return updated_flag == time_parity;
}


void set_tile_updated(Tile *tile, uint64_t current_time)
{
    // Mutate update flag to match parity of passed time.
    if (is_odd(current_time))
    {
        *tile |= GET_UPDATE_MASK;
    }
    else
    {
        *tile &= REMOVE_UPDATE_MASK;
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