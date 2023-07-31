/*
 * Tiles have the following 4 bit flags, starting from most significant bit:
 *
 * 1. Update - If the tile has already been updated in the current update pass.
 * 2. Static - If the tile is set to not be updated due to reaching rest.
 * 3. (UNUSED)
 * 4. (UNUSED)
 *
 * The remaining 4 bits are used for tile ID numbers, going as follow:
 *
 * 0 - Air (empty tile)
 * 1 - Sand
 * 2 - Water
 * 3-15 - (UNUSED)
 *
 */

#include "sandbox.h"


unsigned char **create_sandbox(unsigned int height, unsigned int width)
{
    // Allocate memory for each row.
    unsigned char **new_sandbox = (unsigned char **) malloc(height * sizeof(unsigned char *));

    // Then allocate memory for each tile within each row, setting each tile to
    // 0, which corresponds to non-static air.
    for (int row_index = 0; row_index < height; row_index++)
    {
        new_sandbox[row_index] = (unsigned char *) calloc(width, sizeof(unsigned char));
    }

    return new_sandbox;
}


void sandbox_free(unsigned char **sandbox, unsigned int height, unsigned int width)
{
    // First, free each row as an array of bytes.
    // Then, free the array of pointers that pointed to each array of bytes.
    for (int row_index = 0; row_index < height; row_index++)
    {
        free(sandbox[row_index]);
    }

    free(sandbox);
}


unsigned char get_tile_id(unsigned char tile)
{
    // Use a mask of (0000 1111) to extract the first 4 bits.
    unsigned char tile_mask = 0x0f;
    unsigned char tile_id = tile_mask & tile;

    return tile_id;
}


bool is_tile_updated(unsigned char tile, unsigned int current_time)
{
    // Shift the updated bit flag to the front, and compare.
    unsigned char time_parity = get_time_parity(current_time);
    unsigned char updated_flag = tile >> 7;

    return updated_flag == time_parity;
}


void set_tile_updated(unsigned char *tile, unsigned int current_time)
{
    unsigned char time_parity = get_time_parity(current_time);

    // Create a mask of (1000 0000) or (0000 0000) to copy into updated flag.
    unsigned char parity_mask = time_parity << 7;

    // In the case of 0, we're updating a flag of 1 to 0, so we AND.
    // In the case of 1, we're updating a flag of 0 to 1, so we OR.
    if (time_parity == 0)
    {
        *tile &= parity_mask;
    }
    else
    {
        *tile |= parity_mask;
    }
}


unsigned char get_time_parity(unsigned int current_time)
{
    // Use a mask of (0000 ... 0001 to extract the first bit, granting parity.
    return current_time & 1;
}


bool is_tile_static(unsigned char tile)
{
    // Bring static flag to the front, extract first bit.
    unsigned char front_static = tile >> 6;
    return front_static & 1;
}
