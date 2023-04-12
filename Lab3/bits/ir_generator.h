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

class IrGenerator
{
private:
    bool is_started_;
    bool has_error_;
    const SymbolTable symbol_table_;
    const StructDefSymbolTable struct_def_symbol_table_;

public:
    IrGenerator(const SymbolTable &symbol_table,
                const StructDefSymbolTable &struct_def_symbol_table)
        : is_started_(false),
          has_error_(false),
          symbol_table_(symbol_table),
          struct_def_symbol_table_(struct_def_symbol_table) {}

    bool GetHasError() const{
        return has_error_;
    }

    bool 
};