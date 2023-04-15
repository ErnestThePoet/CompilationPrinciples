#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../../Lab2/bits/symbols/variable_symbol.h"

class ExpValue
{
private:
    std::vector<std::string> preparation_sequence_;
    std::string final_value_;
    VariableSymbolSharedPtr source_type_;

public:
    ExpValue(const std::vector<std::string> &preparation_sequence,
             const std::string &final_value,
             const VariableSymbolSharedPtr &source_type)
        : preparation_sequence_(preparation_sequence),
          final_value_(final_value),
          source_type_(source_type) {}

    std::vector<std::string> GetPreparationSequence() const
    {
        return preparation_sequence_;
    }

    std::string GetFinalValue() const
    {
        return final_value_;
    }

    VariableSymbolSharedPtr GetSourceType() const
    {
        return source_type_;
    }
};

using ExpValueSharedPtr = std::shared_ptr<ExpValue>;
