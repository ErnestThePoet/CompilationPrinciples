%{
#include <stddef.h>
#include <stdbool.h>
#include "../bits/token.h"

#define CREATE_TOKEN(TYPE,BISON_TOKEN) \
do{\
    yylval=CreateToken(yylloc.first_line,yylloc.first_column,TYPE,yytext);\
    return BISON_TOKEN;
}while(false)

#define YYSTYPE Token*

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

%}

/* Partial Reference: Lexical Analysis with Flex, Appendex A.4 */
/* Identifier */
nondigit_ [_[:alpha:]]
id {nondigit_}([_[:alnum:]])*
/* Integer Literal(better coverage than C--) */
literal_int_dec ([[:digit:]]{-}[0])[[:digit:]]*
literal_int_hex 0[xX][[:xdigit:]]+
literal_int_oct 0[01234567]*
/* Decimal Floating Point Literal(better coverage than C--) */
digit_seq_ [:digit:]+
digit_seq_opt_ [:digit:]*
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
operator_div /
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

/* Identifier */
{id} {CREATE_TOKEN(TOKEN_ID,ID);}
/* Integer Literal */
{literal_int_dec} {CREATE_TOKEN(TOKEN_LITERAL_INT,LITERAL_INT);}
{literal_int_hex} {CREATE_TOKEN(TOKEN_LITERAL_INT,LITERAL_INT);}
{literal_int_oct} {CREATE_TOKEN(TOKEN_LITERAL_INT,LITERAL_INT);}
/* Decimal Floating Point Literal */
{literal_fp_dec} {CREATE_TOKEN(TOKEN_LITERAL_FP,LITERAL_FP);}
/* Keywords */
{keyword_type_int} {CREATE_TOKEN(TOKEN_KEYWORD_TYPE_INT,KEYWORD_TYPE_INT);}
{keyword_type_float} {CREATE_TOKEN(TOKEN_KEYWORD_TYPE_FLOAT,KEYWORD_TYPE_FP);}
{keyword_struct} {CREATE_TOKEN(TOKEN_KEYWORD_STRUCT,KEYWORD_STRUCT);}
{keyword_if} {CREATE_TOKEN(TOKEN_KEYWORD_IF,KEYWORD_TYPE_IF);}
{keyword_else} {CREATE_TOKEN(TOKEN_KEYWORD_ELSE,KEYWORD_TYPE_ELSE);}
{keyword_while} {CREATE_TOKEN(TOKEN_KEYWORD_WHILE,KEYWORD_TYPE_WHILE);}
{keyword_return} {CREATE_TOKEN(TOKEN_KEYWORD_RETURN,KEYWORD_TYPE_RETURN);}
/* Delimiters */
{delimiter_l_bracket} 
{delimiter_r_bracket} 
{delimiter_l_brace} 
{delimiter_r_brace} 
{delimiter_l_square} 
{delimiter_r_square} 
{delimiter_semicolon} 
/* Operators */
{operator_dot} 
{operator_comma} 
{operator_assign} 
{operator_logical_and} 
{operator_logical_or} 
{operator_logical_not} 
{operator_add} 
{operator_sub} 
{operator_mul} 
{operator_div} 
{operator_rel_eq} 
{operator_rel_ne} 
{operator_rel_gt} 
{operator_rel_lt} 
{operator_rel_ge} 
{operator_rel_le} 
/* Misc */
{line_terminator} 
{whitespace} 

. {printf(
    "Error type A at Line %d: Lexical analyser encountered unexpected '%s' \n",
    yylineno,
    yytext);}
%%
int main(void){
    yylex();
    return 0;
}