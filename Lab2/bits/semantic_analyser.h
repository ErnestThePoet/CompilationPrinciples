#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <random>
#include <algorithm>

extern "C"
{
#include "../../Lab1/bits/k_tree.h"
#include "../../Lab1/bits/token.h"
#include "../../Lab1/bits/variable.h"
}

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
    bool is_started_;
    bool has_semantic_error_;
    SymbolTable symbol_table_;
    StructDefSymbolTable struct_def_symbol_table_;
    std::random_device random_device_;
    std::mt19937 mt19937_;
    std::uniform_int_distribution<> distribution_;

    static constexpr int kErrorUndefinedVariable = 1;         // Impled
    static constexpr int kErrorUndefinedFunction = 2;         // Impled
    static constexpr int kErrorDuplicateVariableName = 3;     // Impled
    static constexpr int kErrorDuplicateFunctionName = 4;     // Impled
    static constexpr int kErrorAssignTypeMismatch = 5;        // Impled
    static constexpr int kErrorAssignToRValue = 6;            // Impled
    static constexpr int kErrorOperandTypeMismatch = 7;       // Impled
    static constexpr int kErrorReturnTypeMismatch = 8;        // Impled
    static constexpr int kErrorFunctionArgsMismatch = 9;      // Impled
    static constexpr int kErrorInvalidIndexOperator = 10;     // Impled
    static constexpr int kErrorInvalidInvokeOperator = 11;    // Impled
    static constexpr int kErrorIndexNotInteger = 12;          // Impled
    static constexpr int kErrorInvalidDotOperator = 13;       // Impled
    static constexpr int kErrorUndefinedStructField = 14;     // Impled
    static constexpr int kErrorDuplicateStructFieldName = 15; // Impled
    static constexpr int kErrorStructFieldInitialized = 15;   // Impled
    static constexpr int kErrorDuplicateStructName = 16;      // Impled
    static constexpr int kErrorUndefinedStruct = 17;          // Impled

public:
    SemanticAnalyser() : is_started_(false),
                         has_semantic_error_(false),
                         mt19937_(random_device_()) {}
    void Analyse(const KTreeNode *node);

    // Debug only
    void PrintKTreeNodeInfo(const KTreeNode *node) const;

    void PrintSymbolTable() const;
    void PrintStructDefSymbolTable() const;

    bool GetHasSemanticError() const
    {
        return has_semantic_error_;
    }

    SymbolTable GetSymbolTable() const
    {
        return symbol_table_;
    }

    StructDefSymbolTable GetStructDefSymbolTable() const
    {
        return struct_def_symbol_table_;
    }

private:
    int GetKTreeNodeLineNumber(const KTreeNode *node) const;
    std::string GetVariableSymbolTypeName(const VariableSymbolSharedPtr &symbol) const;
    std::string GetVariableSymbolTypeName(const VariableSymbol *symbol) const;
    void PrintError(
        const int type, const int line_number, const std::string &message);
    std::string GetNewAnnoyStructName();

    bool IsIntArithmeticSymbol(const VariableSymbol &var) const;
    bool IsSameTypeArithmeticSymbol(const VariableSymbol &var1, const VariableSymbol &var2) const;

    bool IsAssignmentValid(
        const VariableSymbol &var_l,
        const VariableSymbol &var_r) const;

    bool IsStructAssignmentValid(
        const StructDefSymbol &def_l,
        const StructDefSymbol &def_r) const;

    bool InsertVariableSymbol(const VariableSymbolSharedPtr &symbol);

    // Refer to C-- syntax defined in Lab1/parser.y for a better understanding of each method
    // Contract: Functions that return a single ptr may return nullptr.
    //           Functions that return a vector of ptr also preserve nullptr in that vector.
    void DoExtDefList(const KTreeNode *node);
    void DoExtDef(const KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoDecListDefCommon(
        const VariableSymbolSharedPtr &specifier,
        const std::vector<VariableSymbolSharedPtr> &dec_list);
    std::vector<VariableSymbolSharedPtr> DoExtDecList(const KTreeNode *node);
    VariableSymbolSharedPtr DoSpecifier(const KTreeNode *node);
    std::shared_ptr<StructSymbol> DoStructSpecifier(const KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoDefList(const KTreeNode *node, const bool should_insert);
    std::vector<VariableSymbolSharedPtr> DoDef(const KTreeNode *node, const bool should_insert);
    std::vector<VariableSymbolSharedPtr> DoDecList(const KTreeNode *node);
    VariableSymbolSharedPtr DoDec(const KTreeNode *node);
    VariableSymbolSharedPtr DoVarDec(const KTreeNode *node);
    std::shared_ptr<FunctionSymbol> DoFunDec(const KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoVarList(const KTreeNode *node);
    VariableSymbolSharedPtr DoParamDec(const KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoCompSt(const KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoStmtList(const KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoStmt(const KTreeNode *node);
    std::pair<VariableSymbolSharedPtr, bool> DoExp(const KTreeNode *node);
    std::vector<VariableSymbolSharedPtr> DoArgs(const KTreeNode *node);
};