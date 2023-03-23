#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <stddef.h>

#define AST_NODE_MAX_NAME_LENGTH 50
#define AST_NODE_MAX_VALUE_LENGTH 50

typedef struct
{
    size_t line_number = 0;
    char name[AST_NODE_MAX_NAME_LENGTH + 1];
    char value[AST_NODE_MAX_VALUE_LENGTH + 1];
} AstNode;

#endif