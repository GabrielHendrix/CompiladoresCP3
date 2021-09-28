
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "tables.h"
#include "types.h"

#define CHILDREN_LIMIT 50 // Don't try this at home, kids... :P

struct node {
    NodeKind kind;
    union {
        int   as_int;
        float as_float;
    } data;
    Type type;
    int count;
    AST* child[CHILDREN_LIMIT];
};

AST* new_node(NodeKind kind, int data, Type type) {
    AST* node = malloc(sizeof * node);
    node->kind = kind;
    node->data.as_int = data;
    node->type = type;
    node->count = 0;
    for (int i = 0; i < CHILDREN_LIMIT; i++) {
        node->child[i] = NULL;
    }
    return node;
}

void add_child(AST *parent, AST *child) {
    if (parent->count == CHILDREN_LIMIT) {
        fprintf(stderr, "Cannot add another child!\n");
        exit(1);
    }
    parent->child[parent->count] = child;
    parent->count++;
}

AST* get_child(AST *parent, int idx) {
    return parent->child[idx];
}

AST* new_subtree(NodeKind kind, Type type, int child_count, ...) {
    if (child_count > CHILDREN_LIMIT) {
        fprintf(stderr, "Too many children as arguments!\n");
        exit(1);
    }

    AST* node = new_node(kind, 0, type);
    va_list ap;
    va_start(ap, child_count);
    for (int i = 0; i < child_count; i++) {
        add_child(node, va_arg(ap, AST*));
    }
    va_end(ap);
    return node;
}

NodeKind get_kind(AST *node) {
    return node->kind;
}

void set_kind (AST * node, NodeKind kind)
{
	if (node->kind == INT_VAL_NODE && kind == REAL_VAL_NODE)
		node->data.as_float = (float) node->data.as_int;
	if (node->kind == REAL_VAL_NODE && kind == INT_VAL_NODE)
		node->data.as_int = (int) node->data.as_float;
	node->kind = kind;
}

Type get_AST_type(AST *node)
{
    return node->type;
}

void set_AST_type (AST * node, Type type)
{
    node->type = type;
}

int get_data(AST *node) {
    return node->data.as_int;
}

void set_float_data(AST *node, float data) {
    node->data.as_float = data;
}

float get_float_data(AST *node) {
    return node->data.as_float;
}


void set_int_data(AST *node, int data)
{
    node->data.as_int = data;
}

int get_int_data(AST *node)
{
    return node->data.as_int;
}

Type get_node_type(AST *node) {
    return node->type;
}

int get_child_count(AST *node) {
    return node->count;
}

void free_tree(AST *tree) {
    if (tree == NULL) return;
    for (int i = 0; i < tree->count; i++) {
        free_tree(tree->child[i]);
    }
    free(tree);
}

// Dot output.

int nr;

extern VarTable *vt;

char* kind2str(NodeKind kind) {
    switch(kind) {
        case ASSIGN_NODE:		return "=";
        case BLOCK_NODE:		return "block";
        case IF_NODE:			return "if";
        case INT_VAL_NODE:		return "";
        case LT_NODE:			return "<";
        case GT_NODE:			return ">";
        case LE_NODE:			return "<=";
        case GE_NODE:			return ">=";
        case EQ_NODE:			return "==";
        case NE_NODE:			return "!=";
        case MINUS_NODE:		return "-";
        case OVER_NODE:			return "/";
        case MOD_NODE:			return "%";
        case PLUS_NODE:			return "+";
        case AND_NODE:			return "&&";
        case OR_NODE:			return "||";
        case POST_INC_NODE:
        case PRE_INC_NODE:		return "++";
        case POST_DEC_NODE:
        case PRE_DEC_NODE:		return "--";
        case REF_NODE:			return "&";
        case DEREF_NODE:		return "*";
        case UPLUS_NODE:		return "+";
        case UMINUS_NODE:		return "-";
        case LNEG_NODE:			return "!";
        case PROGRAM_NODE:		return "program";
        case READ_NODE:			return "read";
        case REAL_VAL_NODE:		return "";
        case REPEAT_NODE:		return "repeat";
        case STR_VAL_NODE:		return "";
        case TIMES_NODE:		return "*";
        case VAR_DECL_NODE:		return "var_decl";
        case VAR_LIST_NODE:		return "var_list";
        case ARG_LIST_NODE:		return "arg_list";
        case EXPR_LIST_NODE:	return "expr_list";
        case STMT_LIST_NODE:	return "stmt_list";
        case FUNC_LIST_NODE:	return "func_list";
        case VAR_USE_NODE:		return "var_use";
        case PRINTF_NODE:       return "printf";
        case SCANF_NODE:        return "scanf";
        case WRITE_NODE:		return "write";
        case RETURN_NODE:		return "return";
        case INIT_LIST_NODE:	return "init_list";
        case B2I_NODE:			return "B2I";
        case B2R_NODE:			return "B2R";
        case B2S_NODE:			return "B2S";
        case I2R_NODE:			return "I2R";
        case I2S_NODE:			return "I2S";
        case R2S_NODE:			return "R2S";
        default:				return "ERROR!!";
    }
}

int has_data(NodeKind kind) {
    switch(kind) {
        case INT_VAL_NODE:
        case REAL_VAL_NODE:
        case STR_VAL_NODE:
        case VAR_DECL_NODE:
        case VAR_USE_NODE:
            return 1;
        default:
            return 0;
    }
}

char check_int (AST *node) {
    // fprintf(stderr, "%s@", get_name(vt, node->data.as_int));
    // printf(get_name(vt, node->data.as_float)[0]);
    if (node->kind == VAR_DECL_NODE || node->kind == VAR_USE_NODE)
        return get_name(vt, node->data.as_int)[0];
    else
        return NULL;
}

char check_float (AST *node) {
    // fprintf(stderr, "%s@", get_name(vt, node->data.as_float));
    // printf(get_name(vt, node->data.as_float)[0]);
    if (node->kind == VAR_DECL_NODE || node->kind == VAR_USE_NODE)
        return get_name(vt, node->data.as_float)[0];
    else
        return NULL;
}

int print_node_dot(AST *node) {
    int my_nr = nr++;

    fprintf(stderr, "node%d[label=\"", my_nr);
    if (node->type != NO_TYPE) {
        fprintf(stderr, "(%s) ", get_text(node->type));
    }
    if (node->kind == VAR_DECL_NODE || node->kind == VAR_USE_NODE) {
        fprintf(stderr, "%s@", get_name(vt, node->data.as_int));
    } else {
        fprintf(stderr, "%s", kind2str(node->kind));
    }
    if (has_data(node->kind)) {
        if (node->kind == REAL_VAL_NODE) {
            fprintf(stderr, "%.2f", node->data.as_float);
        } else if (node->kind == STR_VAL_NODE) {
            fprintf(stderr, "@%d", node->data.as_int);
        } else {
            fprintf(stderr, "%d", node->data.as_int);
        }
    }
    fprintf(stderr, "\"];\n");

    for (int i = 0; i < node->count; i++) {
        int child_nr = print_node_dot(node->child[i]);
        fprintf(stderr, "node%d -> node%d;\n", my_nr, child_nr);
    }
    return my_nr;
}

void print_dot(AST *tree) {
    nr = 0;
    fprintf(stderr, "digraph {\ngraph [ordering=\"out\"];\n");
    print_node_dot(tree);
    fprintf(stderr, "}\n");
}
