#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include "scanner.h"
#include "symbol_table.h"

typedef struct ObjectInfo ObjectInfo;

struct ObjectInfo {
    ObjectType object_type;
    SymbolType symbol_type;
};

void error(const char msg[]);
void factor();
void term();
void expression();
void condition();
void statement();
void block();
void program();

#endif // PARSER_H
