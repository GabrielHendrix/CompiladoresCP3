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

// ----------------------------------------------------------------------------
// Code memory ----------------------------------------------------------------

Instr code[INSTR_MEM_SIZE];
int next_instr;
#define MAX_STR_SIZE 128
static char str_buf[MAX_STR_SIZE];
#define clear_str_buf() str_buf[0] = '\0'
// ----------------------------------------------------------------------------
// Emits ----------------------------------------------------------------------

void emit(OpCode op, int o1, int o2, int o3) {
    code[next_instr].op = op;
    code[next_instr].o1 = o1;
    code[next_instr].o2 = o2;
    code[next_instr].o3 = o3;
    next_instr++;
}

#define emit0(op) \
    emit(op, 0, 0, 0)

#define emit1(op, o1) \
    emit(op, o1, 0, 0)

#define emit2(op, o1, o2) \
    emit(op, o1, o2, 0)

#define emit3(op, o1, o2, o3) \
    emit(op, o1, o2, o3)

void backpatch_jump(int instr_addr, int jump_addr) {
    code[instr_addr].o1 = jump_addr;
}

void backpatch_branch(int instr_addr, int offset) {
    code[instr_addr].o2 = offset;
}

// ----------------------------------------------------------------------------
// Prints ---------------------------------------------------------------------

#define LINE_SIZE 80
#define MAX_STR_SIZE 128

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

void get_instruction_string(Instr instr, char *s) {
    OpCode op = instr.op;
    s += sprintf(s, "%s", OpStr[op]);
    int op_count = OpCount[op];
    if (op_count == 1) {
        sprintf(s, " %d", instr.o1);
    } else if (op_count == 2) {
        sprintf(s, " %d, %d", instr.o1, instr.o2);
    } else if (op_count == 3) {
        sprintf(s, " %d, %d, %d", instr.o1, instr.o2, instr.o3);
    }
}

void write_instruction(int addr) {
    Instr instr = code[addr];
    char instr_str[LINE_SIZE];
    get_instruction_string(instr, instr_str);
    printf("%s\n", instr_str);
}

void dump_program() {
    for (int addr = 0; addr < next_instr; addr++) {
        write_instruction(addr);
    }
}

void dump_str_table() {
    int table_size = get_str_table_size(st);
    for (int i = 0; i < table_size; i++) {
        printf("SSTR %s\n", get_string(st, i));
    }
}

// ----------------------------------------------------------------------------
// AST Traversal --------------------------------------------------------------

int int_regs_count;
int float_regs_count;

#define new_int_reg() \
    int_regs_count++

#define new_float_reg() \
    float_regs_count++

int rec_emit_code(AST *ast);

// ----------------------------------------------------------------------------

int emit_assign(AST *ast) {
    AST *r = get_child(ast, 1);
    int x = rec_emit_code(r);
    int addr = get_data(get_child(ast, 0));
    Type var_type = get_type(vt, addr);
    if (var_type == REAL_TYPE) {
        emit2(STWf, addr, x);
    } else { // All other types, include ints, bools and strs.
        emit2(STWi, addr, x);
    }
    return -1; // This is not an expression, hence no value to return.
}

int emit_eq(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int y = rec_emit_code(l);
    int z = rec_emit_code(r);
    int x = new_int_reg();
    if (get_node_type(r) == REAL_TYPE) { // Could equally test 'l' here.
        emit3(EQUf, x, y, z);
    } else if (get_node_type(r) == INT_TYPE) {
        emit3(EQUi, x, y, z);
    } else { // Must be STR_TYPE
        emit3(EQUs, x, y, z);
    }
    return x;
}

int emit_neq(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int y = rec_emit_code(l);
    int z = rec_emit_code(r);
    int x = new_int_reg();
    if (get_node_type(r) == REAL_TYPE) { // Could equally test 'l' here.
        emit3(NEQUf, x, y, z);
    } else if (get_node_type(r) == INT_TYPE) {
        emit3(NEQUi, x, y, z);
    } else { // Must be STR_TYPE
        emit3(NEQUs, x, y, z);
    }
    return x;
}

int emit_le(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int y = rec_emit_code(l);
    int z = rec_emit_code(r);
    int x = new_int_reg();
    int w = new_int_reg();
    int a = new_int_reg();
    if (get_node_type(r) == REAL_TYPE) { // Could equally test 'l' here.
        emit3(LTHf, x, y, z);
        emit3(EQUf, w, y, z);
    } else if (get_node_type(r) == INT_TYPE) {
        emit3(LTHi, x, y, z);
        emit3(EQUi, w, y, z);
    } else { // Must be STR_TYPE
        emit3(LTHs, x, y, z);
        emit3(EQUs, w, y, z);
    }
    emit3(OROR, a, x, w);
    return a;
}

int emit_ge(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int y = rec_emit_code(l);
    int z = rec_emit_code(r);
    int x = new_int_reg();
    int w = new_int_reg();
    int a = new_int_reg();
    if (get_node_type(r) == REAL_TYPE) { // Could equally test 'l' here.
        emit3(LTHf, x, z, y);
        emit3(EQUf, w, x, y);
    } else if (get_node_type(r) == INT_TYPE) {
        emit3(LTHi, x, z, y);
        emit3(EQUi, w, z, y);
    } else { // Must be STR_TYPE
        emit3(LTHs, x, z, y);
        emit3(EQUs, w, z, y);
    }
    emit3(OROR, a, x, w);
    return a;
}

int emit_block(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
    return -1; // This is not an expression, hence no value to return.
}

int emit_stmt_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
    return -1; // This is not an expression, hence no value to return.
}

int emit_expr_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
    return -1; // This is not an expression, hence no value to return.
}

int emit_init_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
    return -1; // This is not an expression, hence no value to return.
}

int emit_arg_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
    return -1; // This is not an expression, hence no value to return.
}

int emit_func_list(AST *ast) {
    int size = get_child_count(ast);
    for (int i = 0; i < size; i++) {
        rec_emit_code(get_child(ast, i));
    }
    return -1; // This is not an expression, hence no value to return.
}

int emit_bool_val(AST *ast) {
    int x = new_int_reg();
    int c = get_data(ast);
    emit2(LDIi, x, c);
    return x;
}

int emit_if(AST *ast) {
    // Code for test.
    int test_reg = rec_emit_code(get_child(ast, 0));
    int cond_jump_instr = next_instr;
    emit2(BOFb, test_reg, 0); // Leave offset empty now, will be backpatched.

    // Code for TRUE block.
    int true_branch_start = next_instr;
    rec_emit_code(get_child(ast, 1)); // Generate TRUE block.

    // Code for FALSE block.
    int false_branch_start;
    if (get_child_count(ast) == 3) { // We have an else.
        // Emit unconditional jump for TRUE block.
        int uncond_jump_instr = next_instr;
        emit1(JUMP, 0); // Leave address empty now, will be backpatched.
        false_branch_start = next_instr;
        rec_emit_code(get_child(ast, 2)); // Generate FALSE block.
        // Backpatch unconditional jump at end of TRUE block.
        backpatch_jump(uncond_jump_instr, next_instr);
    } else {
        false_branch_start = next_instr;
    }

    // Backpatch test.
    backpatch_branch(cond_jump_instr, false_branch_start - true_branch_start + 1);

    return -1; // This is not an expression, hence no value to return.
}

int emit_int_val(AST *ast) {
    int x = new_int_reg();
    int c = get_data(ast);
    emit2(LDIi, x, c);
    return x;
}

int emit_lt(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int y = rec_emit_code(l);
    int z = rec_emit_code(r);
    int x = new_int_reg();
    if (get_node_type(r) == REAL_TYPE) {  // Could equally test 'l' here.
        emit3(LTHf, x, y, z);
    } else if (get_node_type(r) == INT_TYPE) {
        emit3(LTHi, x, y, z);
    } else { // Must be STR_TYPE
        emit3(LTHs, x, y, z);
    }
    return x;
}

int emit_and(AST *ast){
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int y = rec_emit_code(l);
    int z = rec_emit_code(r);
    int x = new_int_reg();
    emit3(ANDAND, x, z, y);

    return x;
}

int emit_or(AST *ast){
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int y = rec_emit_code(l);
    int z = rec_emit_code(r);
    int x = new_int_reg();
    emit3(OROR, x, z, y);

    return x;
}

int emit_gt(AST *ast) {
    AST *l = get_child(ast, 0);
    AST *r = get_child(ast, 1);
    int y = rec_emit_code(l);
    int z = rec_emit_code(r);
    int x = new_int_reg();
    if (get_node_type(r) == REAL_TYPE) {  // Could equally test 'l' here.
        emit3(LTHf, x, z, y);
    } else if (get_node_type(r) == INT_TYPE) {
        emit3(LTHi, x, z, y);
    } else { // Must be STR_TYPE
        emit3(LTHs, x, z, y);
    }
    return x;
}

int emit_minus(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int z = rec_emit_code(get_child(ast, 1));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit3(SUBf, x, y, z);
    } else {
        x = new_int_reg();
        emit3(SUBi, x, y, z);
    }
    return x;
}

int emit_pre_inc(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int addr = get_data(get_child(ast, 0));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit2(INCf, x, y);
        emit2(STWf, addr, x);
    } else {
        x = new_int_reg();
        emit2(INCi, x, y);
        emit2(STWi, addr, x);
    }
    return x;
}

int emit_pre_dec(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int addr = get_data(get_child(ast, 0));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit2(DECf, x, y);
        emit2(STWf, addr, x);
    } else {
        x = new_int_reg();
        emit2(DECi, x, y);
        emit2(STWi, addr, x);
    }
    return x;
}

int emit_uplus(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int addr = get_data(get_child(ast, 0));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit2(UPLUSf, x, y);
        emit2(STWf, addr, x);
    } else {
        x = new_int_reg();
        emit2(UPLUSi, x, y);
        emit2(STWi, addr, x);
    }
    return x;
}

int emit_uminus(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int addr = get_data(get_child(ast, 0));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit2(UMINUSf, x, y);
        emit2(STWf, addr, x);
    } else {
        x = new_int_reg();
        emit2(UMINUSi, x, y);
        emit2(STWi, addr, x);
    }
    return x;
}

int emit_over(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int z = rec_emit_code(get_child(ast, 1));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit3(DIVf, x, y, z);
    } else {
        x = new_int_reg();
        emit3(DIVi, x, y, z);
    }
    return x;
}

int emit_mod(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int z = rec_emit_code(get_child(ast, 1));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit3(MODi, x, y, z);
    } else {
        x = new_int_reg();
        emit3(MODi, x, y, z);
    }
    return x;
}

int emit_plus(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int z = rec_emit_code(get_child(ast, 1));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit3(ADDf, x, y, z);
    } else if (get_node_type(ast) == INT_TYPE) {
        x = new_int_reg();
        emit3(ADDi, x, y, z);
    } else if (get_node_type(ast) == BOOL_TYPE) {
        x = new_int_reg();
        emit3(OROR, x, y, z);
    } else { // Must be STR_TYPE
        x = new_int_reg();
        emit3(CATs, x, y, z);
    }
    return x;
}

int emit_program(AST *ast) {
    rec_emit_code(get_child(ast, 0)); // var_list
    //rec_emit_code(get_child(ast, 1)); // block
    return -1;  // This is not an expression, hence no value to return.
}

int emit_read(AST *ast) {
    AST *var = get_child(ast, 0);
    Type var_type = get_node_type(var);
    int addr = get_data(var);
    int x;
    if (var_type == INT_TYPE) {
        x = new_int_reg();
        emit2(CALL, 0, x);
        emit2(STWi, addr, x);
    } else if (var_type == REAL_TYPE) {
        x = new_float_reg();
        emit2(CALL, 1, x);
        emit2(STWf, addr, x);
    } else if (var_type == BOOL_TYPE) {
        x = new_int_reg();
        emit2(CALL, 2, x);
        emit2(STWi, addr, x);
    } else { // Must be STR_TYPE
        x = new_int_reg();
        emit2(CALL, 3, x);
        emit2(STWi, addr, x);
    }
    return -1;  // This is not an expression, hence no value to return.
}

int emit_real_val(AST *ast) {
    int x = new_float_reg();
    // We need to read as an int because the TM cannot handle floats directly.
    // But we have a float stored in the AST, so we just read it as an int
    // and magically we have a float encoded as an int... :P
    int c = get_data(ast);
    emit2(LDIf, x, c);
    return x;
}

int emit_repeat(AST *ast) {
    
    int test_reg = rec_emit_code(get_child(ast, 1)); // Emit code for test.
    int begin_repeat = next_instr;
    emit2(BOFb, test_reg, begin_repeat - next_instr - 1);
    rec_emit_code(get_child(ast, 0)); // Emit code for body.
    return -1;  // This is not an expression, hence no value to return.
}

int emit_return(AST *ast) {
    int begin_repeat = next_instr;
    rec_emit_code(get_child(ast, 0)); // Emit code for body.
    int test_reg = rec_emit_code(get_child(ast, 1)); // Emit code for test.
    emit2(BOFb, test_reg, begin_repeat - next_instr);
    return -1;  // This is not an expression, hence no value to return.
}

int emit_str_val(AST *ast) {
    int x = new_int_reg();
    int c = get_data(ast);
    emit2(LDIi, x, c);
    return x;
}


int emit_scanf(AST *ast) {
    int x;
    int addr = get_data(get_child(ast, 1));
    Type var_type = get_type(vt, addr);
    
    if (var_type == INT_TYPE) {
        x = new_int_reg();
        emit2(CALL, 0, x);
        emit2(STWi, addr, x);
    } else if (var_type == REAL_TYPE) {
        x = new_float_reg();
        emit2(CALL, 1, x);
        emit2(STWf, addr, x);
    } else if (var_type == BOOL_TYPE) {
        x = new_int_reg();
        emit2(CALL, 2, x);
        emit2(STWi, addr, x);
    } else { // Must be STR_TYPE
        x = new_int_reg();
        emit2(CALL, 3, x);
        emit2(STWi, addr, x);
    }
    return -1;  // This is not an expression, hence no value to return.
}
   
//     if (expr_type == STR_TYPE)
//     {
//         int s = popi(); // String pointer
//         clear_str_buf();
//         escape_str(get_string(st, s), str_buf);
//         if(strstr(str_buf, "%d") != NULL) {
//             int x = new_int_reg();
//             int addr = get_data(get_child(ast, 1));
//             emit2(CALL, 0, x);
//             emit2(STWi, addr, x);
//         } else if(strstr(str_buf, "%f") != NULL) {
//             int x = new_float_reg();
//             int addr = get_data(get_child(ast, 1));
            
//             emit2(CALL, 1, x);
//             emit2(STWf, addr, x);
//         } 
//         else if(strstr(str_buf, "%s") != NULL){ // Must be STR_TYPE
//             int x = new_int_reg();
//             int addr = get_data(get_child(ast, 1));
//             emit2(CALL, 3, x);
//             emit2(STWi, addr, x);
//         }
//     }
//     return -1;  // This is not an expression, hence no value to return.
// }


int emit_printf(AST *ast) 
{
    int size = get_child_count(ast);
    if(size == 2) 
    {
        AST *expr = get_child(ast, 0);

        Type expr_type = get_node_type(expr);
        
        if (expr_type == STR_TYPE)
        {
            int s = popi(); // String pointer
            clear_str_buf();
            escape_str(get_string(st, s), str_buf);

            if(strstr(str_buf, "%s") != NULL)
            {
                AST *expr2 = get_child(ast, 1);
                int x = rec_emit_code(expr2);
                Type expr_type2 = get_node_type(expr2);

                if (expr_type2 == STR_TYPE)
                {
                    emit2(CALL, 7, x); 
                }    
            }
            if(strstr(str_buf, "%d") != NULL)
            {
                AST *expr2 = get_child(ast, 1);
                int x = rec_emit_code(expr2);
                Type expr_type2 = get_node_type(expr2);
        
                if (expr_type2 == INT_TYPE)
                {
                    emit2(CALL, 4, x);
                } 
                if (expr_type2 == BOOL_TYPE)
                {
                    emit2(CALL, 6, x);
                }    
            }
            if(strstr(str_buf, "%f") != NULL)
            {
                AST *expr2 = get_child(ast, 1);
                int x = rec_emit_code(expr2);
                Type expr_type2 = get_node_type(expr2);
        
                if (expr_type2 == REAL_TYPE)
                {
                    emit2(CALL, 5, x);
                }    
            }
        }
    }
    else
    {
        AST *expr = get_child(ast, 0);
        int x = rec_emit_code(expr);
        
        emit2(CALL, 7, x); 
    }

    return -1;
}

int emit_times(AST *ast) {
    int x;
    int y = rec_emit_code(get_child(ast, 0));
    int z = rec_emit_code(get_child(ast, 1));
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit3(MULf, x, y, z);
    } else {
        x = new_int_reg();
        emit3(MULi, x, y, z);
    }
    return x;
}

int emit_var_decl(AST *ast) {
    // Nothing to do here.
    return -1;  // This is not an expression, hence no value to return.
}

int emit_var_list(AST *ast) {
    // Nothing to do here.
    return -1;  // This is not an expression, hence no value to return.
}

int emit_var_use(AST *ast) {
    int addr = get_data(ast);
    int x;
    if (get_node_type(ast) == REAL_TYPE) {
        x = new_float_reg();
        emit2(LDWf, x, addr);
    } else {
        x = new_int_reg();
        emit2(LDWi, x, addr);
    }
    return x;
}

int emit_write(AST *ast) {
    AST *expr = get_child(ast, 0);
    int x = rec_emit_code(expr);
    Type expr_type = get_node_type(expr);
    switch(expr_type) {
        case INT_TYPE:  emit2(CALL, 4, x);  break;
        case REAL_TYPE: emit2(CALL, 5, x);  break;
        case BOOL_TYPE: emit2(CALL, 6, x);  break;
        case STR_TYPE:  emit2(CALL, 7, x);  break;
        case NO_TYPE:
        default:
            fprintf(stderr, "Invalid type: %s!\n", get_text(expr_type));
            exit(EXIT_FAILURE);
    }
    return -1;  // This is not an expression, hence no value to return.
}

int emit_b2i(AST* ast) {
    int x = rec_emit_code(get_child(ast, 0));
    // Nothing else to do, a bool already is stored as an int.
    return x;
}

int emit_b2r(AST* ast) {
    int i = rec_emit_code(get_child(ast, 0));
    int r = new_float_reg();
    emit2(WIDf, r, i);
    return r;
}

int emit_b2s(AST* ast) {
    int x = new_int_reg();
    int y = rec_emit_code(get_child(ast, 0));
    emit2(B2Ss, x, y);
    return x;
}

int emit_i2r(AST* ast) {
    int i = rec_emit_code(get_child(ast, 0));
    int r = new_float_reg();
    emit2(WIDf, r, i);
    return r;
}

int emit_i2s(AST* ast) {
    int x = new_int_reg();
    int y = rec_emit_code(get_child(ast, 0));
    emit2(I2Ss, x, y);
    return x;
}

int emit_r2s(AST* ast) {
    int x = new_int_reg();
    int y = rec_emit_code(get_child(ast, 0));
    emit2(R2Ss, x, y);
    return x;
}

// ----------------------------------------------------------------------------

int rec_emit_code(AST *ast) {
    switch(get_kind(ast)) {
        case ASSIGN_NODE:	    return emit_assign(ast);      
        case BLOCK_NODE:	    return emit_block(ast);       
        case IF_NODE:			return emit_if(ast);          
        case INT_VAL_NODE:		return emit_int_val(ast);     
        case LT_NODE:			return emit_lt(ast);          
        case GT_NODE:			return emit_gt(ast);          
        case LE_NODE:			return emit_le(ast);          
        case GE_NODE:			return emit_ge(ast);          
        case EQ_NODE:			return emit_eq(ast);          
        case NE_NODE:			return emit_neq(ast);          
        case MINUS_NODE:		return emit_minus(ast);       
        case OVER_NODE:			return emit_over(ast);        
        case MOD_NODE:			return emit_mod(ast);         
        case PLUS_NODE:			return emit_plus(ast);        
        case AND_NODE:			return emit_and(ast);     
        case OR_NODE:			return emit_or(ast);          
        // case POST_INC_NODE:     return emit_post_inc(ast);  
        case PRE_INC_NODE:		return emit_pre_inc(ast); 
        // case POST_DEC_NODE:     return emit_post_dec(ast);  
        case PRE_DEC_NODE:		return emit_pre_dec(ast); 
        //case REF_NODE:			return emit_ref(ast);     
        //case DEREF_NODE:		return emit_dref(ast);        
        // case UPLUS_NODE:		return emit_uplus(ast);       
        // case UMINUS_NODE:		return emit_uminus(ast);      
        //case LNEG_NODE:			return emit_lneg(ast);    
        case PRINTF_NODE:       return emit_printf(ast);  
        case SCANF_NODE:        return emit_scanf(ast);      
        case PROGRAM_NODE:		return emit_program(ast);     
        case READ_NODE:			return emit_read(ast);        
        case REAL_VAL_NODE:		return emit_real_val(ast);    
        case REPEAT_NODE:		return emit_repeat(ast);      
        case STR_VAL_NODE:		return emit_str_val(ast);     
        case TIMES_NODE:		return emit_times(ast);       
        case VAR_DECL_NODE:		return emit_var_decl(ast);    
        case VAR_LIST_NODE:		return emit_var_list(ast);    
        case ARG_LIST_NODE:		return emit_arg_list(ast);
        case EXPR_LIST_NODE:	return emit_expr_list(ast);   
        case STMT_LIST_NODE:	return emit_stmt_list(ast);   
        case FUNC_LIST_NODE:	return emit_func_list(ast);   
        case VAR_USE_NODE:		return emit_var_use(ast);     
        case WRITE_NODE:		return emit_write(ast);       
        case RETURN_NODE:		return emit_return(ast);      
        case INIT_LIST_NODE:	return emit_init_list(ast);   
        case B2I_NODE:			return emit_b2i(ast);         
        case B2R_NODE:			return emit_b2r(ast);         
        case B2S_NODE:			return emit_b2s(ast);         
        case I2R_NODE:			return emit_i2r(ast);         
        case I2S_NODE:			return emit_i2s(ast);         
        case R2S_NODE:			return emit_r2s(ast);         

        default:
            fprintf(stderr, "Invalid kind: %s!\n", kind2str(get_kind(ast)));
            exit(EXIT_FAILURE);
    }
}

// ----------------------------------------------------------------------------

void emit_code(AST *ast) {
    next_instr = 0;
    int_regs_count = 0;
    float_regs_count = 0;
    // dump_str_table();
    rec_emit_code(ast);
    emit0(HALT);
    dump_program();
}
