#pragma once

#include <vector>

#include "symbol.h"

class StructSymbol : public Symbol
{
private:
    std::string struct_name;

public:
    StructSymbol(
        const std::string &name,
        const std::string &struct_name) : Symbol(name, SymbolType::STRUCT),
                                          struct_name(struct_name) {}

    std::string StructName() const
    {
        return struct_name;
    }
};