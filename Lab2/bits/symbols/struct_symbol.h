#pragma once

#include <vector>

#include "variable_symbol.h"

class StructSymbol : public VariableSymbol
{
private:
    std::string struct_name_;

public:
    StructSymbol(
        const int line_number,
        const std::string &name,
        const std::string &struct_name,
        const bool is_initialized = false,
        const VariableSymbolSharedPtr &initial_value = nullptr)
        : VariableSymbol(line_number, name, VariableSymbolType::STRUCT, is_initialized, initial_value),
          struct_name_(struct_name) {}

    std::string GetStructName() const
    {
        return struct_name_;
    }
};