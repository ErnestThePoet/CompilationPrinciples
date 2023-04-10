#pragma once

#include <vector>

#include "symbol.h"
#include "variable_symbol.h"

class StructDefSymbol : public Symbol
{
private:
    std::vector<VariableSymbolSharedPtr> fields_;

public:
    StructDefSymbol(
        const int line_number,
        const std::string &name,
        const std::vector<VariableSymbolSharedPtr> &fields)
        : Symbol(line_number, name, SymbolType::STRUCT_DEF),
          fields_(fields) {}

    VariableSymbolSharedPtr FieldAt(int index) const
    {
        return fields_[index];
    }

    std::vector<VariableSymbolSharedPtr> Fields() const
    {
        return fields_;
    }
};