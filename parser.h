#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

void error(const char msg[]);
void factor();
void term(); // done
void expression(); // done
void condition(); // done
void statement();
void block();
void program(); // done

#endif // PARSER_H
