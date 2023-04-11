#pragma once

#include <string>
#include <memory>

#include "symbol.h"
#include "symbol_type.h"

class VariableSymbol : public Symbol
{
private:
    VariableSymbolType variable_symbol_type_;
    bool is_initialized_;
    VariableSymbolSharedPtr initial_value_;

public:
    VariableSymbol(const int line_number,
                   const std::string &name,
                   const VariableSymbolType variable_symbol_type,
                   const bool is_initialized = false,
                   const VariableSymbolSharedPtr &initial_value = nullptr)
        : Symbol(line_number, name, SymbolType::VARIABLE),
          variable_symbol_type_(variable_symbol_type),
          is_initialized_(is_initialized),
          initial_value_(initial_value) {}

    VariableSymbolType VariableSymbolType() const
    {
        return variable_symbol_type_;
    }

    bool IsInitialized() const
    {
        return is_initialized_;
    }

    // If is initialized, initial value is not nullptr
    VariableSymbolSharedPtr InitialValue() const
    {
        return initial_value_;
    }
};

using VariableSymbolSharedPtr = std::shared_ptr<VariableSymbol>;
