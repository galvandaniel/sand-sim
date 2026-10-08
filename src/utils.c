#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>

void *xmalloc(size_t size, const char *file, int line)
{
    void *user_ptr = malloc(size);

    if (user_ptr == nullptr)
    {
        fprintf(stderr, "ERROR: %s:%d\nReason: Unable to allocate requested (%lu) bytes in malloc().\n", file, line, (unsigned long) size);
        exit(EXIT_FAILURE);
    }
    return user_ptr;
}


void* xcalloc(size_t num, size_t size, const char *file, int line)
{
    void *user_ptr = calloc(num, size);

    if (user_ptr == nullptr)
    {
        fprintf(stderr, "ERROR: %s:%d\nReason: Unable to allocate requested (%lu) bytes in calloc().\n", file, line, (unsigned long) size);
        exit(EXIT_FAILURE);  
    }
    return user_ptr;
}


int randint(int min, int max)
{
    return min + (rand() % ((max - min) + 1));
}


double random(void)
{
    return rand() / (double) RAND_MAX;
}


bool flip_coin(void)
{
    return randint(0, 1) == 1;
}


// Implementation from learncpp.com chapter 6.7 by Alex (author of learncpp)
bool approx_equal(double first, double second)
{
    // Epsilon values chosen empirically.
    constexpr double relative_epsilon = 1e-8;
    constexpr double absolute_epsilon = 1e-12;
    const double distance = fabs(first - second);

    if (distance <= absolute_epsilon)
    {
        return true;
    }

    // Fallback to Knuth's algorithm for FP comparison if absolute comparison
    // failed. Taken from "The Art of Computer Programming" (Addison-Wesley 1969)
    return (distance <= (fmax(fabs(first), fabs(second)) * relative_epsilon));
}


int clamp(int val, int min, int max)
{
    val = (val < min) ? min : val;
    val = (val > max) ? max : val;
    return val;
}


bool is_odd(uint64_t value)
{
    return value & 0b0000'0001;
}
