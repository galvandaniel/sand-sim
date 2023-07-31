#include "sandbox.h"

int main(void)
{
    
    unsigned char **sandbox = create_sandbox(6, 7);

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            printf("Tile: %d\n", sandbox[i][j]);
        }
    }

    sandbox_free(sandbox, 6, 7);
    return 0;
}
