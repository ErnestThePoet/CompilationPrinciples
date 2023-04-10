#pragma once

#include "variable_symbol.h"

class ArithmeticSymbol : public VariableSymbol
{
private:
    ArithmeticSymbolType arithmetic_symbol_type_;

public:
    ArithmeticSymbol(
        const int line_number,
        const std::string &name,
        const ArithmeticSymbolType arithmetic_symbol_type,
        const bool is_initialized = false,
        const VariableSymbolSharedPtr &initial_value = nullptr)
        : VariableSymbol(line_number, name, VariableSymbolType::ARITHMETIC, is_initialized, initial_value),
          arithmetic_symbol_type_(arithmetic_symbol_type) {}

    ArithmeticSymbolType ArithmeticSymbolType() const
    {
        return arithmetic_symbol_type_;
    }
};