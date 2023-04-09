#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <utility>
#include "../../Lab1/bits/k_tree.h"
#include "../../Lab1/bits/token.h"
#include "../../Lab1/bits/variable.h"
#include "./symbols/symbol.h"
#include "./symbols/arithmetic_symbol.h"
#include "./symbols/array_symbol.h"
#include "./symbols/function_symbol.h"
#include "./symbols/struct_symbol.h"
#include "./symbols/struct_def_symbol.h"

using SymbolTable = std::unordered_map<std::string, SymbolSharedPtr>;

class SymbolTableBuilder
{
private:
    SymbolTable *symbol_table_;

public:
    SymbolTableBuilder(SymbolTable &symbol_table) : symbol_table_(&symbol_table) {}
    void Build(KTreeNode *node, size_t, void *);

private:
    void DoExtDefList(KTreeNode *node);
    SymbolSharedPtr DoExtDef(KTreeNode *node);
    SymbolSharedPtr DoVarDec(KTreeNode *node);
};