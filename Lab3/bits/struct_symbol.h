#pragma once

#include "symbol.h"
#include <vector>

class StructSymbol : public Symbol
{
private:
    std::string struct_name;
    std::vector<Symbol> fields;

public:
    StructSymbol(
        const std::string &name,
        const std::string &struct_name,
        const std::vector<Symbol> &fields) : Symbol(name, SymbolType::STRUCT),
                                             struct_name(struct_name),
                                             fields(fields) {}

    std::string StructName() const
    {
        return struct_name;
    }

    Symbol FieldAt(int index) const
    {
        return fields[index];
    }
};