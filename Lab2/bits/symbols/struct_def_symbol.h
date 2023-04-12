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

    std::vector<VariableSymbolSharedPtr> Fields() const
    {
        return fields_;
    }
};

using StructDefSymbolSharedPtr = std::shared_ptr<StructDefSymbol>;
