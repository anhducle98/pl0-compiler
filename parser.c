#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "parser.h"

TokenType token;

void next_token() {
    token = get_token();
}

void error(const char *msg) {
    fprintf(stdout, "\n%d:%d: error: %s\n", line_number, col_number, msg);
    exit(-1);
}

void factor() {
    if (token == IDENT) {
        next_token();
        if (token == LBRACK) {
            next_token();
            expression();
            if (token == RBRACK) {
                next_token();
            } else {
                error("expected ']'");
            }
        }
    } else if (token == NUMBER) {
        next_token();
    } else if (token == LPARENT) {
        next_token();
        expression();
        if (token == RPARENT) {
            next_token();
        } else {
            error("expected ')'");
        }
    } else {
        error("expected factor");
    }
}

void term() {
    factor();
    while (token == TIMES || token == SLASH) {
        next_token();
        factor();
    }
}

void expression() {
    if (token == PLUS || token == MINUS) {
        next_token();
    }
    term();
    while (token == PLUS || token == MINUS) {
        next_token();
        term();
    }
}

void condition() {
    expression();
    if (token == EQU || token == NEQ || token == LSS ||
        token == LEQ || token == GTR || token == GEQ) {
        next_token();
        expression();
    } else {
        error("expected comparison operator");
    }
}

void statement() {
    if (token == IDENT) {
        next_token();
        if (token == LBRACK) {
            next_token();
            expression();
            if (token == RBRACK) {
                next_token();
            } else {
                error("expected ']'");
            }
        }
        if (token == ASSIGN) {
            next_token();
            expression();
        } else {
            error("expected ASSIGN operator");
        }
    } else if (token == CALL) {
        next_token();
        if (token == IDENT) {
            next_token();
            if (token == LPARENT) {
                next_token();
                expression();
                while (token == COMMA) {
                    next_token();
                    expression();
                }
                if (token == RPARENT) {
                    get_token();
                } else {
                    error("expected ')'");
                }
            }
        } else {
            error("expected function name");
        }
    } else if (token == BEGIN) {
        next_token();
        statement();
        while (token == SEMICOLON) {
            next_token();
            statement();
        }
        if (token == END) {
            next_token();
        } else {
            error("expected END");
        }
    } else if (token == IF) {
        next_token();
        condition();
        if (token == THEN) {
            next_token();
            statement();
            if (token == ELSE) {
                next_token();
                statement();
            }
        } else {
            error("expected THEN");
        }
    } else if (token == WHILE) {
        next_token();
        condition();
        if (token == DO) {
            next_token();
            statement();
        } else {
            error("expected DO");
        }
    } else if (token == FOR) {
        next_token();
        if (token == IDENT) {
            next_token();
            if (token == ASSIGN) {
                next_token();
                expression();
                if (token == TO) {
                    next_token();
                    expression();
                    if (token == DO) {
                        next_token();
                        statement();
                    } else {
                        error("expected DO");
                    }
                } else {
                    error("expected TO");
                }
            } else {
                error("expected ASSIGN operator");
            }
        } else {
            error("expected an identifier");
        }
    }
}

void block() {
    if (token == CONST) {
        next_token();
        while (1) {
            if (token == IDENT) {
                next_token();
                if (token == EQU) {
                    next_token();
                    if (token == NUMBER) {
                        next_token();
                        if (token == SEMICOLON) {
                            next_token();
                            break;
                        } else if (token == COMMA) {
                            next_token();
                            continue;
                        } else {
                            error("expected ';' or ','");
                        }
                    } else {
                        error("expected a number");
                    }
                } else {
                    error("expected '='");
                }
            } else {
                error("expected an identifier");
            }
        }
    }
    
    if (token == VAR) {
        next_token();
        while (1) {
            if (token == IDENT) {
                next_token();
                if (token == LBRACK) {
                    next_token();
                    if (token == NUMBER) {
                        next_token();
                        if (token == RBRACK) {
                            next_token();
                        } else {
                            error("expected ']'");
                        }
                    } else {
                        error("expected a number");
                    }
                }
                if (token == SEMICOLON) {
                    next_token();
                    break;
                } else if (token == COMMA) {
                    next_token();
                    continue;
                } else {
                    error("expected ';' or ','");
                }
            }
        }
    }

    while (token == PROCEDURE) {
        next_token();
        if (token == IDENT) {
            next_token();
            if (token == LPARENT) {
                next_token();
                while (1) {
                    if (token == VAR) {
                        next_token();
                    }
                    if (token == IDENT) {
                        next_token();
                        if (token == SEMICOLON) {
                            next_token();
                            continue;
                        } else if (token == RPARENT) {
                            next_token();
                            break;
                        } else {
                            error("expected ';' or ','");
                        }
                    } else {
                        error("expected an identifier");
                    }
                }
            }
            if (token == SEMICOLON) {
                next_token();
                block();
                if (token == SEMICOLON) {
                    next_token();
                    continue;
                } else {
                    error("expected ';'");
                }
            } else {
                error("expected ';'");
            }
        } else {
            error("expected an identifier");
        }
    }

    if (token == BEGIN) {
        next_token();
        statement();
        while (token == SEMICOLON) {
            next_token();
            statement();
        }
        if (token == END) {
            next_token();
        } else {
            error("expected END");
        }
    } else {
        error("expected BEGIN");
    }
}

void program() {
    if (token == PROGRAM) {
        next_token();
        if (token == IDENT) {
            next_token();
            if (token == SEMICOLON) {
                next_token();
                block();
                if (token == PERIOD) {
                    fprintf(stdout, "\n\nSuccess!\n");
                } else {
                    error("expected '.'");
                }
            } else {
                error("expected ';'");
            }
        } else {
            error("expected program name");
        }
    } else {
        error("expected PROGRAM keyword");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stdout, "Missing source file  argument");
        return -1;
    }

    file = fopen(argv[1], "rt");
    initialize_scanner();

    next_token();
    program();

    fclose(file);
    return 0;
}
