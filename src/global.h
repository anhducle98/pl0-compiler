#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>

#define MAX_IDENT_LEN 10
#define ASCII_LIMIT 128
#define NUM_KEYWORDS 15
#define MAX_NUMERIC_VALUE 100000000

typedef enum {
    NONE = 0, IDENT, NUMBER, BEGIN, CALL, CONST, DO,  ELSE, END, FOR, IF, ODD,
    PROCEDURE, PROGRAM, THEN, TO, VAR, WHILE, PLUS, MINUS, TIMES, SLASH, EQU, NEQ,
    LSS, LEQ, GTR, GEQ, LPARENT, RPARENT, LBRACK, RBRACK, PERIOD, COMMA, SEMICOLON, ASSIGN, PERCENT
} TokenType;

typedef enum {
    TYPE_ERROR,
    TYPE_VOID,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_ARRAY
} ObjectType;

typedef enum {
    TYPE_VARIABLE,
    TYPE_PROCEDURE,
    TYPE_CONSTANT
} SymbolType;


static const TokenType KEYWORDS_TYPE[NUM_KEYWORDS] = {BEGIN, END, IF, THEN, WHILE, DO, CALL, ODD, TO, CONST, VAR, PROCEDURE, PROGRAM, ELSE, FOR};
static const char* TOKEN_TEXT[] = {
    "NONE", "IDENT", "NUMBER", "BEGIN", "CALL", "CONST", "DO",  "ELSE", "END", "FOR", "IF", "ODD",
    "PROCEDURE", "PROGRAM", "THEN", "TO", "VAR", "WHILE", "PLUS", "MINUS", "TIMES", "SLASH", "EQU", "NEQ",
    "LSS", "LEQ", "GTR", "GEQ", "LPARENT", "RPARENT", "LBRACK", "RBRACK", "PERIOD", "COMMA", "SEMICOLON", "ASSIGN", "PERCENT"
};

typedef enum {
    NULL_TYPE = -1, DIGIT = -2, SPACE = -3, LETTER = -4
} CharType;

// ASM BEGIN
#define NUM_OPCODES 32
#define STACK_LIMIT 1000000
#define CODE_LIMIT 1000000

typedef enum {
    NOOP = 0, OP_LA, OP_LV, OP_LC, OP_LI, OP_INT, OP_DCT,
    OP_J, OP_FJ, OP_HLT, OP_ST, OP_CALL, OP_EP, OP_EF,
    OP_RC, OP_RI, OP_WRC, OP_WRI, OP_WLN,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_NEG, OP_CV,
    OP_EQ, OP_NE, OP_GT, OP_LT, OP_GE, OP_LE
} OpCode;

static int NUM_ARGS[NUM_OPCODES] = {
    0, 2, 2, 1, 0, 1, 1,
    1, 1, 0, 0, 2, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};

static const char* ASM[NUM_OPCODES] = {
    "\0", "LA", "LV", "LC", "LI", "INT", "DCT",
    "J", "FJ", "HLT", "ST", "CALL", "EP", "EF",
    "RC", "RI", "WRC", "WRI", "WLN",
    "ADD", "SUB", "MUL", "DIV", "NEG", "CV",
    "EQ", "NE", "GT", "LT", "GE", "LE"
};

typedef struct {
    OpCode op;
    int p;
    int q;
} Instruction;

// ASM END

CharType CHAR_TYPE[ASCII_LIMIT];
TokenType ONECHAR_TOKEN[ASCII_LIMIT];

FILE* file;
TokenType token;
int numeric_value;
char identifier[MAX_IDENT_LEN + 1];
int identifier_length;
int line_number;
int col_number;
int num_errors;

#endif // GLOBAL_H
