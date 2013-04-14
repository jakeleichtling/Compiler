#include "sst.h"

int main()
{
    Sst sst = create_sst(0);
    destroy_sst(sst);

    sst = create_sst(2);
    print_sst(sst);

    add_string(sst, "Hello there!");
    print_sst(sst);

    add_string(sst, "Hi...");
    print_sst(sst);

    destroy_sst(sst);
}