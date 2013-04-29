#include "symtab.h"
#include <stdio.h>


static char *type_strings[] = 
{
    "identifier",
    "stringconst"
};

static char *vartype_strings[] = 
{
	"inttype",
	"doubletype",
	"voidtype"
};

void print_node(symnode node)
{
    if (node) {
    	printf("Node: %s, type: %s, vartype: %s, addr: %d, ptr: %p \n", node->name,
			type_strings[node->node_type],
			vartype_strings[node->var_type], 
			node->varaddr,
            node
            );
    } else {
        printf("Node is null\n");
    }
}

void print_symhashtable(symhashtable hashtable)
{
	int i;
	for (i = 0; i < hashtable->size; i++) {
		symnode node;
		for (node = hashtable->table[i]; node != NULL; node = node->next) {
			print_node(node);
		}
	}
}

void print_symboltable(symboltable symtab)
{
	symhashtable table;
	for (table = symtab->inner_scope; table != NULL; table = table->outer_scope) {
		printf("Table at level %d contains:\n", table->level);
		print_symhashtable(table);
	}
}

int main()
{
	symboltable t = create_symboltable();
    int level;

    printf("1.) Add int x @ addr 1\n\n");
    symnode int_x = insert_into_symboltable(t, "x", identifier);
    set_node_vartype(int_x, inttype);
    set_node_addr(int_x, 1);

    printf("2.) Enter new scope\n\n");
    enter_scope(t);

    printf("3.) Add double x @ addr 2\n\n");
    symnode double_x = insert_into_symboltable(t, "x", identifier);
    set_node_vartype(double_x, doubletype);
    set_node_addr(double_x, 2);

    printf("4.) Enter new scope\n\n");
    enter_scope(t);

    printf("5.) Look up and print out id \"x\" --> double x @ level 1\n");
    print_node(lookup_in_symboltable(t, "x", &level, identifier));
    printf("@ level: %d\n\n", level);

    printf("6.) Add stringconst x @ addr 3\n\n");
    symnode sc_x = insert_into_symboltable(t, "x", stringconst);
    set_node_addr(sc_x, 3);

    printf("7.) Look up and print out stringconst \"x\" --> stringconst x @ level 2\n");
    print_node(lookup_in_symboltable(t, "x", &level, stringconst));
    printf("@ level: %d\n\n", level);

    printf("8.) Add stringconst x @ addr 3 (same name as before!)\n\n");
    insert_into_symboltable(t, "x", stringconst);

    printf("9.) Look up and print out stringconst \"x\" --> stringconst x @ level 2 (same node as before!)\n");
    print_node(lookup_in_symboltable(t, "x", &level, stringconst));
    printf("@ level: %d\n\n", level);

    printf("10.) Look up and print out id \"x\" --> double x @ level 1\n");
    print_node(lookup_in_symboltable(t, "x", &level, identifier));
    printf("@ level: %d\n\n", level);

    printf("11.) Add void function x @ addr 4\n\n");
    symnode v_ftn_x = insert_into_symboltable(t, "x", identifier);
    set_node_vartype(v_ftn_x, voidtype);
    set_node_addr(sc_x, 3);

    printf("12.) Look up and print out id \"x\" --> void function x @ level 2\n");
    print_node(lookup_in_symboltable(t, "x", &level, identifier));
    printf("@ level: %d\n\n", level);

    printf("13.) Print out the entire symbol table:\n------------------------------------\n");
    print_symboltable(t);
    printf("------------------------------------\n");

    printf("14.) Leave the current scope\n\n");

    printf("15.) Look up and print out stringconst \"x\" --> null\n");
    print_node(lookup_in_symboltable(t, "x", &level, stringconst));
    printf("@ level: %d\n\n", level);


    printf("17.) Leave the current scope\n\n");

	destroy_symboltable(t);
}
