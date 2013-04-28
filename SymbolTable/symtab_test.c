#include "symtab.h"
#include <stdio.h>

static char *vartype_strings[] = 
{
	"inttype",
	"doubletype",
	"other"
};

static char *type_strings[] = 
{
	"variable",
	"function",
	"stringconst"
};

void print_node(symnode node) {
	printf("Node: %s, type: %s, vartype: %s, addr: %d \n", node->name,
			type_strings[node->node_type],
			vartype_strings[node->var_type], 
			node->varaddr);
}

void print_symhashtable(symhashtable hashtable){
	int i;
	for (i = 0; i < hashtable->size; i++) {
		symnode node;
		for (node = hashtable->table[i]; node != NULL; node = node->next) {
			print_node(node);
		}
	}
}

void print_symboltable(symboltable symtab) {

	symhashtable table;
	for (table = symtab->inner_scope; table != NULL; table = table->outer_scope) {
		printf("Table at level %d contains:\n", table->level);
		print_symhashtable(table);
	}


}



int main()
{
	symboltable t = create_symboltable();
	enum nodetype type;
	enum vartype vtype;

	symnode str = insert_into_symboltable(t, "STR_CONST");
	type = stringconst;
	set_node_nodetype(str, type);

	symnode ftn = insert_into_symboltable(t, "FTN");
	type = function;
	set_node_nodetype(ftn, type);

	symnode var = insert_into_symboltable(t, "INT");
	type = variable;
	vtype = inttype;
	set_node_nodetype(var, type);
	set_node_vartype(var, vtype);
	set_node_addr(var, 1);

	print_symboltable(t);
	destroy_symboltable(t);
}
