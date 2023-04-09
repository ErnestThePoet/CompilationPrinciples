#pragma once

#include "symbol.h"

class ArraySymbol : public Symbol
{
private:
    SymbolSharedPtr elem_type_;
    size_t size_;

public:
    ArraySymbol(const std::string &name,
                const SymbolSharedPtr &elem_type,
                const size_t size) : Symbol(name, SymbolType::ARRAY),
                                     elem_type_(elem_type),
                                     size_(size) {}

    SymbolSharedPtr ElemType() const
    {
        return elem_type_;
    }

    size_t Size() const
    {
        return size_;
    }
};