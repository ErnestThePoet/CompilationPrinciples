#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <stdbool.h>
#include <stdlib.h>
#include "defs.h"
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

AstNode *AstNodeCreate(bool is_token,
                       void *ast_node_value);

void AstNodeFree(AstNode *node);

#endif