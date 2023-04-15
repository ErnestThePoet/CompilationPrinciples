#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../../Lab2/bits/symbols/variable_symbol.h"
#include "../../Lab2/bits/symbols/array_symbol.h"

#include "exp_value.h"

class ArrayElementExpValue : public ExpValue
{
private:
    // for a[m][n], dim for a[0] is 1 and for a[0][0] is 2
    size_t current_dim_;

    std::vector<size_t> array_dim_sizes_;
    VariableSymbolSharedPtr array_element_type_;
    size_t array_element_size_;

public:
    ArrayElementExpValue(const std::vector<std::string> &preparation_sequence,
                         const std::string &final_value,
                         const std::shared_ptr<ArraySymbol> &source_type,
                         const size_t current_dim,
                         const std::vector<size_t> &array_dim_sizes,
                         const VariableSymbolSharedPtr &array_element_type,
                         const size_t array_element_size)
        : ExpValue(preparation_sequence, final_value, source_type),
          current_dim_(current_dim),
          array_dim_sizes_(array_dim_sizes),
          array_element_type_(array_element_type),
          array_element_size_(array_element_size) {}

    size_t GetCurrentDim() const
    {
        return current_dim_;
    }

    std::vector<size_t> GetArrayDimSizes() const
    {
        return array_dim_sizes_;
    }

    VariableSymbolSharedPtr GetArrayElementType() const
    {
        return array_element_type_;
    }

    size_t GetArrayElementSize() const
    {
        return array_element_size_;
    }
};
