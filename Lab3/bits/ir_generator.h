#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <algorithm>

extern "C"
{
#include "../../Lab1/bits/defs.h"
#include "../../Lab1/bits/k_tree.h"
#include "../../Lab1/bits/token.h"
#include "../../Lab1/bits/variable.h"
}

#include "../../Lab2/bits/semantic_analyser.h"
#include "../../Lab2/bits/symbols/variable_symbol.h"
#include "../../Lab2/bits/symbols/arithmetic_symbol.h"
#include "../../Lab2/bits/symbols/array_symbol.h"
#include "../../Lab2/bits/symbols/function_symbol.h"
#include "../../Lab2/bits/symbols/struct_symbol.h"
#include "../../Lab2/bits/symbols/struct_def_symbol.h"
#include "../../Lab2/bits/symbols/symbol_type.h"

#include "instruction_generator.h"
#include "exp_values/exp_value.h"
#include "exp_values/array_element_exp_value.h"

using IrSequence = std::vector<std::string>;
// <no error, ir sequence>
using IrSequenceGenerationResult = std::pair<bool, IrSequence>;

class IrGenerator
{
private:
    bool has_error_;

    const SymbolTable symbol_table_;
    const StructDefSymbolTable struct_def_symbol_table_;

    // Maps symbol name to IR variable name
    std::unordered_map<std::string, std::string> ir_variable_table_;
    // Maps symbol name to whether it's an address
    // (in C-- this can only be an array/struct parameter)
    std::unordered_map<std::string, bool> is_address_symbol_;

    InstructionGenerator instruction_generator_;

    size_t next_variable_id_;
    size_t next_label_id_;

    IrSequence ir_sequence_;

    const IrSequenceGenerationResult kErrorIrSequenceGenerationResult;

public:
    IrGenerator(const SymbolTable &symbol_table,
                const StructDefSymbolTable &struct_def_symbol_table)
        : has_error_(false),
          symbol_table_(symbol_table),
          struct_def_symbol_table_(struct_def_symbol_table),
          next_variable_id_(0),
          next_label_id_(0),
          kErrorIrSequenceGenerationResult({false, IrSequence()}) {}
    IrGenerator() : IrGenerator(SymbolTable(), StructDefSymbolTable()) {}

    void Generate(const KTreeNode *root);

    bool GetHasError() const
    {
        return has_error_;
    }

    IrSequence GetIrSequence() const
    {
        return ir_sequence_;
    }

private:
    // Debug only
    void PrintKTreeNodeInfo(const KTreeNode *node) const;

    void PrintError(const std::string &message);
    std::string GetNextVariableName();
    std::string GetNextLabelName();
    size_t GetVariableSize(const VariableSymbol &variable) const;
    std::tuple<std::vector<size_t>, VariableSymbolSharedPtr, size_t> GetArrayInfo(
        const ArraySymbol &variable) const;
    std::string GetBinaryOperator(const int type) const;
    bool ShouldPassAddress(const VariableSymbol &variable) const;
    void ConcatenateIrSequence(IrSequence &seq1, const IrSequence &seq2) const;
    void AppendIrSequence(const IrSequence &instruction);

    bool DoExtDefList(const KTreeNode *node);
    bool DoExtDef(const KTreeNode *node);
    IrSequenceGenerationResult DoExtDecList(const KTreeNode *node);
    IrSequenceGenerationResult DoDefList(const KTreeNode *node);
    IrSequenceGenerationResult DoDef(const KTreeNode *node);
    IrSequenceGenerationResult DoDecList(const KTreeNode *node);
    IrSequenceGenerationResult DoDec(const KTreeNode *node);
    std::string DoVarDec(const KTreeNode *node);
    IrSequenceGenerationResult DoFunDec(const KTreeNode *node);
    std::vector<std::string> DoVarList(const KTreeNode *node);
    std::string DoParamDec(const KTreeNode *node);
    IrSequenceGenerationResult DoCompSt(const KTreeNode *node);
    IrSequenceGenerationResult DoStmtList(const KTreeNode *node);
    IrSequenceGenerationResult DoStmt(const KTreeNode *node);
    ExpValueSharedPtr DoExp(const KTreeNode *node,
                            const bool force_singular,
                            const bool singular_no_prefix);
    std::vector<ExpValueSharedPtr> DoArgs(const KTreeNode *node);
};