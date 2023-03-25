#include "ast_node.h"

AstNode *AstNodeCreate(bool is_token,
                       void *ast_node_value)
{
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (node == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    node->is_token = is_token;
    if (is_token)
    {
        node->ast_node_value.token = (Token *)ast_node_value;
    }
    else
    {
        node->ast_node_value.variable = (Variable *)ast_node_value;
    }
    
    return node;
}

void AstNodeFree(AstNode *node)
{
    if (node != NULL)
    {
        if (node->is_token)
        {
            TokenFree(node->ast_node_value.token);
        }
        else
        {
            VariableFree(node->ast_node_value.variable);
        }

        free(node);
    }
}