#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <random>
#include "../../Lab1/bits/k_tree.h"
#include "../../Lab1/bits/token.h"
#include "../../Lab1/bits/variable.h"
#include "./symbols/variable_symbol.h"
#include "./symbols/arithmetic_symbol.h"
#include "./symbols/array_symbol.h"
#include "./symbols/function_symbol.h"
#include "./symbols/struct_symbol.h"
#include "./symbols/struct_def_symbol.h"

using SymbolTable = std::unordered_map<std::string, VariableSymbolSharedPtr>;
using StructDefSymbolTable = std::unordered_map<std::string, StructDefSymbolSharedPtr>;

class SemanticAnalyser
{
private:
    bool has_semantic_error_;
    SymbolTable symbol_table_;
    StructDefSymbolTable struct_def_symbol_table_;
    std::random_device random_device_;
    std::mt19937 mt19937_;
    std::uniform_int_distribution<> distribution_;

    static constexpr int kErrorUndefinedVariable = 1;
    static constexpr int kErrorUndefinedFunction = 2;
    static constexpr int kErrorDuplicateVariableName = 3; // ExtDef impled
    static constexpr int kErrorDuplicateFunctionName = 4;
    static constexpr int kErrorAssignTypeMismatch = 5; // Dec impled
    static constexpr int kErrorAssignToRValue = 6;
    static constexpr int kErrorOperandTypeMismatch = 7;
    static constexpr int kErrorReturnTypeMismatch = 8;
    static constexpr int kErrorFunctionArgsMismatch = 9;
    static constexpr int kErrorInvalidIndexOperator = 10;
    static constexpr int kErrorInvalidInvokeOperator = 11;
    static constexpr int kErrorIndexNotInteger = 12;
    static constexpr int kErrorInvalidDotOperator = 13;
    static constexpr int kErrorUndefinedStructField = 14;
    static constexpr int kErrorDuplicateStructFieldName = 15; // Impled
    static constexpr int kErrorStructFieldInitialized = 15;   // Impled
    static constexpr int kErrorDuplicateStructName = 16;      // Impled
    static constexpr int kErrorUndefinedStruct = 17;          // Impled

public:
    SemanticAnalyser() : has_semantic_error_(false), mt19937_(random_device_()) {}
    void Build(KTreeNode *node, size_t, void *);

    bool HasSemanticError() const
    {
        return has_semantic_error_;
    }

    SymbolTable SymbolTable() const
    {
        return symbol_table_;
    }

    StructDefSymbolTable StructDefSymbolTable() const
    {
        return struct_def_symbol_table_;
    }

private:
    int
    GetLineNumber(const KTreeNode *node) const;
    std::string GetSymbolTypeName(const VariableSymbolSharedPtr &symbol) const;
    std::string GetSymbolTypeName(const VariableSymbol *symbol) const;
    void PrintError(
        const int type, const int line_number, const std::string &message);
    std::string GetNewAnnoyStructName();

    bool CheckAssignmentTypeCompatibility(
        const VariableSymbol &var1,
        const VariableSymbol &var2) const;

    bool CheckStructAssignmentTypeCompatibility(
        const StructDefSymbol &def1,
        const StructDefSymbol &def2) const;

    // Please combine the actual syntax in .y file to fully understand each method
    // Contract: Functions that return a single ptr may return nullptr.
    //           Functions that return a vector of ptr will ensure no nullptr is in that vector.
    void DoExtDefList(KTreeNode *node);
    void DoExtDef(KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoDecListDefCommon(
        const VariableSymbolSharedPtr &specifier,
        const std::vector<VariableSymbolSharedPtr> &dec_list);
    std::vector<VariableSymbolSharedPtr> DoExtDecList(KTreeNode *node);
    VariableSymbolSharedPtr DoSpecifier(KTreeNode *node);
    std::shared_ptr<StructSymbol> DoStructSpecifier(KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoDefList(KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoDef(KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoDecList(KTreeNode *node);
    VariableSymbolSharedPtr DoDec(KTreeNode *node);
    VariableSymbolSharedPtr DoVarDec(KTreeNode *node);
    VariableSymbolSharedPtr DoExp(KTreeNode *node);
};