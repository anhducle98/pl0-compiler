#include "symbol_table.h"

SymbolTable* make_symbol_table(SymbolTable *parent) {
    SymbolTable *table = (SymbolTable*) malloc(sizeof(SymbolTable));
    table->parent = parent;
    table->total_width = 0;
    table->pool = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
    table->size = 0;
    table->capacity = 1;
    table->num_args = 0; // MUST be updated later
    return table;
}

SymbolTableEntry make_entry(char *name, SymbolType symbol_type) {
    SymbolTableEntry entry;
    entry.name = (char*) malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(entry.name, name);
    entry.symbol_type = symbol_type;
    if (symbol_type == TYPE_PROCEDURE) {
        entry.width = 0;
    } else {
        entry.width = 4; // MUST be modified in case of ARRAY
        entry.object_type = TYPE_INT;
    }
    entry.subproc_symtab = NULL; // MUST be updated in case of PROCEDURE
    entry.is_reference = 0; // MUST be updated in case of VAR argument in PROCEDURE
    return entry;
}

SymbolTableEntry* get_entry_by_name(SymbolTable *table, char *name) {
    for (int i = 0; i < table->size; ++i) {
        if (strcmp(table->pool[i].name, name) == 0) {
            return &table->pool[i];
        }
    }
    if (table->parent  == NULL) return NULL;
    return get_entry_by_name(table->parent, name);
}

SymbolTableEntry* add_entry(SymbolTable *table, SymbolTableEntry entry) {
    // check existence
    for (int i = 0; i < table->size; ++i) {
        if (strcmp(table->pool[i].name, entry.name) == 0) {
            return NULL;
        }
    }

    table->pool[table->size++] = entry;
    if (table->size == table->capacity) {
        table->capacity *= 2;
        SymbolTableEntry *new_pool = (SymbolTableEntry*) malloc(table->capacity * sizeof(SymbolTableEntry));
        for (int i = 0; i < table->size; ++i) {
            new_pool[i] = table->pool[i];
        }
        free(table->pool);
        table->pool = new_pool;
    }
    return table->pool + table->size - 1;
}