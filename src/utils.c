#include "utils.h"

#include <stdlib.h>
#include <stdbool.h>


int randint(int min, int max)
{
    return min + (rand() % (max + 1 - min));
}


double random(void)
{
    return (double) rand() / (double) RAND_MAX;
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
