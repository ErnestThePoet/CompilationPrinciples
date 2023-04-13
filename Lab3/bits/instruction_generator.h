#pragma once

#include <string>

class InstructionGenerator
{
public:
    const static std::string kBinaryOperatorAdd;
    const static std::string kBinaryOperatorSub;
    const static std::string kBinaryOperatorMul;
    const static std::string kBinaryOperatorDiv;
    const static std::string kBinaryOperatorGt;
    const static std::string kBinaryOperatorGe;
    const static std::string kBinaryOperatorLt;
    const static std::string kBinaryOperatorLe;
    const static std::string kBinaryOperatorEq;
    const static std::string kBinaryOperatorNe;

    // "number" argument should contain a number literal.
    // "variable" argument should contain a variable name.
    // "value" argument should contain either a number literal or variable name.
    std::string GenerateLabel(const std::string &name) const;
    std::string GenerateFunction(const std::string &name) const;
    std::string GenerateImm(const std::string &number) const;
    std::string GenerateAssign(const std::string &left,
                               const std::string &right) const;
    std::string GenerateBinaryOperation(const std::string binary_operator,
                                        const std::string &left,
                                        const std::string &right) const;
    std::string GenerateAddress(const std::string &variable) const;
    std::string GenerateDereference(const std::string &variable) const;
    std::string GenerateGoto(const std::string &label_name) const;
    std::string GenerateIf(const std::string &condition,
                           const std::string goto_label_name) const;
    std::string GenerateReturn(const std::string &value) const;
    std::string GenerateDec(const std::string &variable, const size_t size) const;
    std::string GenerateArg(const std::string &value) const;
    std::string GenerateCall(const std::string &variable) const;
    std::string GenerateParam(const std::string &variable) const;
    std::string GenerateRead(const std::string &variable) const;
    std::string GenerateWrite(const std::string &value) const;
};