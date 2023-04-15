#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../../Lab2/bits/symbols/struct_symbol.h"

#include "exp_value.h"

class StructFieldExpValue : public ExpValue
{
private:
    int field_index_;

public:
    StructFieldExpValue(const std::vector<std::string> &preparation_sequence,
                        const std::string &final_value,
                        const std::shared_ptr<StructSymbol> &source_type,
                        const int field_index) : ExpValue(preparation_sequence, final_value, source_type),
                                                 field_index_(field_index) {}

    int GetFieldIndex() const
    {
        return field_index_;
    }
};
