#pragma once

#include <vector>

#include "symbol.h"

class StructDefSymbol : public Symbol
{
private:
    std::vector<SymbolSharedPtr> fields_;

public:
    StructDefSymbol(
        const std::string &name,
        const std::vector<SymbolSharedPtr> &fields) : Symbol(name, SymbolType::STRUCT_DEF),
                                                      fields_(fields) {}

    SymbolSharedPtr FieldAt(int index) const
    {
        return fields_[index];
    }

    std::vector<SymbolSharedPtr> Fields() const
    {
        return fields_;
    }
};