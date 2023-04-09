#pragma once

#include <vector>

#include "symbol.h"

class StructSymbol : public Symbol
{
private:
    std::string struct_name_;

public:
    StructSymbol(
        const std::string &name,
        const std::string &struct_name) : Symbol(name, SymbolType::STRUCT),
                                          struct_name_(struct_name) {}

    std::string StructName() const
    {
        return struct_name_;
    }
};