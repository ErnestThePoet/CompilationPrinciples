#pragma once

#include "symbol.h"
#include <vector>

class FunctionSymbol : public Symbol
{
private:
    std::vector<Symbol> args;

public:
    FunctionSymbol(
        const std::string &name,
        const std::vector<Symbol> &argsargs) : Symbol(name, SymbolType::FUNCTION),
                                               args(args) {}

    Symbol ArgAt(int index) const
    {
        return args[index];
    }
};