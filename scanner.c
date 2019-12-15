#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "scanner.h"

char current_char;

void initialize_scanner() {
    for (int i = 0; i < ASCII_LIMIT; ++i) CHAR_TYPE[i] = i;
    for (int i = 'a'; i <= 'z'; ++i) CHAR_TYPE[i] = LETTER;
    for (int i = 'A'; i <= 'Z'; ++i) CHAR_TYPE[i] = LETTER;
    for (int i = '0'; i <= '9'; ++i) CHAR_TYPE[i] = DIGIT;
    CHAR_TYPE[' '] = CHAR_TYPE['\t'] = CHAR_TYPE['\r'] = CHAR_TYPE['\n'] = SPACE;
    ONECHAR_TOKEN['+'] = PLUS;
    ONECHAR_TOKEN['-'] = MINUS;
    ONECHAR_TOKEN['*'] = TIMES;
    ONECHAR_TOKEN['/'] = SLASH;
    ONECHAR_TOKEN['%'] = PERCENT;
    ONECHAR_TOKEN['='] = EQU;
    ONECHAR_TOKEN[')'] = RPARENT;
    ONECHAR_TOKEN['['] = LBRACK;
    ONECHAR_TOKEN[']'] = RBRACK;
    ONECHAR_TOKEN[';'] = SEMICOLON;
    ONECHAR_TOKEN[','] = COMMA;
    ONECHAR_TOKEN['.'] = PERIOD;

    line_number = col_number = 1;
    current_char = '\n';
}

void get_char() {
    current_char = fgetc(file);
    if (current_char == EOF) return;
    putc(current_char, stdout);
    ++col_number;
    if (current_char == '\n') {
        ++line_number;
        col_number = 1;
    }
    if (CHAR_TYPE[current_char] == LETTER) current_char = toupper(current_char);
}

int get_keyword_id(const char* keyword_str) {
    for (int i = 0; i < NUM_KEYWORDS; ++i)
        if (strcmp(keyword_str, TOKEN_TEXT[KEYWORDS_TYPE[i]]) == 0) return i;
    return -1;
}

TokenType get_token() {
START:
    while (current_char != EOF && CHAR_TYPE[current_char] == SPACE) get_char();
    if (current_char == EOF) return NONE;

    switch (CHAR_TYPE[current_char])  {
        case LETTER: {
            identifier_length = 1;
            identifier[0] = current_char;

            get_char();
            while (current_char != EOF && (CHAR_TYPE[current_char] == LETTER || CHAR_TYPE[current_char] == DIGIT)) {
                if (identifier_length < MAX_IDENT_LEN) identifier[identifier_length++] = current_char;
                get_char();
            }
            identifier[identifier_length] = '\0';

            int keyword_id = get_keyword_id(identifier);
            if (keyword_id >= 0) return KEYWORDS_TYPE[keyword_id];

            return IDENT;
        }
        case DIGIT: {
            numeric_value = 0;
            while (current_char != EOF && CHAR_TYPE[current_char] == DIGIT) {
                if (numeric_value >= MAX_NUMERIC_VALUE) {
                    fprintf(stdout, "\n%d:%d: error: too large numeric value\n", line_number, col_number);
                    return NONE;
                }
                numeric_value = numeric_value * 10 + (current_char - '0');
                get_char();
            }
            return NUMBER;
        }
        case '(': {
            get_char();
            if (current_char == '*') {
                do get_char(); while (current_char != EOF && current_char != '*');
                do get_char(); while (current_char != EOF && current_char != ')');
                get_char();
                goto START; // ignore comments
            }
            return LPARENT;
        }
        case ':': {
            get_char();
            if (current_char != '=') goto STOP;
            get_char();
            return ASSIGN;
        }
        case '<': {
            get_char();
            if (current_char == '=') {
                get_char(); return LEQ;
            } else if (current_char == '>') {
                get_char(); return NEQ;
            }
            return LSS;
        }
        case '>': {
            get_char(); 
            if (current_char == '=') {
                get_char(); return GEQ;
            }
            return GTR;
        }
        default: {
            if (ONECHAR_TOKEN[current_char]) {
                TokenType result = ONECHAR_TOKEN[current_char];
                get_char(); return result;
            }
STOP:
            fprintf(stdout, "\n%d:%d: error: unexpected character \'%c\'\n", line_number, col_number, current_char);
            return NONE;
        }
    }
}

/*
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Missing source file argument");
        return -1;
    }

    file = fopen(argv[1], "rt");
    initialize_scanner();

    while (1) {
        TokenType token_type = get_token();
        if (token_type == NONE) break;
        printf("%s", TOKEN_TEXT[token_type]);
        if (token_type == IDENT) {
            printf("(%s)", identifier);
        } else if (token_type == NUMBER) {
            printf("(%d)", numeric_value);
        }
        printf("\n");
    }

    fclose(file);
    return 0;
}
*/
