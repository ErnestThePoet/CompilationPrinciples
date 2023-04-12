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

#include "../Lab2/bits/semantic_analyser.h"

KTreeNode *kRoot = NULL;
bool kHasLexicalError = false;
bool kHasSyntaxError = false;
SemanticAnalyser kSemanticAnalyser;

void FreeKTreeNode(KTreeNodeValue *node)
{
    AstNodeFree(*node);
}

void SemanticAnalyse(KTreeNode *root, size_t, void *)
{
    kSemanticAnalyser.Analyse(root);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: parser <input-file-path> <output-file-path>" << std::endl;
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

    if (kHasLexicalError || kHasSyntaxError)
    {
        KTreeFree(kRoot, FreeKTreeNode);
        return FAILURE;
    }

    KTreePreOrderTraverse(kRoot, SemanticAnalyse, NULL);

    // if (!kSemanticAnalyser.GetHasError())
    // {
    //     kSemanticAnalyser.PrintStructDefSymbolTable();
    //     kSemanticAnalyser.PrintSymbolTable();
    // }

    KTreeFree(kRoot, FreeKTreeNode);

    return SUCCESS;
}