#include <stdbool.h>
#include <stdio.h>

#include <list>
#include <unordered_map>

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

    if (kHasLexicalError || kHasSyntaxError)
    {
        FreeKTree(kRoot, FreeKTreeNode);
        return FAILURE;
    }

    FreeKTree(kRoot, FreeKTreeNode);

    return SUCCESS;
}