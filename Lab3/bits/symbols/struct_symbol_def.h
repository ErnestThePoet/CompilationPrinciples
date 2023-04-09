#pragma once

#include <vector>

#include "symbol.h"

class StructSymbolDef : public Symbol
{
private:
    std::vector<SymbolSharedPtr> fields;

public:
    StructSymbolDef(
        const std::string &name,
        const std::vector<SymbolSharedPtr> &fields) : Symbol(name, SymbolType::STRUCT_DEF),
                                                      fields(fields) {}

    SymbolSharedPtr FieldAt(int index) const
    {
        return fields[index];
    }

    std::vector<SymbolSharedPtr> Fields() const
    {
        return fields;
    }
};