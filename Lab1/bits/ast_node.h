#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <stdbool.h>
#include "token.h"
#include "variable.h"

typedef struct
{
    bool is_token;
    union
    {
        Token *token;
        Variable *variable;
    } ast_node_value;
} AstNode;

#endif