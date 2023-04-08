#pragma once

#include "symbol.h"

class ArithmeticSymbol : public Symbol
{
private:
    ArithmeticSymbolType arithmetic_type;

public:
    ArithmeticSymbol(
        const std::string &name,
        const ArithmeticSymbolType arithmetic_type) : Symbol(name, SymbolType::ARITHMETIC),
                                                      arithmetic_type(arithmetic_type) {}

    ArithmeticSymbolType ArithmeticType() const
    {
        return arithmetic_type;
    }
};