#include "interpreter.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

void putint(int x) {
    printf("%d", x);
}

int getint() {
    int res;
    scanf("%d", &res);
    return res;
}

void read_asm(FILE *f) {
    n = 0;
    while (!feof(f)) {
        char op[10];
        fscanf(f, "%s", op);
        for (int i = 1; i < NUM_OPCODES; ++i) {
            if (strcmp(ASM[i], op) == 0) {
                code[n].op = i;
                if (NUM_ARGS[i] == 1) {
                    fscanf(f, "%d", &code[n].q);
                } else {
                    fscanf(f, "%d%d", &code[n].p, &code[n].q);
                }
                n++;
                break;
            }
        }
    }
}

int base(int b, int p) {
    int c = b;
    while (p > 0) {
        c = s[c + 3];
        --p;
    }
    return c;
}

void run() {
    int pc = 0, t = -1, b = 0;

    while (pc < n) {
        int op = code[pc].op;
        int p = code[pc].p;
        int q = code[pc].q;
        //printf("%s %d %d\n", ASM[op], p, q);
        switch (op) {
            case OP_LA:
                s[++t] = base(b, p) + q;
                break;
            case OP_LV:
                s[++t] = s[base(b, p) + q];
                break;
            case OP_LC:
                s[++t] = q;
                break;
            case OP_LI:
                s[t] = s[s[t]];
                break;
            case OP_INT:
                t += q;
                break;
            case OP_DCT:
                t-= q;
                break;
            case OP_J:
                pc = q - 1;
                break;
            case OP_FJ:
                if (!s[t]) pc = q - 1; --t;
                break;
            case OP_HLT:
                return;
            case OP_ST:
                s[s[t - 1]] = s[t]; t -= 2;
                break;
            case OP_CALL:
                s[t + 2] = b; s[t + 3] = pc;
                s[t + 4] = base(b, p); b = t + 1;
                pc = q - 1;
                break;
            case OP_EP:
                t = b - 1; pc = s[b + 2]; b = s[b + 1];
                break;
            case OP_EF:
                t = b; pc = s[b + 2]; b = s[b + 1];
                break;
            case OP_RC:
                s[s[t--]] = getchar();
                break;
            case OP_RI:
                s[s[t--]] = getint();
                break;
            case OP_WRC:
                putchar(s[t--]);
                break;
            case OP_WRI:
                putint(s[t--]);
                break;
            case OP_WLN:
                putchar('\n');
                break;
            case OP_ADD:
                --t; s[t] += s[t + 1];
                break;
            case OP_SUB:
                --t; s[t] -= s[t + 1];
                break;
            case OP_MUL:
                --t; s[t] *= s[t + 1];
                break;
            case OP_DIV:
                --t; s[t] /= s[t + 1];
                break;
            case OP_NEG:
                --t; s[t] = -s[t];
                break;
            case OP_CV:
                ++t; s[t] = s[t - 1];
                break;
            case OP_EQ:
                --t; s[t] = s[t] == s[t + 1];
                break;
            case OP_NE:
                --t; s[t] = s[t] != s[t + 1];
                break;
            case OP_GT:
                --t; s[t] = s[t] > s[t + 1];
                break;
            case OP_LT:
                --t; s[t] = s[t] < s[t + 1];
                break;
            case OP_GE:
                --t; s[t] = s[t] >= s[t + 1];
                break;
            case OP_LE:
                --t; s[t] = s[t] <= s[t + 1];
                break;
            default:
                assert(0);
        }
        ++pc;
        
        // printf("pc=%d t=%d b=%d stack=[", pc, t, b);
        // for (int i = 0; i <= t; ++i) printf("%d ", s[i]);
        // printf("]\n\n");    
    }
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        fprintf(stderr, "input file missing\n");
        return -1;
    }

    FILE *f = fopen(argv[1], "rt");
    read_asm(f);
    fclose(f);

    run();

    return 0;
}
