#pragma once

#include <vector>

#include "symbol.h"

class FunctionSymbol : public Symbol
{
private:
    std::vector<SymbolSharedPtr> args;

public:
    FunctionSymbol(
        const std::string &name,
        const std::vector<SymbolSharedPtr> &argsargs) : Symbol(name, SymbolType::FUNCTION),
                                                        args(args) {}

    SymbolSharedPtr ArgAt(int index) const
    {
        return args[index];
    }
};