#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>


void *xmalloc(size_t size, const char *file, int line)
{
    void *user_ptr = malloc(size);

    if (user_ptr == NULL)
    {
        fprintf(stderr, "ERROR: %s:%d\nReason: Unable to allocate requested (%zu) bytes in malloc().\n", file, line, size);
        exit(EXIT_FAILURE);
    }
    return user_ptr;
}


void* xcalloc( size_t num, size_t size, const char *file, int line)
{
    void *user_ptr = calloc(num, size);

    if (user_ptr == NULL)
    {
        fprintf(stderr, "ERROR: %s:%d\nReason: Unable to allocate requested (%zu) bytes in calloc().\n", file, line, size);
        exit(EXIT_FAILURE);  
    }
    return user_ptr;
}


int randint(int min, int max)
{
    return min + (rand() % (max + 1 - min));
}


double random(void)
{
    return rand() / (double) RAND_MAX;
}


bool flip_coin(void)
{
    // Generate a random number between 0 and 1.
    double random_value = random();

    if (random_value > 0.5)
    {
        return true;
    }

    return false;
}


// Implementation from learncpp.com chapter 6.7 by Alex (author of learncpp)
bool approx_equal(double a, double b)
{
    // Epsilon values chosen empirically.
    double relative_epsilon = 1e-8;
    double absolute_epsilon = 1e-12;

    if (fabs(a - b) <= absolute_epsilon)
    {
        return true;
    }

    // Fallback to Knuth's algorithm for FP comparison if absolute comparison
    // failed. Taken from "The Art of Computer Programming" (Addison-Wesley 1969)
    return (fabs(a - b) <= (fmax(fabs(a), fabs(b)) * relative_epsilon));
}


int clamp(int val, int min, int max)
{
    val = (val < min) ? min : val;
    val = (val > max) ? max : val;
    return val;
}
