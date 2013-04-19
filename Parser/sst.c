/* sst.c
 * -Simple String Table
 * -datastructure used for storing IDs and string literals
 *  in parser AST tree
 *
 *  Jake Leichtling & Derek Salama
 *  CS57
 *  4/19/2013
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sst.h"

#define DEFAULT_TABLE_SIZE 1024

void expand_sst(Sst sst);

Sst create_sst(int table_size)
{
    Sst new_sst = malloc(sizeof(struct simple_string_table_struct));

    if (table_size > 0)
        new_sst->size = table_size;
    else
        new_sst->size = DEFAULT_TABLE_SIZE;

    new_sst->table_ptr = calloc(new_sst->size, sizeof(char *));

    return new_sst;
}

char *add_string(Sst sst, char *str)
{
    char *str_ptr = lookup_string(sst, str);
    if (str_ptr == NULL)
    {
        while (1)
        {
            int i;
            for (i = 0; i < sst->size; i++)
            {
                if (*(sst->table_ptr + i) == NULL)
                {
                    *(sst->table_ptr + i) = malloc(strlen(str) + 1);
                    strcpy(*(sst->table_ptr + i), str);
                    return *(sst->table_ptr + i);
                }
            }

            expand_sst(sst);
        }
    } else
        return str_ptr;
}

char *lookup_string(Sst sst, char *str)
{
    int i;
    for (i = 0; i < sst->size; i++)
    {
        char *str_bucket = *(sst->table_ptr + i);
        if (str_bucket != NULL && strcmp(str_bucket, str) == 0)
            return str_bucket;
    }

    return NULL;
}

void expand_sst(Sst sst)
{
    int double_size = sst->size * 2;

    char **double_table_ptr = calloc(double_size, sizeof(char *));
    int i;
    for (i = 0; i < sst->size; i++)
        *(double_table_ptr + i) = *(sst->table_ptr + i);

    sst->size = double_size;
    free(sst->table_ptr);
    sst->table_ptr = double_table_ptr;
}

void print_sst(Sst sst)
{
    int i;
    for (i = 0; i < sst->size; i++)
    {
        if (*(sst->table_ptr + i) == NULL)
            printf("%d: NULL\n", i);
        else
            printf("%d: %s\n", i, *(sst->table_ptr + i));
    }
    printf("\n");
}

void destroy_sst(Sst sst)
{
    int i;
    for (i = 0; i < sst->size; i++)
    {
        if (*(sst->table_ptr + i) != NULL)
            free(*(sst->table_ptr + i));
    }

    free(sst->table_ptr);
    free(sst);
}
