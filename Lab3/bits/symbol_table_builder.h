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
#include "./symbols/struct_symbol_def.h"

using SymbolTable = std::unordered_map<std::string, SymbolSharedPtr>;

class SymbolTableBuilder
{
public:
    void Build(KTreeNode *node, size_t, void *user_arg);

    private:
        void DoExtDefList(KTreeNode *node, SymbolTable &symbol_table);
        SymbolSharedPtr DoExtDef(KTreeNode *node);
        SymbolSharedPtr DoVarDec(KTreeNode *node);
};