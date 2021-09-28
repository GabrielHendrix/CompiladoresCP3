%output "parser.c"		  // File name of generated parser.
%defines "parser.h"		 // Produces a 'parser.h'
%define parse.error verbose // Give proper messages when a syntax error is found.
%define parse.lac full	  // Enable LAC to improve syntax error handling.

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/types.h"
#include "src/tables.h"
#include "src/ast.h"
#include "parser.h"
#include "src/code.h"


#define YYERROR_VERBOSE 1

int yylex (void);
int yylex_destroy (void);
void yyerror (char const *s);

AST* check_var ();
AST* new_var ();

AST* unify_bin_node (AST* l, AST* r, NodeKind kind, const char* op, Unif (*unify) (Type,Type));

AST* check_assign (AST *l, AST *r);
AST* check_if_then (AST *e, AST *b);
AST* check_if_then_else (AST *e, AST *b1, AST *b2);
AST* check_string(AST *x, AST *y);
AST* check_repeat (AST *b, AST *e);

AST * cast_node (AST * a);

extern char * yytext;
extern int yylineno;

StrTable * st;
VarTable * vt;

Type last_decl_type;
AST *root;
%}

%define api.value.type {AST*}

// keywords
%token AUTO BREAK CASE CHAR CONST CONTINUE DEFAULT DO DOUBLE PRINTF SCANF ELSE ENUM EXTERN FLOAT FOR GOTO IF INT LONG REGISTER RETURN SHORT SIGNED SIZEOF STATIC STRUCT SWITCH TYPEDEF UNION UNSIGNED VOID VOLATILE WHILE

// identifiers
%token ID TYPEDEF_NAME

// constant
%token CONSTVAL

// string literal
%token STR_VAL

// punctuators
%token AMPER TIMES OVER MOD PLUS MINUS EXCL ACC_OP PTR_OP INC_OP DEC_OP LT_OP GT_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP DDOT QSTM ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN SUB_ASSIGN COMMA SEMI LPAR RPAR LBRAC RBRAC LCURL RCURL

// custom
%token READ WRITE 

%precedence RPAR
%precedence ELSE

%start root
%%

primary_expression:
	  ID		{ $$ = check_var (); }
	| CONSTVAL	{ $$ = $1; }
	| STR_VAL	{ $$ = $1; }
	| LPAR expression RPAR { $$ = $1; };

postfix_expression:
	  primary_expression { $$ = $1; }
	| postfix_expression LBRAC expression RBRAC { add_child ($1, $3); $$ = $1; }
	| postfix_expression LPAR RPAR { $$ = $1; }
	| postfix_expression LPAR argument_expression_list RPAR { add_child ($1, $3); $$ = $1; }
	// | postfix_expression ACC_OP ID
	// | postfix_expression PTR_OP ID
	| postfix_expression INC_OP { $$ = new_subtree (POST_INC_NODE, get_AST_type ($1), 1, $1); }
	| postfix_expression DEC_OP { $$ = new_subtree (POST_DEC_NODE, get_AST_type ($1), 1, $1); };

argument_expression_list:
	  assignment_expression { $$ = new_subtree (ARG_LIST_NODE, NO_TYPE, 1, $1); }
	| argument_expression_list COMMA assignment_expression { add_child ($1, $3); $$ = $1; };

unary_expression:
	  postfix_expression { $$ = $1; }
	| INC_OP unary_expression { $$ = new_subtree (PRE_INC_NODE, get_AST_type ($2), 1, $2); }
	| DEC_OP unary_expression { $$ = new_subtree (PRE_DEC_NODE, get_AST_type ($2), 1, $2); }
	| PLUS unary_expression { $$ = new_subtree (UPLUS_NODE, get_AST_type ($2), 1, $2); }
	| MINUS unary_expression { $$ = new_subtree (UMINUS_NODE, get_AST_type ($2), 1, $2); }
	//| unary_operator { add_child ($1, $2); if (get_AST_type ($1) == NO_TYPE) set_AST_type ($1, get_AST_type ($2)); $$ = $1; };
	// | SIZEOF unary_expression { $$ = $2; }
	// | SIZEOF LPAR type_name RPAR { $$ = $2; };


// This project's "simplification" statement states that
// bitwise operations shouldn't be implemented.
// This removes the unary bitwise not from the rule below.
// unary_operator:
// 	  AMPER primary_expression { $$ = new_subtree (REF_NODE, INT_TYPE, 0); }
// 	| TIMES primary_expression { $$ = new_subtree (DEREF_NODE, NO_TYPE, 0); }
// 	| PLUS primary_expression { $$ = new_subtree (UPLUS_NODE, NO_TYPE,  1, $2); }
// 	| MINUS primary_expression { $$ = new_subtree (UMINUS_NODE, NO_TYPE,  1, $2); }
// 	| EXCL primary_expression { $$ = new_subtree (LNEG_NODE, INT_TYPE, 0); };


cast_expression:
	  unary_expression { $$ = $1; }
	| LPAR type_name RPAR cast_expression { $$ = cast_node ($4); };

multiplicative_expression:
	  cast_expression { $$ = $1; }
	| LPAR cast_expression RPAR { $$ = $2; }
	| multiplicative_expression TIMES cast_expression { $$ = unify_bin_node($1, $3, TIMES_NODE, "*", unify_other_arith); }
	| multiplicative_expression OVER cast_expression { $$ = unify_bin_node($1, $3, OVER_NODE, "/", unify_other_arith); }
	| multiplicative_expression MOD cast_expression { $$ = unify_bin_node($1, $3, MOD_NODE, "%", unify_other_arith); };
	| cast_expression  TIMES multiplicative_expression { $$ = unify_bin_node($1, $3, TIMES_NODE, "*", unify_other_arith); }
	| cast_expression OVER multiplicative_expression { $$ = unify_bin_node($1, $3, OVER_NODE, "/", unify_other_arith); }
	| cast_expression  MOD multiplicative_expression { $$ = unify_bin_node($1, $3, MOD_NODE, "%", unify_other_arith); };

additive_expression:
	  multiplicative_expression { $$ = $1; }
	| LPAR additive_expression RPAR { $$ = $2; }
	| LPAR multiplicative_expression RPAR { $$ = $2; }
	| additive_expression PLUS multiplicative_expression { $$ = unify_bin_node($1, $3, PLUS_NODE, "+", unify_plus); }
	| additive_expression MINUS multiplicative_expression { $$ = unify_bin_node($1, $3, MINUS_NODE, "-", unify_plus); };
	| multiplicative_expression PLUS additive_expression { $$ = unify_bin_node($1, $3, PLUS_NODE, "+", unify_plus); }
	| multiplicative_expression MINUS additive_expression { $$ = unify_bin_node($1, $3, MINUS_NODE, "-", unify_plus); };

// Again, this project's "simplification" statement states that
// bitwise operations shouldn't be implemented.
// This transforms the rule
// shift_expression:
//	  additive_expression
//	| shift_expression "<<" additive_expression
//	| shift_expression ">>" additive_expression;
// into the rule
// shift_expression:
//	  additive_expression;
// which is pointless.
// The rule was, then, not transcribed,
// and in the places it should have been used, like below,
// "shift_expression" was directly substituted by "additive_expression".
relational_expression:
	  additive_expression { $$ = $1; }
	| relational_expression LT_OP additive_expression { $$ = unify_bin_node ($1, $3, LT_NODE, "<", unify_comp); }
	| relational_expression GT_OP additive_expression { $$ = unify_bin_node ($1, $3, GT_NODE, ">", unify_comp); }
	| relational_expression LE_OP additive_expression { $$ = unify_bin_node ($1, $3, LE_NODE, "<=", unify_comp); }
	| relational_expression GE_OP additive_expression { $$ = unify_bin_node ($1, $3, GE_NODE, ">=", unify_comp); };

equality_expression:
	  relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = unify_bin_node ($1, $3, EQ_NODE, "==", unify_comp); }
	| equality_expression NE_OP relational_expression { $$ = unify_bin_node ($1, $3, NE_NODE, "!=", unify_comp); };

// For the same reason stated in the previous comment,
// in the places it should have been used, like below,
// the rule chain composed by "and_expression", "exclusive_or_expression"
// and "inclusive_or_expression" were removed/substituted by
// "equality_expression".
logical_and_expression:
	  equality_expression { $$ = $1; }
	| logical_and_expression AND_OP equality_expression { $$ = unify_bin_node($1, $3, AND_NODE, "&&", unify_logic); };

logical_or_expression:
	  logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = unify_bin_node($1, $3, OR_NODE, "||", unify_logic); };

conditional_expression:
	  logical_or_expression { $$ = $1; };
	// | logical_or_expression QSTM expression DDOT conditional_expression;

assignment_expression:
	  conditional_expression { $$ = $1; }
	| unary_expression assignment_operator assignment_expression { $$ = check_assign ($1, $3); };

// This project's "simplification" statement states that
// bitwise operations shouldn't be implemented.
// This removes all bitwise assignments from the rule below.
assignment_operator:
	  ASSIGN { }
	| MUL_ASSIGN { }
	| DIV_ASSIGN { }
	| MOD_ASSIGN { }
	| ADD_ASSIGN { }
	| SUB_ASSIGN { };

expression:
	  assignment_expression { $$ = new_subtree (EXPR_LIST_NODE, NO_TYPE, 1, $1); }
	| expression COMMA assignment_expression { add_child ($1, $3); $$ = $1; };


// Reference material has the rule constant_expression: conditional_expression;
// It seems quite pointless for the purposes of this Checkpoint.
// The rule was, then, not transcribed,
// and in the places it should have been used,
// "constant_expression" was substituted by "conditional_expression".

// Here and downwards, the "<opt>" modifier was adapted as deemed fit.
declaration:
	  declaration_specifier_list SEMI { }
	| declaration_specifier_list init_declarator_list SEMI { $$ = $2; };

declaration_specifier_list:
	  declaration_specifier { }
	| declaration_specifier declaration_specifier_list { };

declaration_specifier:
	//   storage_class_specifier
	  type_specifier { };
	// | type_qualifier;

init_declarator_list:
	  init_declarator { $$ = new_subtree(VAR_LIST_NODE, NO_TYPE, 1, $1); }
	| init_declarator_list COMMA init_declarator { add_child($1, $3); $$ = $1; };

init_declarator:
	  declarator { $$ = $1; }
	| declarator ASSIGN initializer { $$ = check_assign ($1, $3); };

// storage_class_specifier:
// 	  TYPEDEF
// 	| EXTERN
// 	| STATIC
// 	| AUTO
// 	| REGISTER;

type_specifier:
	  VOID			{ last_decl_type = VOID_TYPE; }
	| CHAR			{ last_decl_type = STR_TYPE; }
	| SHORT			{ last_decl_type = INT_TYPE; }
	| INT			{ last_decl_type = INT_TYPE; }
	| LONG			{ last_decl_type = INT_TYPE; }
	| FLOAT			{ last_decl_type = REAL_TYPE; }
	| DOUBLE		{ last_decl_type = REAL_TYPE; }
	| SIGNED		{ last_decl_type = INT_TYPE; }
	| UNSIGNED		{ last_decl_type = INT_TYPE; };
	// | struct_or_union_specifier
	// | enum_specifier
	// | TYPEDEF_NAME;

// struct_or_union_specifier:
// 	  struct_or_union LCURL struct_declaration_list RCURL
// 	| struct_or_union ID LCURL struct_declaration_list RCURL
// 	| struct_or_union ID;

// struct_or_union:
// 	  STRUCT
// 	| UNION;

// struct_declaration_list:
// 	  struct_declaration
// 	| struct_declaration_list struct_declaration;

// struct_declaration:
// 	  specifier_qualifier_list struct_declarator_list SEMI;

specifier_qualifier_list:
	  type_specifier { };
	// | type_specifier specifier_qualifier_list
	// | type_qualifier
	// | type_qualifier specifier_qualifier_list;

// struct_declarator_list:
// 	  struct_declarator
// 	| struct_declarator_list COMMA struct_declarator;

// struct_declarator:
// 	  declarator
// 	| DDOT conditional_expression
// 	| declarator DDOT conditional_expression;

// enum_specifier:
// 	  ENUM LCURL enumerator_list RCURL
// 	| ENUM ID LCURL enumerator_list RCURL
// 	| ENUM ID;

// enumerator_list:
// 	  enumerator
// 	| enumerator_list COMMA enumerator;

// Reference material has the rule enumeration-constant: ID;
// Again, it seems quite pointless.
// The rule was, then, not transcribed,
// and in the places it should have been used, like below,
// "enumeration_constant" was directly substituted by "ID".
// enumerator:
// 	  ID
// 	| ID ASSIGN conditional_expression;

// type_qualifier:
// 	  CONST
// 	| VOLATILE;

declarator:
	  direct_declarator { $$ = $1; };
	// | pointer direct_declarator;

// This project's "simplification" statement states that
// "funções com varargs, valores default, chamada com nomes de parâmetros, 
// packing/unpacking, etc" shouldn't be implemented.
// This transforms the rule
// parameter_type_list:
//	  parameter_list
//	| parameter_list COMMA "...";
// into the rule
// parameter_type_list:
//	  parameter_list;
// which is pointless.
// The rule was, then, not transcribed,
// and in the places it should have been used, like below,
// "parameter_type_list" was directly substituted by "parameter_list".
direct_declarator:
	  ID { $$ = new_var(); }
	| LPAR declarator RPAR { $$ = $2; }
	| direct_declarator LBRAC RBRAC { $$ = $1; }
	| direct_declarator LBRAC conditional_expression RBRAC { add_child ($1, $3); $$ = $1; }
	// | direct_declarator LPAR parameter_list RPAR
	| direct_declarator LPAR RPAR { $$ = $1; };
	// | direct_declarator LPAR identifier_list RPAR;

// pointer:
// 	  TIMES
// 	| TIMES type_qualifier_list
// 	| TIMES pointer
// 	| TIMES type_qualifier_list pointer;

// type_qualifier_list:
// 	  type_qualifier
// 	| type_qualifier_list type_qualifier;

// parameter_list:
// 	  parameter_declaration
// 	| parameter_list COMMA parameter_declaration;

// parameter_declaration:
// 	  declaration_specifier_list declarator
// 	| declaration_specifier_list;
// 	| declaration_specifier_list abstract_declarator;

// identifier_list:
// 	  ID
// 	| identifier_list COMMA ID;

type_name:
	  specifier_qualifier_list { };
	// | specifier_qualifier_list abstract_declarator;

// abstract_declarator:
// 	  pointer
// 	| direct_abstract_declarator
// 	| pointer direct_abstract_declarator;

// direct_abstract_declarator:
// 	  LPAR abstract_declarator RPAR
// 	| LBRAC RBRAC
// 	| LBRAC conditional_expression RBRAC
// 	| direct_abstract_declarator LBRAC RBRAC
// 	| direct_abstract_declarator LBRAC conditional_expression RBRAC
// 	| LPAR RPAR
// 	| LPAR parameter_list RPAR
// 	| direct_abstract_declarator LPAR RPAR
// 	| direct_abstract_declarator LPAR parameter_list RPAR;

// Reference material has, right here, the rule typedef-name: ID;
// The rule was not transcribed,
// and in the places it should have been used,
// "typedef-name" was directly substituted by "ID".

initializer:
	  assignment_expression	{ $$ = $1; };
	// | LCURL initializer_list RCURL { $$ = $1; }
	// | LCURL initializer_list COMMA RCURL { $$ = $1; };

// initializer_list:
// 	  initializer { $$ = new_subtree (INIT_LIST_NODE, NO_TYPE, 1, $1); }
// 	| initializer_list COMMA initializer { add_child ($1, $3); $$ = $1; };

statement:
	//   labeled_statement
	  compound_statement	{ $$ = $1; }
	| expression_statement	{ $$ = $1; }
	| selection_statement	{ $$ = $1; }
	| iteration_statement	{ $$ = $1; }
	| jump_statement		{ $$ = $1; }
	| write_statement		{ $$ = $1; }
	| printf_statement 		{ $$ = $1; }
	| scanf_statement 		{ $$ = $1; };

// labeled_statement:
// 	  ID DDOT statement
// 	| CASE conditional_expression DDOT statement
// 	| DEFAULT DDOT statement;

compound_statement:
	  LCURL RCURL { $$ = new_subtree (BLOCK_NODE, NO_TYPE, 0); }
	| LCURL declaration_list RCURL  { $$ = $2; }
	| LCURL statement_list RCURL { $$ = $2; }
	| LCURL declaration_list statement_list RCURL { $$ = new_subtree (BLOCK_NODE, NO_TYPE, 2, $2, $3); };

declaration_list:
	  declaration { $$ = new_subtree (VAR_LIST_NODE, NO_TYPE, 1, $1); }
	| declaration_list declaration { add_child ($1, $2); $$ = $1; };

statement_list:
	  statement { $$ = new_subtree (STMT_LIST_NODE, NO_TYPE, 1, $1); }
	| statement_list statement { add_child ($1, $2); $$ = $1; };

expression_statement:
	  SEMI { $$ = new_subtree (BLOCK_NODE, NO_TYPE, 0); }
	| expression SEMI { $$ = $1; };

selection_statement:
	  IF LPAR conditional_expression RPAR statement { $$ = check_if_then ($3, $5); }
	| IF LPAR conditional_expression RPAR statement ELSE statement { $$ = check_if_then_else ($3, $5, $7); };
	// | SWITCH LPAR expression RPAR statement;

// Reference material uses "expression<opt> ;" in the rule below.
// A rule for this exact match was already made above
// expression_statement:
//	  SEMI
//	| expression SEMI;
// To simplify the rule below, "expression_statement" will be used instead of
// the transcription of "expression<opt> ;"
iteration_statement:
	  WHILE LPAR conditional_expression RPAR statement { $$ = check_repeat($5, $3); }
	  ;
	// | DO statement WHILE LPAR expression RPAR SEMI
	// | FOR LPAR expression_statement expression_statement RPAR statement
	// | FOR LPAR expression_statement expression_statement expression RPAR statement;

jump_statement:
// 	  GOTO ID SEMI
// 	| CONTINUE SEMI
// 	| BREAK SEMI
	//  RETURN SEMI				{ $$ = new_subtree (RETURN_NODE, NO_TYPE, 0); }
	RETURN expression SEMI	{ { $$ = new_subtree (RETURN_NODE, NO_TYPE, 1, $1); }/*printf ("\tjump_statement2\t%s %d\n", kind2str (get_kind ($2)), get_int_data ($2)); $$ = new_subtree (RETURN_NODE, NO_TYPE, 1, $2);*/ };

root:
	  translation_unit { root = new_subtree (PROGRAM_NODE, NO_TYPE, 1, $1); };

// program:
//   PROGRAM ID SEMI vars-sect stmt-sect { root = new_subtree(PROGRAM_NODE, NO_TYPE, 2, $4, $5); }
// ;

translation_unit:
	  external_declaration					{ $$ = new_subtree (FUNC_LIST_NODE, NO_TYPE, 1, $1); }
	| translation_unit external_declaration	{ add_child ($1, $2); $$ = $1; };

external_declaration:
	  function_definition	{ $$ = $1; }
	| declaration			{ $$ = $1; };

function_definition:
	  declarator compound_statement { $$ = $2; }
	| declaration_specifier_list declarator compound_statement { $$ = $3; }
	| declarator declaration_list compound_statement { $$ = $3; }
	| declaration_specifier_list declarator declaration_list compound_statement { $$ = $4; };

write_statement:
  	WRITE LPAR assignment_expression RPAR SEMI { $$ = new_subtree(WRITE_NODE, NO_TYPE, 1, $3); }
;

printf_statement:
	  PRINTF LPAR STR_VAL RPAR SEMI { $$ = new_subtree(PRINTF_NODE, NO_TYPE, 1, $3); }
	| PRINTF LPAR STR_VAL COMMA assignment_expression RPAR SEMI { $$ = check_string ($3, $5); }
;

scanf_statement:
	| SCANF LPAR STR_VAL COMMA AMPER unary_expression RPAR SEMI{ $$ = new_subtree(SCANF_NODE, NO_TYPE, 2, $3, $6); }
;
%%

// ----------------------------------------------------------------------------

AST* check_var() {
	
    int idx = lookup_var(vt, yytext);
    if (idx == -1) {
        printf("SEMANTIC ERROR (%d): variable '%s' was not declared.\n",
                yylineno, yytext);
        exit(EXIT_FAILURE);
    }
    return new_node(VAR_USE_NODE, idx, get_type(vt, idx));
}

AST* new_var() {
    int idx = lookup_var(vt, yytext);
    if (idx != -1) {
        printf("SEMANTIC ERROR (%d): variable '%s' already declared at line %d.\n",
                yylineno, yytext, get_line(vt, idx));
        exit(EXIT_FAILURE);
    }
    idx = add_var(vt, yytext, yylineno, last_decl_type);
    return new_node(VAR_DECL_NODE, idx, last_decl_type);
}

// ----------------------------------------------------------------------------

// Type checking and inference.

void type_error(const char* op, Type t1, Type rt) {
    printf("SEMANTIC ERROR (%d): incompatible types for operator '%s', LHS is '%s' and RHS is '%s'.\n",
           yylineno, op, get_text(t1), get_text(rt));
    exit(EXIT_FAILURE);
}

AST* create_conv_node(Conv conv, AST *n) {
    switch(conv) {
        case B2I:  return new_subtree(B2I_NODE, INT_TYPE,  1, n);
        case B2R:  return new_subtree(B2R_NODE, REAL_TYPE, 1, n);
        case B2S:  return new_subtree(B2S_NODE, STR_TYPE,  1, n);
        case I2R:  return new_subtree(I2R_NODE, REAL_TYPE, 1, n);
        case I2S:  return new_subtree(I2S_NODE, STR_TYPE,  1, n);
        case R2S:  return new_subtree(R2S_NODE, STR_TYPE,  1, n);
        case NONE: return n;
        default:
            printf("INTERNAL ERROR: invalid conversion of types!\n");
            exit(EXIT_FAILURE);
    }
}

AST* unify_bin_node(AST* l, AST* r,
                    NodeKind kind, const char* op, Unif (*unify)(Type,Type)) {
    Type lt = get_node_type(l);
    Type rt = get_node_type(r);
    Unif unif = unify(lt, rt);
    if (unif.type == NO_TYPE) {
        type_error(op, lt, rt);
    }
    l = create_conv_node(unif.lc, l);
    r = create_conv_node(unif.rc, r);
    return new_subtree(kind, unif.type, 2, l, r);
}


AST* check_assign(AST *l, AST *r) {
    Type lt = get_node_type(l);
    Type rt = get_node_type(r);

    if (lt == INT_TYPE  && rt != INT_TYPE)  type_error("==", lt, rt);
    if (lt == STR_TYPE  && rt != STR_TYPE)  type_error("==", lt, rt);

    if (lt == REAL_TYPE) {
        if (rt == INT_TYPE) {
            r = create_conv_node(I2R, r);
			AST * rc = get_child (r, 0);
			set_float_data (r, (float) get_int_data (rc));
        } else if (rt != REAL_TYPE) {
            type_error("==", lt, rt);
        }
    }

	AST * ret = new_subtree (ASSIGN_NODE, lt, 2, l, r);
	if (lt == INT_TYPE) set_int_data (ret, get_int_data (r));
	if (lt == REAL_TYPE) set_float_data (ret, get_float_data (r));

	
    return ret;
}

AST* check_if_then(AST *e, AST *b) {
    return new_subtree(IF_NODE, NO_TYPE, 2, e, b);
}

AST* check_if_then_else(AST *e, AST *b1, AST *b2) {
    return new_subtree(IF_NODE, NO_TYPE, 3, e, b1, b2);
}

AST* check_repeat(AST *b, AST *e) {
    return new_subtree(REPEAT_NODE, NO_TYPE, 2, e, b);
}

AST* check_string(AST *x, AST *y){
	return new_subtree(PRINTF_NODE, NO_TYPE, 2, x, y);
}

AST * cast_node (AST * a)
{
	if (last_decl_type == REAL_TYPE && (get_kind (a) == REAL_VAL_NODE || get_kind (a) == INT_VAL_NODE))
	{
		set_kind (a, REAL_VAL_NODE);
		return a;
	}
	if (last_decl_type == INT_TYPE && (get_kind (a) == REAL_VAL_NODE || get_kind (a) == INT_VAL_NODE))
	{
		set_kind (a, INT_VAL_NODE);
		return a;
	}
	printf("SEMANTIC ERROR (%d): invalid casting from type '%s' to '%s'.\n", yylineno, kind2str (get_kind (a)), get_text (last_decl_type));
    exit(EXIT_FAILURE);
}

// Primitive error handling.
void yyerror (char const *s) {
    printf("SYNTAX ERROR (%d): %s\n", yylineno, s);
    exit(EXIT_FAILURE);
}

int main() {
    st = create_str_table();
    vt = create_var_table();

    yyparse();
	emit_code(root);
	print_dot(root);
    free_str_table(st);
    free_var_table(vt);
    //free_tree(root);
    yylex_destroy();    // To avoid memory leaks within flex...

    return 0;
}
