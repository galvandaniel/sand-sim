#include "sandbox.h"
#include <unistd.h>

int main(void)
{
    unsigned char **sandbox = create_sandbox(10, 10);

    /*
    sandbox[0][5] = 1;
    sandbox[1][5] = 1;
    sandbox[0][2] = 1;
    */
    sandbox[0][0] = 1;
    sandbox[0][1] = 2;
    sandbox[0][2] = 2;
    // sandbox[1][4] = 1;

    while (true)
    {
        print_sandbox(sandbox, 10, 10);
        process_sandbox(sandbox, 10, 10);
        putchar('\n');

        sleep(1);
        if (SANDBOX_LIFETIME == 15)
        {
            break;
        }
    }

    sandbox_free(sandbox, 10, 10);
    return 0;
}
