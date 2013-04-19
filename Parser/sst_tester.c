/* 
 * sst_tester.c
 * -main ftn for testing simple string tbale
 * -tests creating simple table and printing
 *  values
 *
 *  Derek Salama & Jake Leichtling
 *  CS57
 *  4/19/2013
 */
#include <stdio.h>
#include "sst.h"

int main()
{
    Sst sst = create_sst(0);
    destroy_sst(sst);

    sst = create_sst(2);
    print_sst(sst);

    printf("%p\n\n", add_string(sst, "Hello there!"));
    print_sst(sst);

    printf("%p\n\n", add_string(sst, "Hi..."));
    print_sst(sst);

    printf("%p\n\n", add_string(sst, "Sup?"));
    print_sst(sst);

    printf("%p\n\n", add_string(sst, "Not much."));
    print_sst(sst);

    printf("%p\n\n", add_string(sst, "Totally!"));
    print_sst(sst);

    printf("%p\n\n", add_string(sst, "Hello there!"));
    print_sst(sst);

    destroy_sst(sst);
}
