/* Flex output */
%option outfile="scanner.c"
/* Unused functions */
%option noyywrap
%option nounput
%option noinput
/* Auto increment variable yylineno every time a '\n' is read. */
%option yylineno

nonzero				[1-9]
digit				[0-9]
octdigit			[0-7]
hexdigit			[0-9a-fA-F]
nondigit			[_a-zA-Z]
float-exp			[Ee][+-]?{digit}+
float-suff			(f|F|l|L)
unsig-suff			[uU]
long-suff			[lL]
int-suff			({unsig-suff}({long-suff}?))|({long-suff}({unsig-suff}?))

id					{nondigit}({nondigit}|{digit})*
hex-val				0[xX]{hexdigit}+({int-suff}?)
oct-val				0{octdigit}*({int-suff}?)
int-val				{nonzero}{digit}*({int-suff}?)

float-val1			{digit}+{float-exp}({float-suff}?)
float-val2			{digit}*"."{digit}+({float-exp}?)({float-suff}?)
float-val3			{digit}+"."{digit}*({float-exp}?)({float-suff}?)

char-val			L?'(\\.|[^\\'])+'
str-val				L?\"(\\.|[^\\"])*\"

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/types.h"
#include "src/tables.h"
#include "src/ast.h"
#include "parser.h"

#define process_token(type) echo (); return (type)

extern StrTable *st;

void echo ();

// {char-val}		{ process_token (CONSTVAL); } TODO

%}

%%
"/*"((("*"[^/])?)|[^*])*"*/"	{  }
[ \t\v\n\f\r]					{ echo (); }
"read"			{ process_token (READ); }
"write"			{ process_token (WRITE); }
"auto"			{ process_token (AUTO); }
"break"			{ process_token (BREAK); }
"case"			{ process_token (CASE); }
"char"			{ process_token (CHAR); }
"const"			{ process_token (CONST); }
"continue"		{ process_token (CONTINUE); }
"default"		{ process_token (DEFAULT); }
"do"			{ process_token (DO); }
"double"		{ process_token (DOUBLE); }
"else"			{ process_token (ELSE); }
"enum"			{ process_token (ENUM); }
"extern"		{ process_token (EXTERN); }
"float"			{ process_token (FLOAT); }
"for"			{ process_token (FOR); }
"goto"			{ process_token (GOTO); }
"if"			{ process_token (IF); }
"int"			{ process_token (INT); }
"long"			{ process_token (LONG); }
"register"		{ process_token (REGISTER); }
"return"		{ process_token (RETURN); }
"short"			{ process_token (SHORT); }
"signed"		{ process_token (SIGNED); }
"sizeof"		{ process_token (SIZEOF); }
"static"		{ process_token (STATIC); }
"struct"		{ process_token (STRUCT); }
"switch"		{ process_token (SWITCH); }
"typedef"		{ process_token (TYPEDEF); }
"union"			{ process_token (UNION); }
"unsigned"		{ process_token (UNSIGNED); }
"void"			{ process_token (VOID); }
"volatile"		{ process_token (VOLATILE); }
"while"			{ process_token (WHILE); }
"printf"		{ process_token (PRINTF); }
"scanf"			{ process_token (SCANF); }

{hex-val}		{ yylval = new_node (INT_VAL_NODE, (int) strtol (yytext, NULL, 16), INT_TYPE); process_token (CONSTVAL); }
{oct-val}		{ yylval = new_node (INT_VAL_NODE, (int) strtol (yytext, NULL, 8), INT_TYPE); process_token (CONSTVAL); }
{int-val}		{ yylval = new_node (INT_VAL_NODE, (int) strtol (yytext, NULL, 10), INT_TYPE); process_token (CONSTVAL); }
{float-val1}	{ yylval = new_node (REAL_VAL_NODE, 0, REAL_TYPE); set_float_data (yylval, (float) strtof (yytext, NULL)); process_token (CONSTVAL); }
{float-val2}	{ yylval = new_node (REAL_VAL_NODE, 0, REAL_TYPE); set_float_data (yylval, (float) strtof (yytext, NULL)); process_token (CONSTVAL); }
{float-val3}	{ yylval = new_node (REAL_VAL_NODE, 0, REAL_TYPE); set_float_data (yylval, (float) strtof (yytext, NULL)); process_token (CONSTVAL); }
{str-val}		{ yylval = new_node (STR_VAL_NODE, add_string (st, yytext), STR_TYPE); process_token (STR_VAL); }
{id}			{ process_token (ID); }

"+="			{ process_token (ADD_ASSIGN); }
"-="			{ process_token (SUB_ASSIGN); }
"*="			{ process_token (MUL_ASSIGN); }
"/="			{ process_token (DIV_ASSIGN); }
"%="			{ process_token (MOD_ASSIGN); }
"++"			{ process_token (INC_OP); }
"--"			{ process_token (DEC_OP); }
"->"			{ process_token (PTR_OP); }
"&&"			{ process_token (AND_OP); }
"||"			{ process_token (OR_OP); }
"<="			{ process_token (LE_OP); }
">="			{ process_token (GE_OP); }
"=="			{ process_token (EQ_OP); }
"!="			{ process_token (NE_OP); }

";"				{ process_token (SEMI); }
"{"				{ process_token (LCURL); }
"}"				{ process_token (RCURL); }
","				{ process_token (COMMA); }
":"				{ process_token (DDOT); }
"="				{ process_token (ASSIGN); }
"("				{ process_token (LPAR); }
")"				{ process_token (RPAR); }
"["				{ process_token (LBRAC); }
"]"				{ process_token (RBRAC); }
"."				{ process_token (ACC_OP); }
"&"				{ process_token (AMPER); }
"!"				{ process_token (EXCL); }
"-"				{ process_token (MINUS); }
"+"				{ process_token (PLUS); }
"*"				{ process_token (TIMES); }
"/"				{ process_token (OVER); }
"%"				{ process_token (MOD); }
"<"				{ process_token (LT_OP); }
">"				{ process_token (GT_OP); }
"?"				{ process_token (QSTM); }

.			{ printf ("LEXICAL ERROR (%d): bad character, %s\n", yylineno, yytext); exit (1); }

%%

void echo ()
{
	// The "ECHO" macro prints what was read, and is useful for debugging.
	// Uncomment bellow for usage.
	// ECHO;
}
