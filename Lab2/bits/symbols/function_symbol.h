#pragma once

#include <vector>

#include "symbol.h"

class FunctionSymbol : public Symbol
{
private:
    std::vector<SymbolSharedPtr> args_;

public:
    FunctionSymbol(
        const std::string &name,
        const std::vector<SymbolSharedPtr> &args) : Symbol(name, SymbolType::FUNCTION),
                                                        args_(args) {}

    SymbolSharedPtr ArgAt(int index) const
    {
        return args_[index];
    }

    std::vector<SymbolSharedPtr> Args() const
    {
        return args_;
    }
};