#pragma once

#include <iostream>
#include <string>
#include <vector>

extern "C"
{
#include "../../Lab1/bits/k_tree.h"
#include "../../Lab1/bits/token.h"
#include "../../Lab1/bits/variable.h"
}

#include "../../Lab2/bits/semantic_analyser.h"

using IrSequence = std::vector<std::string>;

class IrGenerator
{
private:
    bool is_started_;
    bool has_error_;
    const SymbolTable symbol_table_;
    const StructDefSymbolTable struct_def_symbol_table_;

    IrSequence ir_sequence_;

public:
    IrGenerator(const SymbolTable &symbol_table,
                const StructDefSymbolTable &struct_def_symbol_table)
        : is_started_(false),
          has_error_(false),
          symbol_table_(symbol_table),
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
    void AddIrInstruction(const std::string &instruction);

    void DoExtDefList(const KTreeNode *node);
    void DoExtDef(const KTreeNode *node);
    void DoExtDecList(const KTreeNode *node);
    void DoSpecifier(const KTreeNode *node);
    void DoStructSpecifier(const KTreeNode *node);
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
    void DoExp(const KTreeNode *node);
    void DoArgs(const KTreeNode *node);
};