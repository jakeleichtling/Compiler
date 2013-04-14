#ifndef SST_H_
#define SST_H_

struct simple_string_table_struct {
    char **table_ptr;
    int size;
};
typedef struct simple_string_table_struct *Sst;

Sst create_sst(int table_size);

char *add_string(Sst sst, char *str);

char *lookup_string(Sst sst, char *str);

void print_sst(Sst sst);

void destroy_sst(Sst sst);

#endif