#include <cstdio>

extern "C"{
    #include "../Lab1/bits/defs.h"
    #include "../Lab1/bits/token.h"
    #include "../Lab1/bits/ast_node.h"
    #include "../Lab1/bits/k_tree.h"
    #include "../Lab1/generated/lex_analyser.h"
    #include "../Lab1/generated/parser.h"
}

#include "./bits/semantic_analyser.h"

KTreeNode *kRoot = NULL;
bool kHasLexicalError = false;
bool kHasSyntaxError = false;
SemanticAnalyser kSemanticAnalyser;

void FreeKTreeNode(KTreeNodeValue *node)
{
    AstNodeFree(*node);
}

void SemanticAnalyse(KTreeNode *root, size_t current_level, void *user_arg)
{
    kSemanticAnalyser.Analyse(root, current_level, user_arg);
}

void PrintAstNode(KTreeNode *node, size_t current_level, void *)
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
        char variable_name_buffer[VARIABLE_NAME_BUFFER_SIZE];
        GetVariableName(variable_name_buffer, node->value->ast_node_value.variable->type);
        printf("%s (%d)\n",
               variable_name_buffer,
               node->value->ast_node_value.variable->line_start);
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        yyparse();
    }
    else
    {
        FILE *source_file = fopen(argv[1], "r");
        if (source_file == NULL)
        {
            fprintf(stderr, "Failed to open %s\n", argv[1]);
            return FAILURE;
        }

        yyrestart(source_file);
        yyparse();

        fclose(source_file);
    }

    if (kHasLexicalError || kHasSyntaxError)
    {
        KTreeFree(kRoot, FreeKTreeNode);
        return FAILURE;
    }

    KTreePreOrderTraverse(kRoot, PrintAstNode, NULL);
    KTreePreOrderTraverse(kRoot, SemanticAnalyse, NULL);

    if (!kSemanticAnalyser.GetHasSemanticError())
    {
        kSemanticAnalyser.PrintStructDefSymbolTable();
        kSemanticAnalyser.PrintSymbolTable();
    }

    KTreeFree(kRoot, FreeKTreeNode);

    return SUCCESS;
}