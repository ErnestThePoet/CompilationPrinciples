#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

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
#include "exp_values/struct_field_exp_value.h"

using IrSequence = std::vector<std::string>;

class IrGenerator
{
private:
    bool is_started_;
    bool has_error_;
    const SymbolTable symbol_table_;
    const StructDefSymbolTable struct_def_symbol_table_;

    // Maps symbol name to IR variable name
    std::unordered_map<std::string, std::string> ir_variable_table_;

    InstructionGenerator instruction_generator_;

    size_t next_variable_id_;
    size_t next_label_id_;

    IrSequence ir_sequence_;

public:
    IrGenerator(const SymbolTable &symbol_table,
                const StructDefSymbolTable &struct_def_symbol_table)
        : is_started_(false),
          has_error_(false),
          symbol_table_(symbol_table),
          next_variable_id_(0),
          next_label_id_(0),
          struct_def_symbol_table_(struct_def_symbol_table) {}

    void Generate(const KTreeNode *node);

    bool GetHasError() const
    {
        return has_error_;
    }

    IrSequence GetIrSequence() const
    {
        return ir_sequence_;
    }

private:
    void PrintError(const std::string &message);
    std::string GetNextVariableName();
    std::string GetNextLabelName();
    size_t GetVariableSize(const VariableSymbol &variable) const;
    std::pair<std::vector<size_t>, size_t> GetArrayInfo(const ArraySymbol &variable) const;
    void ConcatenateIrSequence(IrSequence &seq1, const IrSequence &seq2) const;
    void AddIrInstruction(const std::string &instruction);

    void DoExtDefList(const KTreeNode *node);
    void DoExtDef(const KTreeNode *node);
    void DoExtDecList(const KTreeNode *node);
    void DoDefList(const KTreeNode *node);
    void DoDef(const KTreeNode *node);
    void DoDecList(const KTreeNode *node);
    void DoDec(const KTreeNode *node);
    void DoVarDec(const KTreeNode *node);
    void DoFunDec(const KTreeNode *node);
    void DoVarList(const KTreeNode *node);
    void DoParamDec(const KTreeNode *node);
    void DoCompSt(const KTreeNode *node);
    void DoStmtList(const KTreeNode *node);
    void DoStmt(const KTreeNode *node);
    ExpValueSharedPtr DoExp(const KTreeNode *node,
                            const bool force_singular = false);
    std::vector<ExpValueSharedPtr> DoArgs(const KTreeNode *node);
};