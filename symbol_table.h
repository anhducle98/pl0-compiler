#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "global.h"
#include <stdlib.h>
#include <string.h>

typedef struct SymbolTable SymbolTable;
typedef struct SymbolTableEntry SymbolTableEntry;

struct SymbolTable {
    SymbolTable *parent;
    SymbolTableEntry *pool;
    int total_width;
    int size;
    int capacity;
    int num_args; // first entries in the pool are arguments
};

struct SymbolTableEntry {
    SymbolType symbol_type;
    ObjectType object_type;
    char *name;
    int width;
    struct SymbolTable *subproc_symtab;
    int is_reference;
};

SymbolTable* make_symbol_table(SymbolTable *parent);

SymbolTableEntry make_entry(char *name, SymbolType symbol_type);

SymbolTableEntry* get_entry_by_name(SymbolTable *table, char *name);

SymbolTableEntry* add_entry(SymbolTable *table, SymbolTableEntry entry);

SymbolTable *symbol_table;

#endif // SYMBOL_TABLE_H