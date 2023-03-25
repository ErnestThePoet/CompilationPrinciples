#include <stdbool.h>
#include <stdio.h>
#include "defs.h"
#include "token.h"
#include "ast_node.h"
#include "k_tree.h"

KTreeNode *kRoot = NULL;
bool kHasLexicalError = false;
bool kHasSyntaxError = false;

extern int yyparse(void);
extern void yyrestart(FILE *input_file);

void FreeKTreeNode(KTreeNodeValue *node)
{
    AstNodeFree(*node);
}

void PrintAstNode(KTreeNode *node, size_t current_level)
{
    for (int i = 0; i < current_level; i++)
    {
        printf("  ");
    }

    if (node->value->is_token)
    {
        char token_name_buffer[TOKEN_NAME_BUFFER_SIZE];
        Token *token = node->value->ast_node_value.token;
        GetTokenName(token_name_buffer, token->type);

        printf("%s: %s\n", token_name_buffer, token->value);
    }
    else
    {
        printf("%s (%d)\n",
               node->value->ast_node_value.variable->name,
               node->value->ast_node_value.variable->line_start);
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        yyparse();
        return SUCCESS;
    }

    FILE *source_file = fopen(argv[1], "r");
    if (source_file == NULL)
    {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return FAILURE;
    }

    yyrestart(source_file);
    yyparse();
    if (!kHasLexicalError && !kHasSyntaxError)
    {
        KTreePreOrderTraverse(kRoot, PrintAstNode);
    }

    fclose(source_file);
    FreeKTree(kRoot, FreeKTreeNode);

    return SUCCESS;
}