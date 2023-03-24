#ifndef TOKEN_H_
#define TOKEN_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

#define TOKEN_MAX_VALUE_LENGTH 50
#define TOKEN_NAME_BUFFER_SIZE 30

// Token types
#define TOKEN_UNKNOWN -1

#define TOKEN_ID 0

#define TOKEN_LITERAL_INT 1
#define TOKEN_LITERAL_FP 2

#define TOKEN_KEYWORD_RETURN 100
#define TOKEN_KEYWORD_IF 101
#define TOKEN_KEYWORD_ELSE 102
#define TOKEN_KEYWORD_WHILE 103
#define TOKEN_KEYWORD_STRUCT 104
#define TOKEN_KEYWORD_TYPE_INT 105
#define TOKEN_KEYWORD_TYPE_FLOAT 106

#define TOKEN_DELIMITER_L_BRACKET 201
#define TOKEN_DELIMITER_R_BRACKET 202
#define TOKEN_DELIMITER_L_BRACE 203
#define TOKEN_DELIMITER_R_BRACE 204
#define TOKEN_DELIMITER_L_SQUARE 205
#define TOKEN_DELIMITER_R_SQUARE 206
#define TOKEN_DELIMITER_SEMICOLON 207

#define TOKEN_OPERATOR_DOT 300
#define TOKEN_OPERATOR_COMMA 301
#define TOKEN_OPERATOR_ASSIGN 302

#define TOKEN_OPERATOR_LOGICAL_AND 310
#define TOKEN_OPERATOR_LOGICAL_OR 311
#define TOKEN_OPERATOR_LOGICAL_NOT 312

#define TOKEN_OPERATOR_ADD 320
#define TOKEN_OPERATOR_SUB 321
#define TOKEN_OPERATOR_MUL 322
#define TOKEN_OPERATOR_DIV 323

#define TOKEN_OPERATOR_REL_EQ 330
#define TOKEN_OPERATOR_REL_NE 331
#define TOKEN_OPERATOR_REL_GT 332
#define TOKEN_OPERATOR_REL_LT 333
#define TOKEN_OPERATOR_REL_GE 334
#define TOKEN_OPERATOR_REL_LE 335

typedef struct
{
    int line_start;
    int column_start;
    int type;
    char value[TOKEN_MAX_VALUE_LENGTH + 1];
} Token;

Token *CreateToken(const int line_start,
                   const int column_start,
                   const int type,
                   const char *value);

void FreeToken(Token *token);

void GetTokenName(char *buffer, const int type);

#endif