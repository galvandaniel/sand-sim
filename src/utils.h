#ifndef UTILS_H
#define UTILS_H

/**
 * A collection of general-purpose utility functions.
 * 
 * The random functions declared in this header can be seeded with srand()
 * for different random sequences.
 * 
 */

#include <stddef.h>
#include <stdbool.h>

// malloc/calloc-wrappers which checks for NULL-failure and report to stderr.
#define SAFE_MALLOC(size) (xmalloc(size, __FILE__, __LINE__))
#define SAFE_CALLOC(num, size) (xcalloc(num, size, __FILE__, __LINE__))


/**
 * Allocate as many bytes of uninitialized storage as requested.
 * 
 * If this function is unable to allocate the number of bytes given, this
 * function will exit() the calling program, in a "succeed or die" approach
 * to memory allocation, printing the passed calling file and line number.
 * 
 * @param size Number of bytes to allocate.
 * @param file, line The expanded values of __FILE__ and __LINE__.
 * @return The pointer to the beginning of newly allocated memory. 
 * To avoid a memory leak, the returned pointer must be deallocated with free().
 */
void *xmalloc(size_t size, const char *file, int line);


/**
 * Allocates memory for an array of num objects of size and initializes all 
 * bytes in the allocated storage to zero.
 * 
 * If this function is unable to allocate the total number of bytes required, 
 * this function will exit() the calling program, in a "succeed or die" approach
 * to memory allocation, printing the passed calling file and line number.
 * 
 * @param num Number of objects to allocate.
 * @param size Size of each object.
 * @param file, line The expanded values of __FILE__ and __LINE__.
 * @return The pointer to the beginning of newly allocated memory. 
 * To avoid a memory leak, the returned pointer must be deallocated with free().
 */
void* xcalloc( size_t num, size_t size, const char *file, int line);


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


/**
 * Clamp the given value to lie within the inclusive integer range [min, max].
 * 
 * @param min Lower bound of clamping range.
 * @param max Upper bound of clampign range.
 * @return If val in [min, max], val. If val < min, min. If val > max, max.
 */
int clamp(int val, int min, int max);


#endif // UTILS_H
