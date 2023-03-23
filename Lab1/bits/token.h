#ifndef TOKEN_H_
#define TOKEN_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

#define TOKEN_MAX_NAME_LENGTH 50
#define TOKEN_MAX_VALUE_LENGTH 50

// Token types
#define TOKEN_UNKNOWN 0

#define TOKEN_KEYWORD_RETURN 100
#define TOKEN_KEYWORD_IF 101
#define TOKEN_KEYWORD_ELSE 102
#define TOKEN_KEYWORD_WHILE 103

#define TOKEN_DELIMITER_L_BRACKET 204
#define TOKEN_DELIMITER_R_BRACKET 205
#define TOKEN_DELIMITER_L_BRACE 206
#define TOKEN_DELIMITER_R_BRACE 207
#define TOKEN_DELIMITER_SEMICOLON 208

#define TOKEN_OP_DOT 300
#define TOKEN_OP_COMMA 301
#define TOKEN_OP_ASSIGN 302

#define TOKEN_OP_BITWISE_AND 310
#define TOKEN_OP_BITWISE_OR 311
#define TOKEN_OP_BITWISE_NOT 312
#define TOKEN_OP_BITWISE_XOR 313

#define TOKEN_OP_LOGICAL_AND 320
#define TOKEN_OP_LOGICAL_OR 321
#define TOKEN_OP_LOGICAL_NOT 322
#define TOKEN_OP_LOGICAL_XOR 323

#define TOKEN_OP_ADD 330
#define TOKEN_OP_SUB 331
#define TOKEN_OP_MUL 332
#define TOKEN_OP_DIV 333
#define TOKEN_OP_MOD 334

#define TOKEN_OP_INC 340
#define TOKEN_OP_DEC 341

#define TOKEN_RELOP_EQ 350
#define TOKEN_RELOP_NE 351
#define TOKEN_RELOP_GT 352
#define TOKEN_RELOP_LT 353
#define TOKEN_RELOP_GE 354
#define TOKEN_RELOP_LE 355

typedef struct
{
    size_t line_start = 0;
    size_t column_start = 0;
    int type = TOKEN_UNKNOWN;
    char name[TOKEN_MAX_NAME_LENGTH + 1];
    char value[TOKEN_MAX_VALUE_LENGTH + 1];
} Token;

Token *CreateToken(const size_t line_start,
                   const size_t column_start,
                   const int type,
                   const char *name,
                   const char *value);
void FreeToken(Token *token);

#endif