#pragma once

#include <vector>

#include "variable_symbol.h"

class FunctionSymbol : public VariableSymbol
{
private:
    std::vector<VariableSymbolSharedPtr> args_;

public:
    FunctionSymbol(
        const int line_number,
        const std::string &name,
        const std::vector<VariableSymbolSharedPtr> &args,
        const bool is_initialized = false,
        const VariableSymbolSharedPtr &initial_value = nullptr)
        : VariableSymbol(line_number, name, VariableSymbolType::FUNCTION, is_initialized, initial_value),
          args_(args) {}

    VariableSymbolSharedPtr ArgAt(int index) const
    {
        return args_[index];
    }

    std::vector<VariableSymbolSharedPtr> Args() const
    {
        return args_;
    }
};