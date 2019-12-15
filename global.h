#ifndef GLOBAL_H
#define GLOBAL_H

#define MAX_IDENT_LEN 10
#define ASCII_LIMIT 128
#define NUM_KEYWORDS 15
#define MAX_NUMERIC_VALUE 100000000

typedef enum {
    NONE = 0, IDENT, NUMBER, BEGIN, CALL, CONST, DO,  ELSE, END, FOR, IF, ODD,
    PROCEDURE, PROGRAM, THEN, TO, VAR, WHILE, PLUS, MINUS, TIMES, SLASH, EQU, NEQ,
    LSS, LEQ, GTR, GEQ, LPARENT, RPARENT, LBRACK, RBRACK, PERIOD, COMMA, SEMICOLON, ASSIGN, PERCENT
} TokenType;

static const TokenType KEYWORDS_TYPE[NUM_KEYWORDS] = {BEGIN, END, IF, THEN, WHILE, DO, CALL, ODD, TO, CONST, VAR, PROCEDURE, PROGRAM, ELSE, FOR};
static const char* TOKEN_TEXT[] = {
    "NONE", "IDENT", "NUMBER", "BEGIN", "CALL", "CONST", "DO",  "ELSE", "END", "FOR", "IF", "ODD",
    "PROCEDURE", "PROGRAM", "THEN", "TO", "VAR", "WHILE", "PLUS", "MINUS", "TIMES", "SLASH", "EQU", "NEQ",
    "LSS", "LEQ", "GTR", "GEQ", "LPARENT", "RPARENT", "LBRACK", "RBRACK", "PERIOD", "COMMA", "SEMICOLON", "ASSIGN", "PERCENT"
};

typedef enum {
    NULL_TYPE = -1, DIGIT = -2, SPACE = -3, LETTER = -4
} CharType;

CharType CHAR_TYPE[ASCII_LIMIT];
TokenType ONECHAR_TOKEN[ASCII_LIMIT];

FILE* file;
TokenType token;
int numeric_value;
char identifier[MAX_IDENT_LEN + 1];
int identifier_length;
int line_number;
int col_number;

#endif // GLOBAL_H
