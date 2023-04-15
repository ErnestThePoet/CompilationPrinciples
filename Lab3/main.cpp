#include <cstdio>
#include <fstream>
#include <memory>

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
#include "./bits/ir_generator.h"

KTreeNode *kRoot = NULL;
bool kHasLexicalError = false;
bool kHasSyntaxError = false;

SemanticAnalyser kSemanticAnalyser(
    {{"read",
      std::make_shared<FunctionSymbol>(
          -1,
          "read",
          std::vector<VariableSymbolSharedPtr>(),
          std::make_shared<ArithmeticSymbol>(
              -1,
              "",
              ArithmeticSymbolType::INT))},
     {"write",
      std::make_shared<FunctionSymbol>(
          -1,
          "write",
          std::vector<VariableSymbolSharedPtr>({std::make_shared<ArithmeticSymbol>(
              -1,
              "value",
              ArithmeticSymbolType::INT)}),
          std::make_shared<ArithmeticSymbol>(
              -1,
              "",
              ArithmeticSymbolType::INT))}});

IrGenerator kIrGenerator;

void FreeKTreeNode(KTreeNodeValue *node)
{
    AstNodeFree(*node);
}

void SemanticAnalyse(KTreeNode *root, size_t, void *)
{
    kSemanticAnalyser.Analyse(root);
}

void GenerateIr(KTreeNode *root, size_t, void *)
{
    kIrGenerator.Generate(root);
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
        std::cerr << "Failed to open input file " << argv[1] << std::endl;
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

    if (kSemanticAnalyser.GetHasError())
    {
        KTreeFree(kRoot, FreeKTreeNode);
        return FAILURE;
    }

    kIrGenerator = IrGenerator(kSemanticAnalyser.GetSymbolTable(),
                               kSemanticAnalyser.GetStructDefSymbolTable());

    KTreePreOrderTraverse(kRoot, GenerateIr, NULL);

    if (kIrGenerator.GetHasError())
    {
        KTreeFree(kRoot, FreeKTreeNode);
        return FAILURE;
    }

    std::ofstream output_file(argv[2], std::ios::out);
    if (!output_file.is_open())
    {
        std::cerr << "Failed to open output file " << argv[2] << std::endl;
        KTreeFree(kRoot, FreeKTreeNode);
        return FAILURE;
    }

    auto ir_sequence = kIrGenerator.GetIrSequence();

    for (auto &ir : ir_sequence)
    {
        output_file << ir << std::endl;
    }

    output_file.close();

    KTreeFree(kRoot, FreeKTreeNode);

    return SUCCESS;
}