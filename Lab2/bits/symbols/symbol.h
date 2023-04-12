#pragma once

#include <string>
#include <memory>

#include "symbol_type.h"

/****************************************************************
 *         Inheritance diagram
 *
 *               Symbol
 *                / \
 *              /    \
 *            /       \
 * VariableSymbol     StructDefSymbol
 *  |          \
 *  |    ...    ------------------------------------
 *  |                    \             \            \
 * ArithmeticSymbol ArraySymbol FunctionSymbol StructSymbol
 *
 * All immutable except ArraySymbol can have its elem_type_ set
 *
 *****************************************************************/

class Symbol
{
private:
    int line_number_;
    std::string name_;
    SymbolType symbol_type_;

public:
    Symbol(const int line_number,
           const std::string &name,
           const SymbolType symbol_type)
        : line_number_(line_number),
          name_(name),
          symbol_type_(symbol_type) {}

    int GetLineNumber() const
    {
        return line_number_;
    }

    std::string GetName() const
    {
        return name_;
    }

    SymbolType GetSymbolType() const
    {
        return symbol_type_;
    }
};
