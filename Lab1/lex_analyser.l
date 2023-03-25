%{
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "../bits/token.h"
#include "../bits/ast_node.h"
#include "../bits/k_tree.h"
#include "parser.h"

#define CREATE_TOKEN_NODE(TYPE,BISON_TOKEN) \
do{\
    Token* token=TokenCreate(yylloc.first_line,yylloc.first_column,TYPE,yytext);\
    AstNode* ast_node=AstNodeCreate(true,token);\
    yylval.k_tree_node=KTreeCreateNode(&ast_node);\
    return BISON_TOKEN;\
}while(false)

#define YY_USER_ACTION \
    do{\
        yylloc.first_line=yylineno;\
        yylloc.last_line=yylineno;\
        yylloc.first_column=current_column;\
        yylloc.last_column=current_column+yyleng-1;\
        current_column+=yyleng;\
    }while(false);

// Line and column numbers start from 1
size_t current_column=1;

extern bool kHasLexicalError;

%}

/* Partial Reference: Lexical Analysis with Flex, Appendex A.4 */
/* Identifier */
nondigit_ [_[:alpha:]]
id {nondigit_}([_[:alnum:]])*
/* Integer Literal(better coverage than C--) */
int_suffix_ [uU]([lL]|ll|LL)?
int_suffix_opt_ {int_suffix_}?
literal_int_dec ([[:digit:]]{-}[0])[[:digit:]]*{int_suffix_opt_}
literal_int_hex 0[xX][[:xdigit:]]+{int_suffix_opt_}
literal_int_oct 0[01234567]*{int_suffix_opt_}
/* Decimal Floating Point Literal(better coverage than C--) */
digit_seq_ [[:digit:]]+
digit_seq_opt_ [[:digit:]]*
frac_ ({digit_seq_opt_}\.{digit_seq_})|({digit_seq_}\.)
exp_ [eE][+-]?{digit_seq_}
exp_opt_ {exp_}?
fp_suffix_ [flFL]
fp_suffix_opt_ {fp_suffix_}?
literal_fp_dec ({frac_}{exp_opt_}{fp_suffix_opt_})|({digit_seq_}{exp_}{fp_suffix_opt_})
/* Keywords */
keyword_type_int int
keyword_type_float float
keyword_struct struct
keyword_if if
keyword_else else
keyword_while while
keyword_return return
/* Delimiters */
delimiter_l_bracket \(
delimiter_r_bracket \)
delimiter_l_brace \{
delimiter_r_brace \}
delimiter_l_square \[
delimiter_r_square \]
delimiter_semicolon ;
/* Operators */
operator_dot \.
operator_comma ,
operator_assign =
operator_logical_and &&
operator_logical_or \|\|
operator_logical_not !
operator_add \+
operator_sub -
operator_mul \*
operator_div \/
operator_rel_eq ==
operator_rel_ne !=
operator_rel_gt >
operator_rel_lt <
operator_rel_ge >=
operator_rel_le <=
/* Misc */
line_terminator [\r\n]
whitespace [\t ]

%option yylineno

%%

{keyword_type_int} {CREATE_TOKEN_NODE(TOKEN_KEYWORD_TYPE_INT,TYPE_INT);}
{keyword_type_float} {CREATE_TOKEN_NODE(TOKEN_KEYWORD_TYPE_FLOAT,TYPE_FLOAT);}
{keyword_struct} {CREATE_TOKEN_NODE(TOKEN_KEYWORD_STRUCT,STRUCT);}
{keyword_if} {CREATE_TOKEN_NODE(TOKEN_KEYWORD_IF,IF);}
{keyword_else} {CREATE_TOKEN_NODE(TOKEN_KEYWORD_ELSE,ELSE);}
{keyword_while} {CREATE_TOKEN_NODE(TOKEN_KEYWORD_WHILE,WHILE);}
{keyword_return} {CREATE_TOKEN_NODE(TOKEN_KEYWORD_RETURN,RETURN);}

{id} {CREATE_TOKEN_NODE(TOKEN_ID,ID);}

{literal_int_dec} {CREATE_TOKEN_NODE(TOKEN_LITERAL_INT,LITERAL_INT);}
{literal_int_hex} {CREATE_TOKEN_NODE(TOKEN_LITERAL_INT,LITERAL_INT);}
{literal_int_oct} {CREATE_TOKEN_NODE(TOKEN_LITERAL_INT,LITERAL_INT);}

{literal_fp_dec} {CREATE_TOKEN_NODE(TOKEN_LITERAL_FP,LITERAL_FP);}

{delimiter_l_bracket} {CREATE_TOKEN_NODE(TOKEN_DELIMITER_L_BRACKET,L_BRACKET);}
{delimiter_r_bracket} {CREATE_TOKEN_NODE(TOKEN_DELIMITER_R_BRACKET,R_BRACKET);}
{delimiter_l_brace} {CREATE_TOKEN_NODE(TOKEN_DELIMITER_L_BRACE,L_BRACE);}
{delimiter_r_brace} {CREATE_TOKEN_NODE(TOKEN_DELIMITER_R_BRACE,R_BRACE);}
{delimiter_l_square} {CREATE_TOKEN_NODE(TOKEN_DELIMITER_L_SQUARE,L_SQUARE);}
{delimiter_r_square} {CREATE_TOKEN_NODE(TOKEN_DELIMITER_R_SQUARE,R_SQUARE);}
{delimiter_semicolon} {CREATE_TOKEN_NODE(TOKEN_DELIMITER_SEMICOLON,SEMICOLON);}

{operator_dot} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_DOT,DOT);}
{operator_comma} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_COMMA,COMMA);}
{operator_assign} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_ASSIGN,ASSIGN);}
{operator_logical_and} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_LOGICAL_AND,AND);}
{operator_logical_or} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_LOGICAL_OR,OR);}
{operator_logical_not} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_LOGICAL_NOT,NOT);}
{operator_add} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_ADD,ADD);}
{operator_sub} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_SUB,SUB);}
{operator_mul} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_MUL,MUL);}
{operator_div} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_DIV,DIV);}
{operator_rel_eq} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_REL_EQ,RELOP);}
{operator_rel_ne} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_REL_NE,RELOP);}
{operator_rel_gt} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_REL_GT,RELOP);}
{operator_rel_lt} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_REL_LT,RELOP);}
{operator_rel_ge} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_REL_GE,RELOP);}
{operator_rel_le} {CREATE_TOKEN_NODE(TOKEN_OPERATOR_REL_LE,RELOP);}

{line_terminator} {current_column=1;}
{whitespace} {;}

. {kHasLexicalError=true;
    fprintf(
    stderr,
    "Error type A at line %d: Lexical analyser encountered unexpected '%s' \n",
    yylineno,
    yytext);}
%%
