#include <cstdio>

extern "C"
{
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

void FreeKTreeNode(KTreeNodeValue *node)
{
    AstNodeFree(*node);
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

    SemanticAnalyser semantic_analyser;

    semantic_analyser.Analyse(kRoot);
    if (semantic_analyser.GetHasError())
    {
        KTreeFree(kRoot, FreeKTreeNode);
        return FAILURE;
    }

    KTreeFree(kRoot, FreeKTreeNode);

    return SUCCESS;
}