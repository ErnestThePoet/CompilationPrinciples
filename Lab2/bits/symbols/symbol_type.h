#pragma once

enum class SymbolType
{
    UNKNOWN,
    VARIABLE,
    STRUCT_DEF,
};

enum class VariableSymbolType
{
    UNKNOWN,
    ARITHMETIC,
    ARRAY,
    STRUCT,
    FUNCTION
};

enum class ArithmeticSymbolType
{
    UNKNOWN,
    INT,
    FLOAT
};
