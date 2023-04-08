#pragma once

#include "symbol.h"

class ArraySymbol : public Symbol
{
private:
    Symbol elem_type;
    size_t size;

public:
    ArraySymbol(const std::string &name,
                const Symbol &elem_type,
                const size_t size) : Symbol(name, SymbolType::ARRAY),
                                     elem_type(elem_type),
                                     size(size) {}

    Symbol ElemType() const
    {
        return elem_type;
    }

    size_t Size() const
    {
        return size;
    }
};