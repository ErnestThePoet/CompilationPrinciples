%{
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../bits/token.h"
#include "../bits/variable.h"
#include "../bits/k_tree.h"
#include "lex_analyser.h"

// LOC(@$) must be explicitly referenced in action section 
// because references macro will not trigger the generation 
// of a declaration of yylloc
// VAL($$) is also compulsory because it will not be transpiled
// inside a macro
#define CREATE_VARIABLE_NODE(LOC,VAL,TYPE,ARGC,...) \
do{\
    Variable* variable=VariableCreate(LOC.first_line,LOC.first_column,TYPE);\
    AstNode* ast_node=AstNodeCreate(false,variable);\
    VAL=KTreeCreateNodeWithChidren(&ast_node,ARGC,##__VA_ARGS__);\
}while(false)

#define CREATE_EMPTY_VARIABLE_NODE(VAL) \
do{\
    VAL=NULL;\
}while(false)

#define MARK_SYNTAX_ERROR \
do{\
    kHasSyntaxError=true;\
}while(false)

extern KTreeNode* kRoot;
extern bool kHasSyntaxError;

void yyerror(const char* msg){
    fprintf(stderr, "Error type B at line %d: %s.\n", yylineno, msg);
}
%}

%union{
    KTreeNode *k_tree_node;
}

%token <k_tree_node> TYPE_INT TYPE_FLOAT
%token <k_tree_node> STRUCT
%token <k_tree_node> IF
%token <k_tree_node> ELSE
%token <k_tree_node> WHILE
%token <k_tree_node> RETURN
%token <k_tree_node> ID
%token <k_tree_node> LITERAL_INT LITERAL_FP
%token <k_tree_node> L_BRACKET R_BRACKET
%token <k_tree_node> L_BRACE R_BRACE
%token <k_tree_node> L_SQUARE R_SQUARE
%token <k_tree_node> SEMICOLON
%token <k_tree_node> DOT
%token <k_tree_node> COMMA
%token <k_tree_node> ASSIGN
%token <k_tree_node> AND OR NOT
%token <k_tree_node> ADD SUB MUL DIV
%token <k_tree_node> RELOP

%type <k_tree_node> Program
%type <k_tree_node> ExtDefList ExtDef DefList Def
%type <k_tree_node> ExtDecList DecList Dec
%type <k_tree_node> Specifier StructSpecifier
%type <k_tree_node> VarDec FunDec ParamDec
%type <k_tree_node> VarList
%type <k_tree_node> CompSt
%type <k_tree_node> StmtList Stmt
%type <k_tree_node> Exp Args
%type <k_tree_node> OptTag Tag

// First declaration has lowest precedence
%right ASSIGN
%left OR
%left AND
%left RELOP
%left ADD SUB
%left MUL DIV
%right NOT
%left DOT
%left L_BRACKET R_BRACKET
%left L_BRACE R_BRACE
%left L_SQUARE R_SQUARE
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%define parse.error detailed

%%
Program:ExtDefList {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_PROGRAM,1,$1);kRoot=$$;}
    ;

ExtDefList: ExtDef ExtDefList {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXT_DEF_LIST, 2, $1, $2);}
    | {CREATE_EMPTY_VARIABLE_NODE($$);} 
    ; 
ExtDef: Specifier ExtDecList SEMICOLON {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXT_DEF, 3, $1, $2, $3);}
    | Specifier SEMICOLON {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXT_DEF, 2, $1, $2);}
    | Specifier FunDec CompSt {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXT_DEF, 3, $1, $2, $3);}
    | error SEMICOLON {MARK_SYNTAX_ERROR;}
    ; 
ExtDecList: VarDec {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXT_DEC_LIST, 1, $1);}
    | VarDec COMMA ExtDecList {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXT_DEC_LIST, 3, $1, $2, $3);}
    ; 

Specifier: TYPE_INT {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_SPECIFIER, 1, $1);}
    | TYPE_FLOAT {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_SPECIFIER, 1, $1);}
    | StructSpecifier {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_SPECIFIER, 1, $1);}
    ; 
StructSpecifier: STRUCT OptTag L_BRACE DefList R_BRACE {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STRUCT_SPECIFIER, 5, $1, $2, $3, $4, $5);}
    | STRUCT Tag {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STRUCT_SPECIFIER, 2, $1, $2);}
    ; 
OptTag: ID {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_OPT_TAG, 1, $1);}
    | {CREATE_EMPTY_VARIABLE_NODE($$);}
    ; 
Tag: ID {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_TAG, 1, $1);}
    ; 

VarDec: ID {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_VAR_DEC, 1, $1);}
    | VarDec L_SQUARE LITERAL_INT R_SQUARE {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_VAR_DEC, 4, $1, $2, $3, $4);}
    | error R_SQUARE {MARK_SYNTAX_ERROR;}
    ; 
FunDec: ID L_BRACKET VarList R_BRACKET {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_FUN_DEC, 4, $1, $2, $3, $4);}
    | ID L_BRACKET R_BRACKET {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_FUN_DEC, 3, $1, $2, $3);}
    | error R_BRACKET {MARK_SYNTAX_ERROR;}
    ; 
VarList: ParamDec COMMA VarList {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_VAR_LIST, 3, $1, $2, $3);}
    | ParamDec {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_VAR_LIST, 1, $1);}
    ; 
ParamDec: Specifier VarDec {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_PARAM_DEC, 2, $1, $2);}
    ; 

CompSt: L_BRACE DefList StmtList R_BRACE {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_COMP_ST, 4, $1, $2, $3, $4);}
    | error R_BRACE {MARK_SYNTAX_ERROR;}
    ; 
StmtList: Stmt StmtList {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STMT_LIST, 2, $1, $2);}
    | {CREATE_EMPTY_VARIABLE_NODE($$);}
    ; 
Stmt: Exp SEMICOLON {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STMT, 2, $1, $2);}
    | CompSt {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STMT, 1, $1);}
    | RETURN Exp SEMICOLON {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STMT, 3, $1, $2, $3);} 
    | IF L_BRACKET Exp R_BRACKET Stmt %prec LOWER_THAN_ELSE {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STMT, 5, $1, $2, $3, $4, $5);}
    | IF L_BRACKET Exp R_BRACKET Stmt ELSE Stmt {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STMT, 7, $1, $2, $3, $4, $5, $6, $7);}
    | WHILE L_BRACKET Exp R_BRACKET Stmt {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_STMT, 5, $1, $2, $3, $4, $5);}
    | error SEMICOLON {MARK_SYNTAX_ERROR;}
    ; 

DefList: Def DefList {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_DEF_LIST, 2, $1, $2);}
    | {CREATE_EMPTY_VARIABLE_NODE($$);}
    ; 
Def: Specifier DecList SEMICOLON {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_DEF, 3, $1, $2, $3);}
    ; 
DecList: Dec {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_DEC_LIST, 1, $1);}
    | Dec COMMA DecList {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_DEC_LIST, 3, $1, $2, $3);}
    ; 
Dec: VarDec {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_DEC, 1, $1);}
    | VarDec ASSIGN Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_DEC, 3, $1, $2, $3);}
    ; 

Exp: Exp ASSIGN Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | Exp AND Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | Exp OR Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | Exp RELOP Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | Exp ADD Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | Exp SUB Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | Exp MUL Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | Exp DIV Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | L_BRACKET Exp R_BRACKET {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | SUB Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 2, $1, $2);}
    | NOT Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 2, $1, $2);}
    | ID L_BRACKET R_BRACKET {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | ID L_BRACKET Args R_BRACKET {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 4, $1, $2, $3, $4);}
    | Exp L_SQUARE Exp R_SQUARE {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 4, $1, $2, $3, $4);}
    | Exp DOT ID {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 3, $1, $2, $3);}
    | ID {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 1, $1);}
    | LITERAL_INT {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 1, $1);}
    | LITERAL_FP {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_EXP, 1, $1);}
    ; 

Args: Exp COMMA Args {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_ARGS, 3, $1, $2, $3);}
    | Exp {CREATE_VARIABLE_NODE(@$,$$,VARIABLE_ARGS, 1, $1);}
    ; 

%%
