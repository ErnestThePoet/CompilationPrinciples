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

        switch (token->type)
        {
        case TOKEN_ID:
        case TOKEN_KEYWORD_TYPE_INT:
        case TOKEN_KEYWORD_TYPE_FLOAT:
        case TOKEN_LITERAL_INT:
        case TOKEN_LITERAL_FP:
            printf("%s: %s\n", token_name_buffer, token->value);
            break;
        default:
            printf("%s\n", token_name_buffer);
            break;
        }
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
    if (argc != 3)
    {
        fputs("Usage: parser <input-file> <output-file>\n", stderr);
        return FAILURE;
    }

    FILE *source_file = fopen(argv[1], "r");
    if (source_file == NULL)
    {
        fprintf(stderr, "Failed to open input file %s\n", argv[1]);
        return FAILURE;
    }

    yyrestart(source_file);
    yyparse();

    fclose(source_file);

    if (!kHasLexicalError && !kHasSyntaxError)
    {
        KTreePreOrderTraverse(kRoot, PrintAstNode);
    }

    FreeKTree(kRoot, FreeKTreeNode);

    return SUCCESS;
}