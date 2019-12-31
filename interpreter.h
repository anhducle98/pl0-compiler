#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "global.h"

int s[STACK_LIMIT];
Instruction code[CODE_LIMIT];
int n; // number of instructions
int t; // stack top

#endif // INTERPRETER_H
