#include "instruction_generator.h"

const std::string InstructionGenerator::kBinaryOperatorAdd = "+";
const std::string InstructionGenerator::kBinaryOperatorSub = "-";
const std::string InstructionGenerator::kBinaryOperatorMul = "*";
const std::string InstructionGenerator::kBinaryOperatorDiv = "/";
const std::string InstructionGenerator::kBinaryOperatorGt = ">";
const std::string InstructionGenerator::kBinaryOperatorGe = ">=";
const std::string InstructionGenerator::kBinaryOperatorLt = "<";
const std::string InstructionGenerator::kBinaryOperatorLe = "<=";
const std::string InstructionGenerator::kBinaryOperatorEq = "==";
const std::string InstructionGenerator::kBinaryOperatorNe = "!=";

std::string InstructionGenerator::GenerateLabel(const std::string &name) const
{
    return "LABEL " + name + " :";
}

std::string InstructionGenerator::GenerateFunction(const std::string &name) const
{
    return "FUNCTION " + name + " :";
}

std::string InstructionGenerator::GenerateImm(const std::string &number) const
{
    return "#" + number;
}

std::string InstructionGenerator::GenerateAssign(const std::string &left,
                                                 const std::string &right) const
{
    return left + " := " + right;
}

std::string InstructionGenerator::GenerateBinaryOperation(const std::string binary_operator,
                                                          const std::string &left,
                                                          const std::string &right) const
{
    return left + ' ' + binary_operator + ' ' + right;
}

std::string InstructionGenerator::GenerateAddress(const std::string &variable) const
{
    return "&" + variable;
}

std::string InstructionGenerator::GenerateDereference(const std::string &variable) const
{
    return "*" + variable;
}

std::string InstructionGenerator::GenerateGoto(const std::string &label_name) const
{
    return "GOTO " + label_name;
}

std::string InstructionGenerator::GenerateIf(const std::string &condition,
                                             const std::string goto_label_name) const
{
    return "IF " + condition + ' ' + GenerateGoto(goto_label_name);
}

std::string InstructionGenerator::GenerateReturn(const std::string &value) const
{
    return "RETURN " + value;
}

std::string InstructionGenerator::GenerateDec(const std::string &variable, const size_t size) const
{
    return "DEC " + variable + " [" + std::to_string(size) + ']';
}

std::string InstructionGenerator::GenerateArg(const std::string &value) const
{
    return "ARG " + value;
}

std::string InstructionGenerator::GenerateCall(const std::string &function_name) const
{
    return "CALL " + function_name;
}

std::string InstructionGenerator::GenerateParam(const std::string &variable) const
{
    return "PARAM " + variable;
}

std::string InstructionGenerator::GenerateRead(const std::string &variable) const
{
    return "READ " + variable;
}

std::string InstructionGenerator::GenerateWrite(const std::string &value) const
{
    return "WRITE " + value;
}