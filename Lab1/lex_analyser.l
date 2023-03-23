%{
#include <stddef.h>
#include <stdbool.h>
#include "../bits/token.h"

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

letter [a-zA-Z]

%option yylineno

%%
{letter}+ {words++;chars+=yyleng;}
[ ]+ {}
[\r\n] {current_column=1;}
. {printf(
    "Error type A at Line %d: Lexical analyser encountered unexpected '%s' \n",
    yylineno,
    yytext);}
%%
int main(void){
    yylex();
    return 0;
}