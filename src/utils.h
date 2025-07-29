#ifndef UTILS_H
#define UTILS_H

/**
 * A collection of general-purpose utility functions.
 * 
 * The random functions declared in this header can be seeded with srand()
 * for different random sequences.
 * 
 */

#include <stdbool.h>


/**
 * Generate an integer between the given ranges, inclusive.
 * 
 * @param min, max Inclusive bounds of the random integer to be generated.
 * @return Random integer in the interval [min, max].
 */
int randint(int min, int max);


/**
 * Generate a floating point value between 0 and 1, inclusive.
 * 
 * @return Random real number in the interval [0, 1].
 */
double random(void);


/**
 * Flip a coin, generating either heads or tails.
 * @return True for heads, false for tails.
 */
bool flip_coin(void);


/**
 * Compare the two given floating point values for approximate equality.
 * 
 * @param a,b Floating point values to check for relative equality.
 * @return True if the two passed values are roughly equal, false otherwise.
 */
bool approx_equal(double a, double b);


#endif // UTILS_H
