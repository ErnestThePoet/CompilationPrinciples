#pragma once

#include <string>
#include <memory>

#include "symbol_type.h"

class Symbol
{
private:
    std::string name_;
    SymbolType type_;

public:
    Symbol(const std::string &name,
           const SymbolType type) : name_(name), type_(type) {}

    std::string Name() const
    {
        return name_;
    }

    SymbolType Type() const
    {
        return type_;
    }
};

using SymbolSharedPtr = std::shared_ptr<Symbol>;
