#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <random>
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

class SemanticAnalyser
{
private:
    SymbolTable symbol_table_;
    std::random_device random_device_;
    std::mt19937 mt19937_;
    std::uniform_int_distribution<> distribution_;

    static constexpr int kErrorUndefinedVariable = 1;
    static constexpr int kErrorUndefinedFunction = 2;
    static constexpr int kErrorDuplicateVariableName = 3;
    static constexpr int kErrorDuplicateFunctionName = 4;
    static constexpr int kErrorAssignTypeMismatch = 5;
    static constexpr int kErrorAssignToRValue = 6;
    static constexpr int kErrorOperandTypeMismatch = 7;
    static constexpr int kErrorReturnTypeMismatch = 8;
    static constexpr int kErrorFunctionArgsMismatch = 9;
    static constexpr int kErrorInvalidIndexOperator = 10;
    static constexpr int kErrorInvalidInvokeOperator = 11;
    static constexpr int kErrorIndexNotInteger = 12;
    static constexpr int kErrorInvalidDotOperator = 13;
    static constexpr int kErrorUndefinedStructField = 14;
    static constexpr int kErrorDuplicateStructFieldName = 15;
    static constexpr int kErrorStructFieldInitialized = 15;
    static constexpr int kErrorDuplicateStructName = 16;
    static constexpr int kErrorUndefinedStruct = 17;

public:
    SemanticAnalyser() : mt19937_(random_device_()) {}
    void Build(KTreeNode *node, size_t, void *);

    SymbolTable SymbolTable()
    {
        return symbol_table_;
    }

private:
    void PrintError(
        const int type, const KTreeNode *node, const std::string &message) const;
    std::string GetNewAnnoyStructName();
    void DoExtDefList(KTreeNode *node);
    void DoExtDef(KTreeNode *node);
    SymbolSharedPtr DoSpecifier(KTreeNode *node);
    SymbolSharedPtr DoStructSpecifier(KTreeNode *node);
    std::vector<SymbolSharedPtr> DoDefList(KTreeNode *node);
    std::vector<SymbolSharedPtr> DoDef(KTreeNode *node);
    std::vector<SymbolSharedPtr> DoDecList(KTreeNode *node);
    SymbolSharedPtr DoDec(KTreeNode *node);
    SymbolSharedPtr DoVarDec(KTreeNode *node);
};