#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include "scanner.h"
#include "symbol_table.h"

typedef struct ObjectInfo ObjectInfo;

struct ObjectInfo {
    ObjectType object_type;
    SymbolType symbol_type;
    //char *code;
};

void error(const char msg[]);
void factor();
void term(); // done
void expression(); // done
void condition(); // done
void statement();
void block();
void program(); // done

#endif // PARSER_H
