#ifndef AST_H
#define AST_H

#include "types.h"

typedef enum {
    ASSIGN_NODE,
    BLOCK_NODE,
    BOOL_VAL_NODE,
    IF_NODE,
    INT_VAL_NODE,
    LT_NODE,
    GT_NODE,
    LE_NODE,
    GE_NODE,
    EQ_NODE,
    NE_NODE,
    MINUS_NODE,
    OVER_NODE,
    MOD_NODE,
    PLUS_NODE,
    AND_NODE,
    OR_NODE,
    POST_INC_NODE,
    PRE_INC_NODE,
    POST_DEC_NODE,
    PRE_DEC_NODE,
    REF_NODE,
    DEREF_NODE,
    UPLUS_NODE,
    UMINUS_NODE,
    LNEG_NODE,
    PROGRAM_NODE,
    READ_NODE,
    REAL_VAL_NODE,
    REPEAT_NODE,
    STR_VAL_NODE,
    TIMES_NODE,
    VAR_DECL_NODE,
    VAR_LIST_NODE,
    ARG_LIST_NODE,
    EXPR_LIST_NODE,
    STMT_LIST_NODE,
    FUNC_LIST_NODE,
    VAR_USE_NODE,
    WRITE_NODE,
    PRINTF_NODE,
    SCANF_NODE,
    RETURN_NODE,
    INIT_LIST_NODE,

    B2I_NODE,   // Conversion of types.
    B2R_NODE,
    B2S_NODE,
    I2R_NODE,
    I2S_NODE,
    R2S_NODE
} NodeKind;

struct node; // Opaque structure to ensure encapsulation.

typedef struct node AST;

AST* new_node(NodeKind kind, int data, Type type);

void add_child(AST *parent, AST *child);
AST* get_child(AST *parent, int idx);

AST* new_subtree(NodeKind kind, Type type, int child_count, ...);

NodeKind get_kind(AST *node);
void set_kind (AST * node, NodeKind kind);
char* kind2str(NodeKind kind);

Type get_AST_type(AST *node);
void set_AST_type (AST * node, Type type);

int get_data(AST *node);
void set_float_data(AST *node, float data);
float get_float_data(AST *node);
void set_int_data(AST *node, int data);
int get_int_data(AST *node);

Type get_node_type(AST *node);
int get_child_count(AST *node);

void print_tree(AST *ast);
void print_dot(AST *ast);

void free_tree(AST *ast);

#endif
