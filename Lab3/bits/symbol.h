#pragma once

#include <string>

#include "symbol_type.h"

class Symbol
{
private:
    std::string name;
    SymbolType type;

public:
    Symbol(const std::string &name,
           const SymbolType type) : name(name), type(type) {}

    std::string Name() const
    {
        return name;
    }

    SymbolType Type() const
    {
        return type;
    }
};
