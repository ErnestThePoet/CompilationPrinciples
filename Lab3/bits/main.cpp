#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "../../Lab1/bits/defs.h"
#include "../../Lab1/bits/token.h"
#include "../../Lab1/bits/ast_node.h"
#include "../../Lab1/bits/k_tree.h"

#include "./symbols/symbol.h"

KTreeNode *kRoot = NULL;
bool kHasLexicalError = false;
bool kHasSyntaxError = false;

extern int yyparse(void);
extern void yyrestart(FILE *input_file);

void FreeKTreeNode(KTreeNodeValue *node)
{
    AstNodeFree(*node);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr<<"Usage: parser <input-file> <output-file>\n";
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
        FreeKTree(kRoot, FreeKTreeNode);
        return FAILURE;
    }

    SymbolTable symbol_table;

    FreeKTree(kRoot, FreeKTreeNode);

    return SUCCESS;
}