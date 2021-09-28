#include <stdlib.h>
#include <stdio.h>
#include "code.h"
#include "instruction.h"
#include "tables.h"
#include <string.h>

// ----------------------------------------------------------------------------
// Tables ---------------------------------------------------------------------

extern StrTable *st;
extern VarTable *vt;
// ----------------------------------------------------------------------------
extern char * yytext;
extern int yylineno;

extern StrTable *st;
extern VarTable *vt;

typedef union {
    int   as_int;
    float as_float;
} Word;

// ----------------------------------------------------------------------------


// Data stack -----------------------------------------------------------------

#define STACK_SIZE 100

Word stack[STACK_SIZE];
int sp; // stack pointer

// All these ops should have a boundary check, buuuut... X_X

void pushi(int x) {
    stack[++sp].as_int = x;
}

int popi() {
    return stack[sp--].as_int;
}

void pushf(float x) {
    stack[++sp].as_float = x;
}

float popf() {
    return stack[sp--].as_float;
}

void init_stack() {
    for (int i = 0; i < STACK_SIZE; i++) {
        stack[i].as_int = 0;
    }
    sp = -1;
}

void print_stack() {
    printf("*** STACK: ");
    for (int i = 0; i <= sp; i++) {
        printf("%d ", stack[i].as_int);
    }
    printf("\n");
}


// ----------------------------------------------------------------------------

// Variables memory -----------------------------------------------------------

#define MEM_SIZE 100

Word mem[MEM_SIZE];

void storei(int addr, int val) {
    mem[addr].as_int = val;
}

int loadi(int addr) {
    return mem[addr].as_int;
}

void storef(int addr, float val) {
    mem[addr].as_float = val;
}

float loadf(int addr) {
    return mem[addr].as_float;
}

void init_mem() {
    for (int addr = 0; addr < MEM_SIZE; addr++) {
        mem[addr].as_int = 0;
    }
}

// // ----------------------------------------------------------------------------

// // ----------------------------------------------------------------------------
// // Code memory ----------------------------------------------------------------

// Instr code[INSTR_MEM_SIZE];
// int next_instr;
#define MAX_STR_SIZE 128
static char str_buf[MAX_STR_SIZE];
#define clear_str_buf() str_buf[0] = '\0'


// ----------------------------------------------------------------------------
// AST Traversal --------------------------------------------------------------

int int_regs_count;
int float_regs_count;

#define new_int_reg() \
    int_regs_count++

#define new_float_reg() \
    float_regs_count++


void read_int(int var_idx) {
    int x;
    // printf("read (int): ");
    scanf("%d", &x);
    storei(var_idx, x);
}

void read_real(int var_idx) {
    float x;
    // printf("read (real): ");
    scanf("%f", &x);
    storef(var_idx, x);
}

void read_bool(int var_idx) {
    int x;
    do {
        // printf("read (bool - 0 = false, 1 = true): ");
        scanf("%d", &x);
    } while (x != 0 && x != 1);
    storei(var_idx, x);
}

void read_str(int var_idx) {
    printf("read (str): ");
    clear_str_buf();
    scanf("%s", str_buf);   // Did anyone say Buffer Overflow..? ;P
    storei(var_idx, add_string(st, str_buf));
}

void write_int() {
    printf("%d\n", popi());
}

void write_real() {
    printf("%f\n", popf());
}

void write_bool() {
    popi() == 0 ? printf("false\n") : printf("true\n");
}

// Helper function to write strings.
void escape_str(const char* s, char *n) {
    int i = 0, j = 0;
    char c;
    while ((c = s[i++]) != '\0') {
        if (c == '"') { continue; }
        else if (c == '\\' && s[i] == 'n') {
            n[j++] = '\n';
            i++;
        } else {
            n[j++] = c;
        }
    }
    n[j] = '\0';
}

void write_str() {
    int s = popi(); // String pointer
    clear_str_buf();
    escape_str(get_string(st, s), str_buf);
    printf(str_buf); // Weird language semantics, if printing a string, no new line.
}

void rec_emit_code(AST *ast);

// ----------------------------------------------------------------------------



void emit_assign(AST *ast) {
    int addr = get_data(get_child(ast, 0));
    
    AST *r = get_child(ast, 1);
    int index = 0;
    Type var_type = get_type(vt, addr);
    rec_emit_code(r);
    
    if (var_type == REAL_TYPE && (get_kind(r) != PLUS_NODE) && 
       (get_kind(r) != MOD_NODE) && (get_kind(r) != TIMES_NODE)
       && (get_kind(r) != OVER_NODE) && (get_kind(r) != MINUS_NODE)) {

        if (get_child_count(get_child(ast,0)) == 1) {
            if (get_kind(get_child(get_child(ast,0),0)) == EXPR_LIST_NODE) {
                rec_emit_code(get_child(get_child(ast, 0),0));
                index = popi();
                printf(".data\ntemp%d:\t.float\t%f\n", get_data(r) + 4, popf());
                printf(".text\nlwc1\t$f0, temp%d\n", get_data(r) + 4);
                printf("swc1\t$f0, %s + %d\n", get_name(vt, addr), index * 4);
            }
        } else {
            printf(".data\ntemp%d:\t.float\t%f\n", get_data(r) + 4, popf());
            printf(".text\nlwc1\t$f0, temp%d\n", get_data(r) + 4);
            printf("swc1\t$f0, %s\n", get_name(vt, addr));
        }
    } else if (var_type == INT_TYPE && (get_kind(r) != PLUS_NODE) && 
       (get_kind(r) != MOD_NODE) && (get_kind(r) != TIMES_NODE)
       && (get_kind(r) != OVER_NODE) && (get_kind(r) != MINUS_NODE)) {
        if (get_child_count(get_child(ast,0)) == 1) {
            if (get_kind(get_child(get_child(ast,0),0)) == EXPR_LIST_NODE) {
                rec_emit_code(get_child(get_child(ast, 0),0));
                index = popi();
                printf(".text\nli\t$t0, %d\n", popi());
                printf("sw\t$t0, %s + %d\n", get_name(vt, addr), index * 4);
            }
        } else {
            printf(".text\nli\t$t%d, %d\n", get_data(ast), popi());
            printf("sw\t$t%d, %s\n", get_data(ast), get_name(vt, addr));
        }

    // } else if  (var_type == STR_TYPE) { // All other types, include ints, bools and strs.
    //     int s = popi(); // String pointer
    //     clear_str_buf();
    //     escape_str(get_string(st, s), str_buf);
    //     printf(".data\n%s:\t.asciiz\t", get_name(vt, addr));
    //     printf("\"%s\"\n", str_buf);
    } else if (var_type == REAL_TYPE && ((get_kind(r) == PLUS_NODE) || 
       (get_kind(r) == MOD_NODE) || (get_kind(r) == TIMES_NODE)
       || (get_kind(r) == OVER_NODE) || (get_kind(r) == MINUS_NODE))) {
        printf("swc1\t$f12, %s\n", get_name(vt, addr));
    } else if (var_type == INT_TYPE && ((get_kind(r) == PLUS_NODE) || 
       (get_kind(r) == MOD_NODE) || (get_kind(r) == TIMES_NODE)
       || (get_kind(r) == OVER_NODE) || (get_kind(r) == MINUS_NODE))) {
        printf("sw\t$t%d, %s\n", get_data(ast), get_name(vt, addr));
    } 
}

void emit_block(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
}

void emit_stmt_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
}

void emit_expr_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
}

void emit_init_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
}

void emit_arg_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
}

void emit_func_list(AST *ast) {
    int addr = get_data(get_child(ast, 0));
    printf("%s:\n", get_name(vt, addr));
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));    
    }
}

void emit_return(AST *ast) {
    rec_emit_code(get_child(ast, 0)); // Emit code for test.
}

void emit_int_val(AST *ast) {
    pushi(get_data(ast));
}

void emit_program(AST *ast) {
    printf(".globl main\n");
    rec_emit_code(get_child(ast, 0)); // var_list
}

void emit_real_val(AST *ast) {
    pushf(get_float_data(ast));
}

void emit_str_val(AST *ast) {
    pushi(get_data(ast));
}

void emit_var_decl(AST *ast) {
    rec_emit_code(get_child(ast, 0));
    // printf("%d",popi());
}

void emit_var_list(AST *ast) {
    ast = get_child(ast,0);
    int addr;
    int size = get_child_count(ast);

    for (int i = 0; i < size; i++) {
        AST *r = get_child(ast, i);
        // rec_emit_code(r);
        if (get_child_count(r) == 1) {
            addr = get_data(r);
            Type var_type = get_type(vt, addr);
            
            if (var_type == REAL_TYPE && (get_kind(r) != PLUS_NODE) &&      
            (get_kind(r) != MOD_NODE) && (get_kind(r) != TIMES_NODE)
            && (get_kind(r) != OVER_NODE) && (get_kind(r) != MINUS_NODE)) {
                printf(".data\n%s:\t.float\t0.0\n", get_name(vt, addr));
            } else if (var_type == INT_TYPE && (get_kind(r) != PLUS_NODE) && 
            (get_kind(r) != MOD_NODE) && (get_kind(r) != TIMES_NODE)
            && (get_kind(r) != OVER_NODE) && (get_kind(r) != MINUS_NODE)) {
                if (get_kind(get_child(get_child(ast,0),0)) == INT_VAL_NODE)
                {
                    rec_emit_code(get_child(get_child(ast,0),0));
                    printf(".data\n%s:\t.word\t", get_name(vt, addr));
                    int sizeVet = popi();
                    for (int i=0; i < sizeVet; i++) {
                        printf("0");
                        if (i+1 == sizeVet) {
                            printf("\n");
                        } else {
                            printf(", ");
                        }
                    }
                } else {
                    printf(".data\n%s:\t.word\t0\n", get_name(vt, addr));
                }
            }
        } else {
            addr = get_data(r);
            Type var_type = get_type(vt, addr);
            
            if (var_type == REAL_TYPE && (get_kind(r) != PLUS_NODE) && 
            (get_kind(r) != MOD_NODE) && (get_kind(r) != TIMES_NODE)
            && (get_kind(r) != OVER_NODE) && (get_kind(r) != MINUS_NODE)) {
                // if (get_kind(get_child(get_child(ast,0),0)) == EXPR_LIST_NODE)
                // {
                //     printf("sadsad");
                //     printf(".data\n%s:\t.float\t", get_name(vt, addr));
                // }
                printf(".data\n%s:\t.float\t", get_name(vt, addr));
                printf("%f\n", popf());
            } else if (var_type == INT_TYPE && (get_kind(r) != PLUS_NODE) && 
            (get_kind(r) != MOD_NODE) && (get_kind(r) != TIMES_NODE)
            && (get_kind(r) != OVER_NODE) && (get_kind(r) != MINUS_NODE)) {
                printf(".data\n%s:\t.word\t", get_name(vt, addr));
                printf("%d\n", popi());
            } else if  (var_type == STR_TYPE) { // All other types, include ints, bools and strs.
                int s = popi(); // String pointer
                clear_str_buf();
                escape_str(get_string(st, s), str_buf);
                printf(".data\n%s:\t.asciiz\t", get_name(vt, addr));
                printf("\"%s\"\n", str_buf);
            }
            // rec_emit_code(r);
        }
    }
}

void emit_var_use(AST *ast) {
    rec_emit_code(get_child(ast, 0));
}

void emit_b2i(AST* ast) {
    rec_emit_code(get_child(ast, 0));
}

void emit_b2s(AST* ast) {
    rec_emit_code(get_child(ast, 0));
}

void emit_i2s(AST* ast) {
    rec_emit_code(get_child(ast, 0));
}

void emit_r2s(AST* ast) {
    rec_emit_code(get_child(ast, 0));
}

void emit_minus(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == REAL_TYPE) {
        if (check_float(l) != NULL) {
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, REAL_TYPE, get_data(ast) + 1);
                }
            } else {       
                printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));
            }
        } 
        else if (check_float(l) == NULL) {
            rec_emit_code(l);
            printf(".data\nft%d:\t.float  %f\n", get_data(r) + 9, popf());
            printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 1, get_data(r) + 9);
        }
        if (get_node_type(r) == REAL_TYPE) {
            if (check_float(r) != NULL) {
                if (get_child_count(r) == 1) {
                    if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                        emit_def_vet(r, vt, addr_r, REAL_TYPE, get_data(ast) + 2);
                    }
                } else {
                    printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
                }
            } else if (check_float(r) == NULL) {
                rec_emit_code(r);
                printf(".data\nft%d:\t.float  %f\n", get_data(r), popf());
                printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 2, get_data(r));
            }
            printf("sub.s\t$f%d, $f%d, $f%d\n", get_data(ast), get_data(ast) + 1, get_data(ast) + 2);
            printf("mov.s\t$f12, $f%d\n", get_data(ast)); 
        }
    } else if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL) {
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, INT_TYPE, get_data(ast) + 1);
                }
            } else {       
                printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
            }
        }
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
    }
    if (get_kind(r) == B2R_NODE) { 
        rec_emit_code(r);
        
        printf("sub\t$t0, $t%d, $t%d\n", get_data(ast) + 1, get_data(ast) + 2); 
    }
    // AST *l = get_child(ast, 0);
    // AST *r = get_child(ast, 1);
    // int addr_l, addr_r;
    // addr_l = get_data(l);
    // addr_r = get_data(r);
    // if (get_node_type(l) == INT_TYPE) { 
    //     if (check_int(l) != NULL)
    //         printf(".text\nlw\t$t%d, %s\n", loadi(addr_l), get_name(vt, addr_l));  
    //     else if (check_int(l) == NULL) {
    //         rec_emit_code(l);
    //         printf(".text\nli\t$t%d, %d\n", loadi(addr_l), popi());
    //     }
    // } else if (get_node_type(l) == REAL_TYPE) { 
    //     if (check_float(l) != NULL)
    //         printf(".text\nlwc1\t$f%.0f, %s\n", loadf(addr_l), get_name(vt, addr_l));  
    //     else if (check_float(l) == NULL) {
    //         rec_emit_code(l);
    //         printf(".data\nft1:\t.float  %f\n", popf());
    //         printf(".text\nlwc1\t$%.0f, ft1\n", loadf(addr_r));
    //     }
    // }
    // if (get_kind(r) == B2R_NODE) { 
    //     rec_emit_code(r);
        
    //     printf("sub\t$t1, $t%d, $t%d\n", loadi(addr_r), loadi(addr_r) + 1); 
    // } else if (get_kind(r) != B2R_NODE) {
    //     if (get_node_type(r) == REAL_TYPE) { 
    //         if (check_float(r) == NULL && get_node_type(l) == INT_TYPE) {
    //             rec_emit_code(r);
    //             // printf(".text\nli\t$t%d, %f\n", loadi(addr_l), popf());
    //             printf(".data\nft1:\t.float  %f\n", popf());
    //             printf(".text\nlwc1\t$t%d, ft1\n", loadi(addr_l) + 1);
    //             printf("sub\t$t1, $t%d, $t%d\n", loadi(addr_l), loadi(addr_l) + 1); 
    //         } else if (check_float(r) == NULL && get_node_type(l) == REAL_TYPE) {
    //             rec_emit_code(r);
    //             printf(".data\nft1:\t.float  %f\n", popf());
    //             printf(".text\nlwc1\t$f1, ft1\n");
    //             printf("sub.s\t$f1, $f%.0f, $f%.0f\n", loadf(addr_r), loadf(addr_r) + 1); 
    //         } 
    //         else if (check_float(r) != NULL)
    //             printf(".text\nlwc1\t$f%.0f, %s\n", loadf(addr_r) + 1, get_name(vt, addr_r));
    //             printf("sub.s\t$f1, $f%.0f, $f%.0f\n", loadf(addr_r), loadf(addr_r) + 1);   
    //     }
    // }
}


void emit_over(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL) {
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, INT_TYPE, get_data(ast) + 1);
                }
            } else {
                printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
            }
        } else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL) {
                if (get_child_count(r) == 1) {
                    if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                        emit_def_vet(r, vt, addr_r, INT_TYPE, get_data(ast) + 2);
                    }
                } else {
                    printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
                }
            } else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    } else if (get_node_type(l) == REAL_TYPE) { 
        if (check_float(l) != NULL) {
            if (get_child_count(l) == 1) { 
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, REAL_TYPE, get_data(ast) + 1);
                }
            } else {
                printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));
            }
        } else if (check_float(l) == NULL) {
            rec_emit_code(l);
            printf(".data\nft%d:\t.float  %f\n", get_data(r) + 9, popf());
            printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 1, get_data(r) + 9);  
        }
        if (get_node_type(r) == REAL_TYPE) { 
            if (check_float(r) != NULL) {
                if (get_child_count(r) == 1) {
                    if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                        emit_def_vet(r, vt, addr_r, REAL_TYPE, get_data(ast) + 2);
                    }
                } else {
                    printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));
                }
            }
            else if (check_float(r) == NULL) {
                rec_emit_code(r);
                printf(".data\nft%d:\t.float  %f\n", get_data(r), popf());
                printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 2, get_data(r));
            }
            printf("div.s\t$f%d, $f%d, $f%d\n", get_data(ast), get_data(ast) + 1, get_data(ast) + 2);
            printf("mov.s\t$f12, $f%d\n", get_data(ast));
        }
    }
    if ((get_node_type(l) == INT_TYPE) && (get_node_type(r) == INT_TYPE)) {
        printf("div\t$t%d, $t%d\n", get_data(ast) + 1, get_data(ast) + 2); 
        printf("mflo\t$t%d\n", get_data(ast));
    }   
}

void emit_mod(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL) {
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, INT_TYPE, get_data(ast) + 1);
                }
            } else {
                printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l)); 
            }
        }
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL) {
                if (get_child_count(r) == 1) {
                    if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                        emit_def_vet(r, vt, addr_r, INT_TYPE, get_data(ast) + 2);
                    }
                } else {
                    printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
                }
            } else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    } else if (get_node_type(l) == REAL_TYPE) { 
        if (check_float(l) != NULL) {
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, REAL_TYPE, get_data(ast) + 1);
                }
            } else {
                printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));
            }
        } else if (check_float(l) == NULL) {
            rec_emit_code(l);
            printf(".data\nft%d:\t.float  %f\n", get_data(r) + 9, popf());
            printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 1, get_data(r) + 9);  
        }
        if (get_node_type(r) == REAL_TYPE) { 
            if (check_float(r) != NULL) {
                if (get_child_count(r) == 1) {
                    if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                        emit_def_vet(r, vt, addr_r, REAL_TYPE, get_data(ast) + 2);
                    }
                } else {
                    printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));
                }
            }
            else if (check_float(r) == NULL) {
                rec_emit_code(r);
                printf(".data\nft%d:\t.float  %f\n", get_data(r), popf());
                printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 2, get_data(r));
            }
            printf("div.s\t$f%d, $f%d, $f%d\n", get_data(ast), get_data(ast) + 1, get_data(ast) + 2);
            printf("mov.s\t$f12, $f%d\n", get_data(ast));
        }
    }
    if ((get_node_type(l) == INT_TYPE) && (get_node_type(r) == INT_TYPE)) {
        printf("div\t$t%d, $t%d\n", get_data(ast) + 1, get_data(ast) + 2); 
        printf("mfhi\t$t%d\n", get_data(ast));
    }   
}

void emit_plus(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == REAL_TYPE) {
        if (check_float(l) != NULL)
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, REAL_TYPE, get_data(ast));
                }
            } else {
                printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
            }
        else if (check_float(l) == NULL) {
            rec_emit_code(l);
            printf(".data\nft%d:\t.float  %f\n", get_data(r) + 9, popf());
            printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 1, get_data(r) + 9);
        }
        if (get_node_type(r) == REAL_TYPE) {
            if (check_float(r) != NULL)
            if (get_child_count(r) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, REAL_TYPE, get_data(ast));
                }
            } else {
                printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
            }
            else if (check_float(r) == NULL) {
                rec_emit_code(r);
                printf(".data\nft%d:\t.float  %f\n", get_data(r), popf());
                printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 2, get_data(r));
            }
            printf("add.s\t$f%d, $f%d, $f%d\n", get_data(ast), get_data(ast) + 1, get_data(ast) + 2); 
            printf("mov.s\t$f12, $f%d\n", get_data(ast));
        }
    } else if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL) {
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, INT_TYPE, get_data(ast));
                }
            } else {
                printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));
            }
        }
              
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
 
    }
    if (get_kind(r) == B2R_NODE) { 
        rec_emit_code(r);
        
        printf("add\t$t0, $t%d, $t%d\n", get_data(ast) + 1, get_data(ast) + 2); 
    }
}

void emit_def_vet(AST *ast, VarTable *vt, int addr, Type tp, int aux) {
    if (tp == INT_TYPE) {
        int index = 0;
        // printf(".text\nla\t$s0, %s\n", get_name(vt, addr));
        rec_emit_code(get_child(ast,0));
        index = popi();
        printf(".text\nlw\t$t%d, %s + %d\n", aux, get_name(vt, addr), index * 4);
        // printf("lw\t$t%d, %d ( $s0 )\n", aux, index * 4);
    } else if (tp == REAL_TYPE) {
        int index = 0;
        // printf(".text\nla\t$s0, %s\n", get_name(vt, addr));
        rec_emit_code(get_child(ast,0));
        index = popi();
        printf(".text\nlwc1\t$f%d, %s + %d\n", aux, get_name(vt, addr), index * 4);
        // printf("lwc1\t$f%d, %d ( $s0 )\n", aux, index * 4);
    }
}

void emit_times(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL) {
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(l,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(l, vt, addr_l, INT_TYPE, get_data(ast) + 1);
                }
            } else {
                printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
            }
        }
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL) {
                if (get_child_count(r) == 1) {
                    if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                        emit_def_vet(r, vt, addr_r, INT_TYPE, get_data(ast) + 2);
                    }
                } else {
                    printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
                }
            }
            else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    } else if (get_node_type(l) == REAL_TYPE) { 
        if (check_float(l) != NULL) {
            if (get_child_count(l) == 1) {
                if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(r, vt, addr_r, REAL_TYPE, get_data(ast) + 1);
                }
            } else {
                printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));
            }
        } else if (check_float(l) == NULL) {
            rec_emit_code(l);
            printf(".data\nft%d:\t.float\t%f\n", get_data(r) + 9, popf());
            printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 1, get_data(r) + 9);      
        }
        if (get_node_type(r) == REAL_TYPE) { 
            if (check_float(r) != NULL){
                if (get_child_count(r) == 1) {
                    if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                        emit_def_vet(r, vt, addr_r, REAL_TYPE, get_data(ast) + 2);
                    }
                } else {
                    printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));
                }
            } else if (check_float(r) == NULL) {
                rec_emit_code(r);
                printf(".data\nft%d:\t.float\t%f\n", get_data(r), popf());
                printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast), get_data(r));
            }
            printf("mul.s\t$f%d, $f%d, $f%d\n", get_data(ast), get_data(ast) + 1, get_data(ast) + 2);
            printf("mov.s\t$f12, $f%d\n", get_data(ast));
        }
    }
    if ((get_node_type(l) == INT_TYPE) && (get_node_type(r) == INT_TYPE)) {
        printf("mul\t$t%d, $t%d, $t%d\n", get_data(ast), get_data(ast) + 1, get_data(ast) + 2); 
    }  
}


void emit_printf(AST *ast) 
{
    
    int size = get_child_count(ast);
    if(size == 2) 
    {
        
        AST *expr = get_child(ast, 0);

        Type expr_type = get_node_type(expr);
        if (expr_type == STR_TYPE)
        {
            int s = get_data(ast); // String pointer
            clear_str_buf();
            escape_str(get_string(st, s), str_buf);
            if(strstr(str_buf, "%s") != NULL)
            {
                AST *expr2 = get_child(ast, 1);
                Type expr_type2 = get_node_type(expr2);
                int addr_l = get_data(expr2);
                
                if (expr_type2 == STR_TYPE)
                {
                    printf(".text\nlw\t$t0, %s\n", get_name(vt, addr_l)); 
                    printf("move\t$a0, $t0\n");
                    printf("li\t$v0, 4\n");
                }    
            }
            if(strstr(str_buf, "%d") != NULL)
            {
                
                AST *expr2 = get_child(ast, 1);
                Type expr_type2 = get_node_type(expr2);
                int addr_l = get_data(expr2);
                 
                if (expr_type2 == INT_TYPE)
                {
                    if (check_int(expr2) != NULL) {
                        printf(".text\nlw\t$t0, %s\n", get_name(vt, addr_l)); 
                    }
                    else if (check_int(expr2) == NULL) {
                        rec_emit_code(expr2);
                        printf(".text\nmove\t$t0, $t1\n");
                    }
                    printf("move\t$a0, $t0\n");
                    printf("li\t$v0, 1\n");
                } else if (expr_type2 == BOOL_TYPE)
                {
                    printf("move\t$a0, $t1\n");
                    printf("li\t$v0, 1\n");
                }    
            }
            if(strstr(str_buf, "%f") != NULL)
            {
                AST *expr2 = get_child(ast, 1);
                Type expr_type2 = get_node_type(expr2);
                int addr_l = get_data(expr2);
                if (expr_type2 == REAL_TYPE)
                {
                    if (check_float(expr2) != NULL)
                        printf(".text\nlwc1\t$f12, %s\n", get_name(vt, addr_l)); 
                    else if (check_float(expr2) == NULL) {
                        rec_emit_code(expr2);
                        printf(".text\nmove\t$f12, $f1\n");
                    }
                    printf("li\t$v0, 2\n");
                }    
            }
        }
    }
    else if(size == 1)  {
        int s = get_data(get_child(ast, 0)); // String pointer
        clear_str_buf();
        escape_str(get_string(st, s), str_buf);
        
        printf(".data\nmsg%d:\t.asciiz\t\"%s\"\n", get_data(get_child(ast,0)), str_buf); 
        printf(".text\nla\t$a0, msg%d\n", get_data(get_child(ast,0))); 
        printf("li\t$v0, 4\n");
    }
    printf("syscall\n");
}

void emit_scanf(AST *ast) {
    int size = get_child_count(ast);
    if(size == 2) 
    {
        AST *expr = get_child(ast, 0);

        Type expr_type = get_node_type(expr);
        
        if (expr_type == STR_TYPE)
        {
            int s = get_data(ast); // String pointer
            clear_str_buf();
            escape_str(get_string(st, s), str_buf);
            if(strstr(str_buf, "%s") != NULL)
            {
                AST *expr2 = get_child(ast, 1);
                Type expr_type2 = get_node_type(expr2);
                int addr_l = get_data(expr2);
                
                if (expr_type2 == STR_TYPE)
                {
                    printf(".data\nmsg%d:\t.asciiz\t\"%s\"\n", get_data(get_child(ast,0)), str_buf);
                    printf("li\t$v0, 8\n");
                    printf(".text\nsw\t$v0, msg%d\n", get_data(get_child(ast,0))); 
                }    
            }
            if(strstr(str_buf, "%d") != NULL)
            {
                AST *expr2 = get_child(ast, 1);
                Type expr_type2 = get_node_type(expr2);
                int addr_l = get_data(expr2);
                int s = get_data(ast); // String pointer
                clear_str_buf();
                escape_str(get_string(st, s), str_buf); 
                if (expr_type2 == INT_TYPE)
                {
                    // if (check_int(expr2) != NULL)
                    //     printf(".data\n%s:\t.word\t0\n", get_name(vt, addr_l)); 
                    printf(".text\nli\t$v0, 5\nsyscall\n");
                    printf("sw\t$v0, %s\n", get_name(vt, addr_l));
                } else if (expr_type2 == BOOL_TYPE)
                {
                    printf("move\t$a0, $t1\n");
                    printf("li\t$v0, 5\nsyscall\n");
                }    
            }
            if(strstr(str_buf, "%f") != NULL)
            {
                AST *expr2 = get_child(ast, 1);
                Type expr_type2 = get_node_type(expr2);
                int addr_l = get_data(expr2);
                int s = get_data(ast); // String pointer
                clear_str_buf();
                escape_str(get_string(st, s), str_buf); 

                if (expr_type2 == REAL_TYPE)
                {
                    // if (check_float(expr2) != NULL)
                    //     printf(".data\n%s:\t.float\t0.0\n", get_name(vt, addr_l));
                    printf(".text\nli\t$v0, 6\nsyscall\n");
                    printf("swc1\t$f0, %s\n", get_name(vt, addr_l));
                }    
            }
        }
    }
}

void emit_b2r(AST* ast) {
  
    AST *r = get_child(ast, 0);
    int addr_r = get_data(r);
    if (get_node_type(r) == INT_TYPE) { 
        if (check_int(r) != NULL) {
            if (get_child_count(r) == 1) {
                if (get_kind(get_child(r,0)) == EXPR_LIST_NODE) {
                    emit_def_vet(r, vt, addr_r, INT_TYPE, get_data(ast) + 2);
                }
            } else {
                printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
            }
        }
        else if (check_int(r) == NULL) {
            rec_emit_code(get_child(ast, 0));
            printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
        }
    }
}



void emit_i2r(AST* ast) {
    rec_emit_code(get_child(ast, 0));
    AST *r = get_child(ast, 0);
    int addr_r = get_data(r);

    if (get_node_type(r) == INT_TYPE) { 
        if (check_int(r) != NULL) {
            printf("lwc1\t$f3, %s\n", get_name(vt, addr_r));  
            printf("add.s\t$f1, $f2, $f3\n");
        }
        else if (check_int(r) == NULL) {
            printf(".data\nft1:\t.float  %d\n", get_int_data(r));
            printf(".text\nlwc1\t$f3, ft1\n");
            printf("add.s\t$f1, $f2, $f3\n");
        }
    } 
}



void emit_eq(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL)
            printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL)
                printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
            else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    }// else if (get_node_type(l) == REAL_TYPE) { 
    //     if (check_float(l) != NULL) {
    //         printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));
    //     } else if (check_float(l) == NULL) {
    //         rec_emit_code(l);
    //         printf(".data\nft%d:\t.float\t%f\n", get_data(ast), popf());
    //         printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast) + 1, get_data(ast));      
    //     }
    //     if (get_node_type(r) == REAL_TYPE) { 
    //         if (check_float(r) != NULL)
    //             printf(".text\nlwc1\t$f%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));
    //         else if (check_float(r) == NULL) {
    //             rec_emit_code(r);
    //             printf(".data\nft%d:\t.float\t%f\n", get_data(ast) + 1, popf());
    //             printf(".text\nlwc1\t$f%d, ft%d\n", get_data(ast), get_data(ast) + 1);
    //         }
    //         printf("mul.s\t$f%d, $f%d, $f%d\n", get_data(ast), get_data(ast) + 1, get_data(ast) + 2);
    //         printf("mov.s\t$f12, $f%d\n", get_data(ast));
    //     }
    // }
    // if ((get_node_type(l) == INT_TYPE) && (get_node_type(r) == INT_TYPE)) {
    //     printf("seq\t$t%d, $t%d, $t%d\n", get_data(ast), get_data(ast) + 1, get_data(ast) + 2); 
    // }  
}

void emit_repeat(AST *ast) {
    int size = get_child_count(ast);
    if (size == 2) {
        AST *l = get_child(ast, 0);
        AST *r = get_child(ast, 1);
        int addr_l, addr_r, addr;
        addr_l = get_data(l);
        addr_r = get_data(r);
        if (get_kind(l) == EQ_NODE) {
            printf("loop:\n");
            rec_emit_code(l);
            printf("beq\t$t%d, $t%d, while\njal\texit\nwhile:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tloop\n");
            printf("exit:\n");
        } else if (get_kind(l) == NE_NODE) {
            printf("loop:\n");
            rec_emit_code(l);
            printf("bne\t$t%d, $t%d, while\njal\texit\nwhile:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tloop\n");
            printf("exit:\n");
        } else if (get_kind(l) == GE_NODE) {
            printf("loop:\n");
            rec_emit_code(l);
            printf("bge\t$t%d, $t%d, while\njal\texit\nwhile:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tloop\n");
            printf("exit:\n");
        } else if (get_kind(l) == LT_NODE) {
            printf("loop:\n");
            rec_emit_code(l);
            printf("blt\t$t%d, $t%d, while\njal\texit\nwhile:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tloop\n");
            printf("exit:\n");
        } else if (get_kind(l) == GT_NODE) {
            printf("loop:\n");
            rec_emit_code(l);
            printf("bgt\t$t%d, $t%d, while\njal\texit\nwhile:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tloop\n");
            printf("exit:\n");
        } else if (get_kind(l) == LE_NODE) {
            printf("loop:\n");
            rec_emit_code(l);
            printf("ble\t$t%d, $t%d, while\njal\texit\nwhile:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tloop\n");
            printf("exit:\n");
        } else if (get_kind(l) == AND_NODE) {
            rec_emit_code(l);
        }
    } 
}

void emit_if(AST *ast) {
    int size = get_child_count(ast);
    if (size == 2) {
        AST *l = get_child(ast, 0);
        AST *r = get_child(ast, 1);
        int addr_l, addr_r, addr;
        addr_l = get_data(l);
        addr_r = get_data(r);
        if (get_kind(l) == EQ_NODE) {
            rec_emit_code(l);
            printf("beq\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("else:\n");
        } else if (get_kind(l) == NE_NODE) {
            rec_emit_code(l);
            printf("bne\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("else:\n");
        } else if (get_kind(l) == GE_NODE) {
            rec_emit_code(l);
            printf("bge\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("else:\n");
        } else if (get_kind(l) == LT_NODE) {
            rec_emit_code(l);
            printf("blt\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("else:\n");
        } else if (get_kind(l) == GT_NODE) {
            rec_emit_code(l);
            printf("bgt\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("else:\n");
        } else if (get_kind(l) == LE_NODE) {
            rec_emit_code(l);
            printf("ble\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("else:\n");
        }
    } else if (size == 3) {
        AST *l = get_child(ast, 0);
        AST *r = get_child(ast, 1);
        AST *r2 = get_child(ast, 2);
        int addr_l, addr_r, addr_r2;
        addr_l = get_data(l);
        addr_r = get_data(r);
        addr_r2 = get_data(r2);
        if (get_kind(l) == EQ_NODE) {
            rec_emit_code(l);
            printf("beq\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tout\nelse:\n");
            rec_emit_code(r2);
            printf("out:\n");
        } else if (get_kind(l) == NE_NODE) {
            rec_emit_code(l);
            printf("bne\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tout\nelse:\n");
            rec_emit_code(r2);
            printf("out:\n");
        } else if (get_kind(l) == GE_NODE) {
            rec_emit_code(l);
            printf("bge\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tout\nelse:\n");
            rec_emit_code(r2);
            printf("out:\n");
        } else if (get_kind(l) == LT_NODE) {
            rec_emit_code(l);
            printf("blt\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tout\nelse:\n");
            rec_emit_code(r2);
            printf("out:\n");
        } else if (get_kind(l) == GT_NODE) {
            rec_emit_code(l);
            printf("bgt\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tout\nelse:\n");
            rec_emit_code(r2);
            printf("out:\n");
        } else if (get_kind(l) == LE_NODE) {
            rec_emit_code(l);
            printf("ble\t$t%d, $t%d, if\njal\telse\nif:\n", get_data(ast) + 1, get_data(ast) + 2);
            rec_emit_code(r);
            printf("jal\tout\nelse:\n");
            rec_emit_code(r2);
            printf("out:\n");
        }
    }  
}

void emit_neq(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL)
            printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL)
                printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
            else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    }
}

void emit_le(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL)
            printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL)
                printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
            else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    }
}

void emit_ge(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL)
            printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL)
                printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
            else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    }
}

void emit_gt(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL)
            printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL)
                printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
            else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    }
}

// int emit_bool_val(AST *ast) {
//     int x = new_int_reg();
//     int c = get_data(ast);
//     emit2(LDIi, x, c);
//     return x;
// }


void emit_lt(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    if (get_node_type(l) == INT_TYPE) { 
        if (check_int(l) != NULL)
            printf(".text\nlw\t$t%d, %s\n", get_data(ast) + 1, get_name(vt, addr_l));  
        else if (check_int(l) == NULL) {
            rec_emit_code(l);
            printf(".text\nli\t$t%d, %d\n", get_data(ast) + 1, popi());
        }
        if (get_node_type(r) == INT_TYPE) { 
            if (check_int(r) != NULL)
                printf("lw\t$t%d, %s\n", get_data(ast) + 2, get_name(vt, addr_r));  
            else if (check_int(r) == NULL) {
                rec_emit_code(r);
                printf("li\t$t%d, %d\n", get_data(ast) + 2, popi());
            }
        }
    }
}

void emit_and(AST *ast){
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int addr_l, addr_r;
    addr_l = get_data(l);
    addr_r = get_data(r);
    rec_emit_code(l);
    rec_emit_code(r);

}

// int emit_or(AST *ast){
//     AST *l = get_child(ast, 0);
//     AST *r = get_child(ast, 1);
//     int y = rec_emit_code(l);
//     int z = rec_emit_code(r);
//     int x = new_int_reg();
//     emit3(OROR, x, z, y);

//     return x;
// }




// int emit_pre_inc(AST *ast) {
//     int x;
//     int y = rec_emit_code(get_child(ast, 0));
//     int addr = get_data(get_child(ast, 0));
//     if (get_node_type(ast) == REAL_TYPE) {
//         x = new_float_reg();
//         emit2(INCf, x, y);
//         emit2(STWf, addr, x);
//     } else {
//         x = new_int_reg();
//         emit2(INCi, x, y);
//         emit2(STWi, addr, x);
//     }
//     return x;
// }

// int emit_pre_dec(AST *ast) {
//     int x;
//     int y = rec_emit_code(get_child(ast, 0));
//     int addr = get_data(get_child(ast, 0));
//     if (get_node_type(ast) == REAL_TYPE) {
//         x = new_float_reg();
//         emit2(DECf, x, y);
//         emit2(STWf, addr, x);
//     } else {
//         x = new_int_reg();
//         emit2(DECi, x, y);
//         emit2(STWi, addr, x);
//     }
//     return x;
// }

// int emit_var_use(AST *ast) {
//     int addr = get_data(ast);
//     int x;
//     if (get_node_type(ast) == REAL_TYPE) {
//         x = new_float_reg();
//         emit2(LDWf, x, addr);
//     } else {
//         x = new_int_reg();
//         emit2(LDWi, x, addr);
//     }
//     return x;
// }

// ----------------------------------------------------------------------------

void rec_emit_code(AST *ast) {
    switch(get_kind(ast)) {
        case ASSIGN_NODE:	     emit_assign(ast);              break;    
        case BLOCK_NODE:	     emit_block(ast);               break;          
        case INT_VAL_NODE:		 emit_int_val(ast);             break;    
        case PLUS_NODE:			 emit_plus(ast);                break;    
        case TIMES_NODE:		 emit_times(ast);               break;
        case MINUS_NODE:		 emit_minus(ast);               break;
        case OVER_NODE:			 emit_over(ast);                break;
        case MOD_NODE:			 emit_mod(ast);                 break;
        case PROGRAM_NODE:		 emit_program(ast);             break;     
        case REAL_VAL_NODE:		 emit_real_val(ast);            break;    
        case STR_VAL_NODE:		 emit_str_val(ast);             break;    
        case VAR_DECL_NODE:		 emit_var_decl(ast);            break;    
        case VAR_LIST_NODE:		 emit_var_list(ast);            break;    
        case ARG_LIST_NODE:		 emit_arg_list(ast);            break;    
        case EXPR_LIST_NODE:	 emit_expr_list(ast);           break;    
        case STMT_LIST_NODE:	 emit_stmt_list(ast);           break;    
        case FUNC_LIST_NODE:	 emit_func_list(ast);           break;    
        case VAR_USE_NODE:		 emit_var_use(ast);             break;    
        case INIT_LIST_NODE:	 emit_init_list(ast);           break;    
        case B2I_NODE:			 emit_b2i(ast);                 break;    
        case B2S_NODE:			 emit_b2s(ast);                 break;       
        case I2S_NODE:			 emit_i2s(ast);                 break;    
        case R2S_NODE:			 emit_r2s(ast);                 break;    
        case PRINTF_NODE:        emit_printf(ast);              break;    
        case IF_NODE:			 emit_if(ast);                  break; 
        case LT_NODE:			 emit_lt(ast);                  break;
        case GT_NODE:			 emit_gt(ast);                  break;
        case LE_NODE:			 emit_le(ast);                  break;
        case GE_NODE:			 emit_ge(ast);                  break;
        case EQ_NODE:			 emit_eq(ast);                  break;      
        case NE_NODE:			 emit_neq(ast);                 break;     
        case REPEAT_NODE:		 emit_repeat(ast);              break;
        // case READ_NODE:			return emit_read(ast);  
        case AND_NODE:			 emit_and(ast);                 break;
        // case OR_NODE:			return emit_or(ast);          
        // case POST_INC_NODE:     return emit_post_inc(ast);  
        // case PRE_INC_NODE:		return emit_pre_inc(ast); 
        // case POST_DEC_NODE:     return emit_post_dec(ast);  
        // case PRE_DEC_NODE:		return emit_pre_dec(ast); 
        // case REF_NODE:			return emit_ref(ast);     
        // case DEREF_NODE:		return emit_dref(ast);        
        // case UPLUS_NODE:	    return emit_uplus(ast);       
        // case UMINUS_NODE:		return emit_uminus(ast);      
        // case LNEG_NODE:			return emit_lneg(ast);      
        case SCANF_NODE:         emit_scanf(ast);               break;
        // case WRITE_NODE:		return emit_write(ast);       
        case RETURN_NODE:		 emit_return(ast);              break;    
        case B2R_NODE:			 emit_b2r(ast);                 break;    
        case I2R_NODE:			 emit_i2r(ast);                 break;    
                 
        default:
            fprintf(stderr, "Invalid kind: %s!\n", kind2str(get_kind(ast)));
            exit(EXIT_FAILURE);
    }
}

// ----------------------------------------------------------------------------

void emit_code(AST *ast) {
    // next_instr = 0;
    // int_regs_count = 0;
    // float_regs_count = 0;
    init_stack();
    init_mem();
    rec_emit_code(ast);
    // dump_program();
}
