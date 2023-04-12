#pragma once

#include "variable_symbol.h"

class ArraySymbol : public VariableSymbol
{
private:
    VariableSymbolSharedPtr elem_type_;
    size_t size_;

public:
    ArraySymbol(const int line_number,
                const std::string &name,
                const VariableSymbolSharedPtr &elem_type,
                const size_t size,
                const bool is_initialized = false,
                const VariableSymbolSharedPtr &initial_value = nullptr)
        : VariableSymbol(line_number, name, VariableSymbolType::ARRAY, is_initialized, initial_value),
          elem_type_(elem_type),
          size_(size) {}

    VariableSymbolSharedPtr GetElemType() const
    {
        return elem_type_;
    }

    void SetElemType(const VariableSymbolSharedPtr &elem_type)
    {
        elem_type_ = elem_type;
    }

    size_t GetSize() const
    {
        return size_;
    }
};