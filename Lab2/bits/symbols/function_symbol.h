#pragma once

#include <vector>

#include "variable_symbol.h"

class FunctionSymbol : public VariableSymbol
{
private:
    std::vector<VariableSymbolSharedPtr> args_;
    VariableSymbolSharedPtr return_type_;

public:
    FunctionSymbol(
        const int line_number,
        const std::string &name,
        const std::vector<VariableSymbolSharedPtr> &args,
        const VariableSymbolSharedPtr &return_type,
        const bool is_initialized = false,
        const VariableSymbolSharedPtr &initial_value = nullptr)
        : VariableSymbol(line_number,
                         name,
                         VariableSymbolType::FUNCTION,
                         is_initialized,
                         initial_value),
          args_(args),
          return_type_(return_type) {}

    std::vector<VariableSymbolSharedPtr> GetArgs() const
    {
        return args_;
    }

    VariableSymbolSharedPtr GetReturnType() const
    {
        return return_type_;
    }
};