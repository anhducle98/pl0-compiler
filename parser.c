#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "parser.h"

void next_token() {
    token = get_token();
}

void error(const char *msg) {
    fprintf(stdout, "\n%d:%d: error: %s\n", line_number, col_number, msg);
    exit(-1);
}

void add_error(const char *msg) {
    num_errors++;
    fprintf(stdout, "\n%d:%d: error: %s\n", line_number, col_number, msg);
    // continue working
}

char* concat(char *a, char *b) {
    char *c = (char*) malloc(strlen(a) + strlen(b) + 1);
    strcat(c, a);
    strcat(c, b);
    return c;
}

void set_type_error(ObjectInfo *info) {
    info->object_type = TYPE_ERROR;
}

int ensure_int(ObjectInfo *parent, ObjectInfo child) {
    if (child.object_type != TYPE_INT) {
        set_type_error(parent);
        return 0;
    }
    return 1;
}

void factor(ObjectInfo *info) {
    info->symbol_type = TYPE_CONSTANT;
    info->object_type = TYPE_INT;
    if (token == IDENT) {
        SymbolTableEntry *entry = get_entry_by_name(symbol_table, identifier);
        if (!entry) {
            add_error(concat(identifier, " was not declared in this scope"));
            set_type_error(info);
        }
        if (entry) {
            info->symbol_type = entry->symbol_type;
            info->object_type = entry->object_type;
        }
        if (entry && entry->symbol_type != TYPE_VARIABLE && entry->symbol_type != TYPE_CONSTANT) {
            add_error(concat(entry->name, " must be a VARIABLE or a CONSTANT"));
            set_type_error(info);
        }
        next_token();
        if (token == LBRACK) {
            if (entry && (entry->symbol_type != TYPE_VARIABLE || entry->object_type != TYPE_ARRAY)) {
                add_error(concat(entry->name, " must be an ARRAY VARIABLE"));
                set_type_error(info);
            }
            info->object_type = TYPE_INT;
            next_token();

            ObjectInfo expression_info;
            expression(&expression_info);
            if (expression_info.object_type != TYPE_INT) {
                add_error("Array index must be an INTEGER");
                set_type_error(info);
            }

            if (token == RBRACK) {
                next_token();
            } else {
                error("expected ']'");
            }
        } else {
            if (entry && entry->object_type != TYPE_INT) {
                add_error(concat(entry->name, " must be an INTEGER VARIABLE or CONSTANT"));
                set_type_error(info);
            }
        }
    } else if (token == NUMBER) {
        next_token();
    } else if (token == LPARENT) {
        next_token();
        
        ObjectInfo expression_info;
        expression(&expression_info);
        *info = expression_info;

        if (token == RPARENT) {
            next_token();
        } else {
            error("expected ')'");
        }
    } else {
        error("expected factor");
    }
}

void term(ObjectInfo *info) {
    ObjectInfo factor_info;
    factor(&factor_info);
    ensure_int(info, factor_info);

    *info = factor_info;

    while (token == TIMES || token == SLASH) {
        next_token();
        factor(&factor_info);
        ensure_int(info, factor_info);
        info->symbol_type = TYPE_CONSTANT;
    }
}

void expression(ObjectInfo *info) {
    info->object_type = TYPE_INT;
    info->symbol_type = TYPE_VARIABLE;

    if (token == PLUS || token == MINUS) {
        next_token();
        info->symbol_type = TYPE_CONSTANT;
    }
    ObjectInfo term_info;
    term(&term_info);
    ensure_int(info, term_info);

    if (info->symbol_type == TYPE_VARIABLE) {
        *info = term_info;
    }

    while (token == PLUS || token == MINUS) {
        next_token();
        term(&term_info);
        ensure_int(info, term_info);
        info->symbol_type = TYPE_CONSTANT;
    }
}

void condition(ObjectInfo *info) {
    info->object_type = TYPE_BOOL;
    ObjectInfo expression_info;
    if (token == ODD) {
        next_token();
        expression(&expression_info);
        ensure_int(info, expression_info);
    } else {
        expression(&expression_info);
        ensure_int(info, expression_info);
        if (token == EQU || token == NEQ || token == LSS ||
            token == LEQ || token == GTR || token == GEQ) {
            next_token();
            expression(&expression_info);
            ensure_int(info, expression_info);
        } else {
            error("expected comparison operator");
        }
    }
}

void statement(ObjectInfo *info) {
    if (token == IDENT) {
        SymbolTableEntry *entry = get_entry_by_name(symbol_table, identifier);
        if (!entry) {
            add_error(concat(identifier, " was not declared in this scope"));
        }
        if (entry && entry->symbol_type != TYPE_VARIABLE) {
            add_error(concat(entry->name, " must be a VARIABLE"));
        }
        next_token();
        if (token == LBRACK) {
            if (entry && entry->object_type != TYPE_ARRAY) {
                add_error(concat(entry->name, " must be an ARRAY"));
            }
            next_token();
            ObjectInfo expression_info;
            expression(&expression_info);
            if (expression_info.object_type != TYPE_INT) {
                add_error("Array index must be an INTEGER");
            }
            if (token == RBRACK) {
                next_token();
            } else {
                error("expected ']'");
            }
        } else {
            if (entry && entry->object_type != TYPE_INT) {
                add_error(concat(entry->name, " must be an INTEGER VARIABLE"));
            }
        }
        if (token == ASSIGN) {
            next_token();
            ObjectInfo expression_info;
            expression(&expression_info);
            if (expression_info.object_type != TYPE_INT) {
                add_error("right-value must be INTEGER");
            }
            *info = expression_info;
        } else {
            error("expected ASSIGN operator");
        }
    } else if (token == CALL) {
        int num_args_passed = 0;
        next_token();
        if (token == IDENT) {
            SymbolTableEntry *entry = get_entry_by_name(symbol_table, identifier);
            if (!entry) {
                add_error(concat(identifier, " was not declared in this scope"));
                exit(-1);
            }
            if (entry && entry->symbol_type != TYPE_PROCEDURE) {
                add_error(concat(entry->name, " must be a PROCEDURE"));
            }
            next_token();
            if (token == LPARENT) {
                next_token();
                ObjectInfo expression_info;
                expression(&expression_info);
                if (entry && num_args_passed >= entry->subproc_symtab->num_args) {
                    add_error(concat("wrong number of arguments for call to ", entry->name));
                } else if (entry && entry->subproc_symtab->pool[num_args_passed].is_reference && expression_info.symbol_type != TYPE_VARIABLE) {
                    add_error("argument must be a VARIABLE");
                }

                num_args_passed++;
                while (token == COMMA) {
                    next_token();
                    expression(&expression_info);
                    num_args_passed++;
                }
                if (token == RPARENT) {
                    next_token();
                } else {
                    error("expected ')'");
                }
            }
            if (entry && num_args_passed != entry->subproc_symtab->num_args) {
                add_error(concat("wrong number of arguments for call to ", entry->name));
            }
            info->object_type = TYPE_VOID;
        } else {
            error("expected function name");
        }
    } else if (token == BEGIN) {
        next_token();
        ObjectInfo statement_info;
        statement(&statement_info);
        while (token == SEMICOLON) {
            next_token();
            statement(&statement_info);
        }
        if (token == END) {
            next_token();
        } else {
            error("expected END");
        }
        info->object_type = TYPE_VOID;
    } else if (token == IF) {
        next_token();
        ObjectInfo condition_info;
        condition(&condition_info);
        if (condition_info.object_type != TYPE_BOOL) {
            add_error("condition expected after IF");
        }
        if (token == THEN) {
            next_token();
            ObjectInfo statement_info;
            statement(&statement_info);
            if (token == ELSE) {
                next_token();
                statement(&statement_info);
            }
            *info = statement_info;
        } else {
            error("expected THEN");
        }
    } else if (token == WHILE) {
        next_token();
        ObjectInfo condition_info;
        condition(&condition_info);
        if (condition_info.object_type != TYPE_BOOL) {
            add_error("condition expected after WHILE");
        }
        if (token == DO) {
            next_token();
            ObjectInfo statement_info;
            statement(&statement_info);
            *info = statement_info;
        } else {
            error("expected DO");
        }
    } else if (token == FOR) {
        next_token();
        if (token == IDENT) {
            SymbolTableEntry *entry = get_entry_by_name(symbol_table, identifier);
            if (!entry) {
                add_error(concat(identifier, " was not declared in this scope"));
            }
            if (entry && (entry->symbol_type != TYPE_VARIABLE || entry->object_type != TYPE_INT)) {
                add_error(concat(entry->name, " must be a INTEGER VARIABLE"));
            }
            next_token();
            if (token == ASSIGN) {
                next_token();
                ObjectInfo expression_info;
                expression(&expression_info);
                if (token == TO) {
                    next_token();
                    expression(&expression_info);
                    if (token == DO) {
                        next_token();
                        ObjectInfo statement_info;
                        statement(&statement_info);
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
                SymbolTableEntry entry = make_entry(identifier, TYPE_CONSTANT);
                add_entry(symbol_table, entry);

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
                SymbolTableEntry temp_entry = make_entry(identifier, TYPE_VARIABLE);
                SymbolTableEntry* entry = add_entry(symbol_table, temp_entry);
                if (!entry) {
                    add_error(concat(temp_entry.name, " already declared in this scope"));
                    exit(-1);
                }
                next_token();
                if (token == LBRACK) {
                    entry->object_type = TYPE_ARRAY;
                    next_token();
                    if (token == NUMBER) {
                        entry->width = numeric_value * 4;
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
            SymbolTableEntry temp_entry = make_entry(identifier, TYPE_PROCEDURE);
            SymbolTableEntry* entry = add_entry(symbol_table, temp_entry);
            if (!entry) {
                add_error(concat(temp_entry.name, " already declared in this scope"));
                exit(-1);
            }
            SymbolTable* sub_symbol_table = make_symbol_table(symbol_table);
            entry->subproc_symtab = sub_symbol_table;
            symbol_table = sub_symbol_table;
            // now working on sub-procedure's symbol table

            next_token();
            if (token == LPARENT) {
                next_token();
                while (1) {
                    int is_reference = 0;
                    if (token == VAR) {
                        is_reference = 1;
                        next_token();
                    }
                    if (token == IDENT) {
                        SymbolTableEntry* arg = add_entry(symbol_table, make_entry(identifier, TYPE_VARIABLE));
                        if (!arg) {
                            add_error(concat(identifier, " already declared in this scope"));
                            exit(-1);
                        }
                        symbol_table->num_args++;
                        arg->is_reference = is_reference;

                        next_token();
                        if (token == SEMICOLON) {
                            next_token();
                            continue;
                        } else if (token == RPARENT) {
                            next_token();
                            break;
                        } else {
                            error("expected ';'");
                        }
                    } else {
                        error("expected an identifier");
                    }
                }
            }
            if (token == SEMICOLON) {
                next_token();
                block();
                symbol_table = symbol_table->parent;
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
        ObjectInfo statement_info;
        statement(&statement_info);
        while (token == SEMICOLON) {
            next_token();
            statement(&statement_info);
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
    symbol_table = make_symbol_table(NULL);
    if (token == PROGRAM) {
        next_token();
        if (token == IDENT) {
            next_token();
            if (token == SEMICOLON) {
                next_token();
                block();
                if (token == PERIOD) {
                    if (num_errors == 0) {
                        fprintf(stdout, "\n\nSuccess!\n");
                    } else {
                        fprintf(stdout, "\n\nSomething is wrong, see above\n");
                    }
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
