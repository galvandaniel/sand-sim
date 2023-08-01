#include "sandbox.h"

int main(void)
{
    
    unsigned char **sandbox = create_sandbox(2, 3);
    sandbox[0][1] = 129;
    sandbox[1][1] = 1;

    print_sandbox(sandbox, 2, 3);

    do_gravity(sandbox, 2, 3, 0, 1);
    putchar('\n');

    print_sandbox(sandbox, 2, 3);


    unsigned char test_bit = 4;
    printf("Test_bit: %d\n", test_bit);
    set_tile_static(&test_bit, false);
    printf("Test_bit: %d\n", test_bit);


    sandbox_free(sandbox, 2, 3);
    return 0;
}
