#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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

Instruction code[CODE_LIMIT];
int num_instructions;
Instruction* start_program;

Instruction* gen_code(OpCode op, int p, int q) {
    code[num_instructions].op = op;
    code[num_instructions].p = p;
    code[num_instructions].q = q;
    num_instructions++;
    return &code[num_instructions - 1];
}

void initialize_stdlib() {
    start_program = gen_code(OP_J, 0, 0);
    {
        // WRITEI(n)
        SymbolTableEntry temp_entry = make_entry("WRITEI", TYPE_PROCEDURE);
        SymbolTableEntry* entry = add_entry(symbol_table, temp_entry);
        SymbolTable* sub_symbol_table = make_symbol_table(symbol_table);
        entry->subproc_symtab = sub_symbol_table;
        SymbolTableEntry* arg = add_entry(sub_symbol_table, make_entry("N", TYPE_VARIABLE));
        arg->is_reference = 0;
        sub_symbol_table->num_args++;
        sub_symbol_table->start_proc = num_instructions;
        gen_code(OP_INT, 0, 5);
        gen_code(OP_LV, 0, 4);
        gen_code(OP_WRI, 0, 0);
        gen_code(OP_EP, 0, 0);
    }
    {
        // WRITELN
        SymbolTableEntry temp_entry = make_entry("WRITELN", TYPE_PROCEDURE);
        SymbolTableEntry* entry = add_entry(symbol_table, temp_entry);
        SymbolTable* sub_symbol_table = make_symbol_table(symbol_table);
        entry->subproc_symtab = sub_symbol_table;
        sub_symbol_table->start_proc = num_instructions;
        gen_code(OP_INT, 0, 4);
        gen_code(OP_WLN, 0, 0);
        gen_code(OP_EP, 0, 0);
    }
    {
        // READI(n)
        SymbolTableEntry temp_entry = make_entry("READI", TYPE_PROCEDURE);
        SymbolTableEntry* entry = add_entry(symbol_table, temp_entry);
        SymbolTable* sub_symbol_table = make_symbol_table(symbol_table);
        entry->subproc_symtab = sub_symbol_table;
        SymbolTableEntry* arg = add_entry(sub_symbol_table, make_entry("N", TYPE_VARIABLE));
        arg->is_reference = 1;
        sub_symbol_table->num_args++;
        sub_symbol_table->start_proc = num_instructions;
        gen_code(OP_INT, 0, 5);
        gen_code(OP_LV, 0, 4);
        gen_code(OP_RI, 0, 0);
        gen_code(OP_EP, 0, 0);
    }
}

int get_level(SymbolTableEntry* entry) {
    // return relative level with current frame
    return symbol_table->level - entry->container->level;
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

            gen_code(OP_LA, get_level(entry), entry->offset);
            gen_code(OP_LC, 0, 1);

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

            gen_code(OP_MUL, 0, 0);
            gen_code(OP_ADD, 0, 0);
            gen_code(OP_LI, 0, 0);
        } else {
            if (entry && entry->object_type != TYPE_INT) {
                add_error(concat(entry->name, " must be an INTEGER VARIABLE or CONSTANT"));
                set_type_error(info);
            }
            
            gen_code(OP_LV, get_level(entry), entry->offset);
            if (entry->is_reference) {
                gen_code(OP_LI, 0, 0);
            }
        }
    } else if (token == NUMBER) {
        next_token();
        gen_code(OP_LC, 0, numeric_value);
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
        TokenType op_token = token;
        next_token();
        factor(&factor_info);
        ensure_int(info, factor_info);
        info->symbol_type = TYPE_CONSTANT;
        if (op_token == TIMES) {
            gen_code(OP_MUL, 0, 0);
        } else {
            gen_code(OP_DIV, 0, 0);
        }
    }
}

void expression(ObjectInfo *info) {
    info->object_type = TYPE_INT;
    info->symbol_type = TYPE_VARIABLE;

    TokenType op_token = NONE;
    if (token == PLUS || token == MINUS) {
        op_token = token;
        next_token();
        info->symbol_type = TYPE_CONSTANT;
    }

    ObjectInfo term_info;
    term(&term_info);
    ensure_int(info, term_info);

    if (op_token == MINUS) {
        gen_code(OP_NEG, 0, 0);
    }

    if (info->symbol_type == TYPE_VARIABLE) {
        *info = term_info;
    }

    while (token == PLUS || token == MINUS) {
        op_token = token;
        next_token();
        term(&term_info);
        ensure_int(info, term_info);
        info->symbol_type = TYPE_CONSTANT;

        if (op_token == PLUS) {
            gen_code(OP_ADD, 0, 0);
        } else {
            gen_code(OP_SUB, 0, 0);
        }
    }
}

void condition(ObjectInfo *info) {
    info->object_type = TYPE_BOOL;
    ObjectInfo expression_info;
    if (token == ODD) { // No instruction for this?
        next_token();
        expression(&expression_info);
        ensure_int(info, expression_info);
    } else {
        expression(&expression_info);
        ensure_int(info, expression_info);
        if (token == EQU || token == NEQ || token == LSS ||
            token == LEQ || token == GTR || token == GEQ) {
            TokenType relop_token = token;
            next_token();
            expression(&expression_info);
            ensure_int(info, expression_info);

            if (relop_token == EQU) {
                gen_code(OP_EQ, 0, 0);
            } else if (relop_token == NEQ) {
                gen_code(OP_NE, 0, 0);
            } else if (relop_token == LSS) {
                gen_code(OP_LT, 0, 0);
            } else if (relop_token == LEQ) {
                gen_code(OP_LE, 0, 0);
            } else if (relop_token == GTR) {
                gen_code(OP_GT, 0, 0);
            } else { // relop_token == GEQ
                gen_code(OP_GE, 0, 0);
            }
        } else {
            error("expected comparison operator");
        }
    }
}

void lvalue(ObjectInfo *info) {
    info->object_type = TYPE_INT;
    info->symbol_type = TYPE_VARIABLE;

    SymbolTableEntry *entry = get_entry_by_name(symbol_table, identifier);
    if (!entry) {
        add_error(concat(identifier, " was not declared in this scope"));
        set_type_error(info);
    }
    if (entry && entry->symbol_type != TYPE_VARIABLE) {
        add_error(concat(entry->name, " must be a VARIABLE"));
        set_type_error(info);
    }

    if (entry->is_reference) {
        gen_code(OP_LV, get_level(entry), entry->offset);
    } else {
        gen_code(OP_LA, get_level(entry), entry->offset);
    }

    next_token();
    if (token == LBRACK) {
        if (entry && entry->object_type != TYPE_ARRAY) {
            add_error(concat(entry->name, " must be an ARRAY"));
            set_type_error(info);
        }

        gen_code(OP_LC, 0, 1);

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
        
        gen_code(OP_MUL, 0, 0);
        gen_code(OP_ADD, 0, 0);
    } else {
        if (entry && entry->object_type != TYPE_INT) {
            add_error(concat(entry->name, " must be an INTEGER VARIABLE"));
            set_type_error(info);
        }
    }
}

void statement(ObjectInfo *info) {
    if (token == IDENT) {
        lvalue(info);
        if (info->object_type == TYPE_ERROR) exit(-1);
        if (token == ASSIGN) {
            next_token();
            ObjectInfo expression_info;
            expression(&expression_info);
            if (expression_info.object_type != TYPE_INT) {
                add_error("right-value must be INTEGER");
            }
            *info = expression_info;

            gen_code(OP_ST, 0, 0);
        } else {
            error("expected ASSIGN operator");
        }
    } else if (token == CALL) {
        int num_args_passed = 0;
        next_token();
        if (token == IDENT) {
            SymbolTableEntry *entry = get_entry_by_name(symbol_table, identifier);
            if (!entry) {
                error(concat(identifier, " was not declared in this scope"));
            }
            if (entry->symbol_type != TYPE_PROCEDURE) {
                add_error(concat(entry->name, " must be a PROCEDURE"));
            }

            gen_code(OP_INT, 0, 4);

            next_token();
            if (token == LPARENT) {
                do {
                    if (num_args_passed >= entry->subproc_symtab->num_args) {
                        error(concat("wrong number of arguments for call to ", entry->name));
                    }
                    int must_be_lvalue = entry->subproc_symtab->pool[num_args_passed].is_reference;
                    next_token();
                    ObjectInfo expression_info;
                    if (must_be_lvalue) {
                        lvalue(&expression_info);
                    } else {
                        expression(&expression_info);
                    }

                    if (must_be_lvalue && (expression_info.object_type != TYPE_INT || expression_info.symbol_type != TYPE_VARIABLE)) {
                        add_error("argument must be a VARIABLE");
                    }

                    num_args_passed++;
                } while (token == COMMA);
                if (token == RPARENT) {
                    next_token();
                } else {
                    error("expected ')'");
                }
            }
            if (entry->symbol_type == TYPE_PROCEDURE && num_args_passed != entry->subproc_symtab->num_args) {
                add_error(concat("wrong number of arguments for call to ", entry->name));
            }
            info->object_type = TYPE_VOID;

            gen_code(OP_DCT, 0, 4 + num_args_passed);
            gen_code(OP_CALL, get_level(entry), entry->subproc_symtab->start_proc);
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

        Instruction* false_jump = gen_code(OP_FJ, 0, 0);

        if (token == THEN) {
            next_token();
            ObjectInfo statement_info;
            statement(&statement_info);
            
            false_jump->q = num_instructions;

            if (token == ELSE) {
                Instruction* jump = gen_code(OP_J, 0, 0);
                false_jump->q = num_instructions;
                next_token();
                statement(&statement_info);
                jump->q = num_instructions;
            }
            *info = statement_info;
        } else {
            error("expected THEN");
        }
    } else if (token == WHILE) {
        int start_loop = num_instructions;

        next_token();
        ObjectInfo condition_info;
        condition(&condition_info);
        if (condition_info.object_type != TYPE_BOOL) {
            add_error("condition expected after WHILE");
        }

        Instruction* false_jump = gen_code(OP_FJ, 0, 0);

        if (token == DO) {
            next_token();
            ObjectInfo statement_info;
            statement(&statement_info);
            *info = statement_info;

            gen_code(OP_J, 0, start_loop);
            false_jump->q = num_instructions;
        } else {
            error("expected DO");
        }
    } else if (token == FOR) {
        next_token();
        if (token == IDENT) {
            SymbolTableEntry *entry = get_entry_by_name(symbol_table, identifier);
            if (!entry) {
                error(concat(identifier, " was not declared in this scope"));
            }
            if (entry->symbol_type != TYPE_VARIABLE || entry->object_type != TYPE_INT) {
                add_error(concat(entry->name, " must be a INTEGER VARIABLE"));
            }

            gen_code(OP_LA, get_level(entry), entry->offset);
            gen_code(OP_CV, 0, 0);

            next_token();
            if (token == ASSIGN) {
                next_token();
                ObjectInfo expression_info;
                expression(&expression_info);

                gen_code(OP_ST, 0, 0);
                int start_loop = num_instructions;
                gen_code(OP_CV, 0, 0);
                gen_code(OP_LI, 0, 0);

                if (token == TO) {
                    next_token();
                    expression(&expression_info);

                    gen_code(OP_LE, 0, 0);
                    Instruction* false_jump = gen_code(OP_FJ, 0, 0);

                    if (token == DO) {
                        next_token();
                        ObjectInfo statement_info;
                        statement(&statement_info);

                        gen_code(OP_CV, 0, 0);
                        gen_code(OP_CV, 0, 0);
                        gen_code(OP_LI, 0, 0);
                        gen_code(OP_LC, 0, 1);
                        gen_code(OP_ADD, 0, 0);
                        gen_code(OP_ST, 0, 0);
                        gen_code(OP_J, 0, start_loop);
                        false_jump->q  = num_instructions;
                        gen_code(OP_DCT, 0, 1);
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
                    error(concat(temp_entry.name, " already declared in this scope"));
                }
                next_token();
                if (token == LBRACK) {
                    entry->object_type = TYPE_ARRAY;
                    next_token();
                    if (token == NUMBER) {
                        entry->width = numeric_value;
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

    if (symbol_table->parent == NULL) initialize_stdlib();
    calculate_offsets(symbol_table);

    while (token == PROCEDURE) {
        next_token();
        if (token == IDENT) {
            SymbolTableEntry temp_entry = make_entry(identifier, TYPE_PROCEDURE);
            SymbolTableEntry* entry = add_entry(symbol_table, temp_entry);
            if (!entry) {
                error(concat(temp_entry.name, " already declared in this scope"));
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
                            error(concat(identifier, " already declared in this scope"));
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
                gen_code(OP_EP, 0, 0);
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

    symbol_table->start_proc = num_instructions;
    if (symbol_table->parent == NULL) start_program->q = num_instructions;
    gen_code(OP_INT, 0, 4 + symbol_table->total_width);

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
    gen_code(OP_HLT, 0, 0);
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

    int output_to_file = 0;
    if (argc > 2) {
        freopen(argv[2], "w", stdout);
        output_to_file = 1;
    }

    if (num_errors == 0) {
        fprintf(stdout, "\n");
        for (int i = 0; i < num_instructions; ++i) {
            if (!output_to_file) fprintf(stdout, "%2d: ", i);
            fprintf(stdout, "%s", ASM[code[i].op]);
            if (NUM_ARGS[code[i].op] == 1) {
                fprintf(stdout, " %d", code[i].q);
            } else if (NUM_ARGS[code[i].op] == 2) {
                fprintf(stdout, " %d %d", code[i].p, code[i].q);
            }
            fprintf(stdout, "\n");
        }
    }

    fclose(file);
    return 0;
}
