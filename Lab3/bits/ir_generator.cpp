#include "ir_generator.h"

void IrGenerator::PrintError(const std::string &message)
{
    has_error_ = true;
    std::cerr << "IR translation error : " << message << std::endl;
}

std::string IrGenerator::GetNextVariableName()
{
    return "var" + std::to_string(next_variable_id_++);
}

std::string IrGenerator::GetNextLabelName()
{
    return "label" + std::to_string(next_label_id_++);
}

size_t IrGenerator::GetVariableSize(const VariableSymbol &variable) const
{
    switch (variable.GetVariableSymbolType())
    {
    case VariableSymbolType::ARITHMETIC:
    {
        switch (static_cast<const ArithmeticSymbol *>(&variable)->GetArithmeticSymbolType())
        {
        case ArithmeticSymbolType::INT:
            return 4;
        case ArithmeticSymbolType::FLOAT:
            return 4;
        default:
            return 0;
        }
    }
    case VariableSymbolType::ARRAY:
    {
        auto current_dim_type = &variable;

        size_t element_count = 1;

        while (current_dim_type->GetVariableSymbolType() == VariableSymbolType::ARRAY)
        {
            auto current_array = static_cast<const ArraySymbol *>(current_dim_type);
            element_count *= current_array->GetSize();
            current_dim_type = current_array->GetElemType().get();
        }

        size_t element_size = GetVariableSize(*current_dim_type);

        return element_count * element_size;
    }
    case VariableSymbolType::STRUCT:
    {
        size_t struct_size = 0;

        auto struct_def = struct_def_symbol_table_.at(
            static_cast<const StructSymbol *>(&variable)->GetStructName());

        for (auto &field : struct_def->GetFields())
        {
            struct_size += GetVariableSize(*field);
        }

        return struct_size;
    }
    default:
        return 0;
    }
}

// Returns <array dim sizes, element size>
std::pair<std::vector<size_t>, size_t> IrGenerator::GetArrayInfo(const ArraySymbol &array_variable) const
{
    const VariableSymbol *current_dim_type = static_cast<const VariableSymbol *>(&array_variable);

    std::vector<size_t> dim_sizes;

    while (current_dim_type->GetVariableSymbolType() == VariableSymbolType::ARRAY)
    {
        auto current_array = static_cast<const ArraySymbol *>(current_dim_type);
        dim_sizes.push_back(current_array->GetSize());
        current_dim_type = current_array->GetElemType().get();
    }

    return {dim_sizes, GetVariableSize(*current_dim_type)};
}

void IrGenerator::ConcatenateIrSequence(IrSequence &seq1, const IrSequence &seq2) const
{
    seq1.insert(seq1.cend(), seq2.cbegin(), seq2.cend());
}

void IrGenerator::AddIrInstruction(const std::string &instruction)
{
    ir_sequence_.push_back(instruction);
}

void IrGenerator::DoExtDefList(const KTreeNode *node)
{
    // ExtDefList: ExtDef ExtDefList(Nullable) | <NULL>
    while (node != NULL)
    {
        DoExtDef(node->l_child);
        node = node->r_child;
    }
}

void IrGenerator::DoExtDef(const KTreeNode *node)
{
    // ExtDef: Specifier SEMICOLON
    // ignored

    // ExtDef: Specifier ExtDecList SEMICOLON
    if (node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_EXT_DEC_LIST)
    {
        DoExtDecList(node->l_child->r_sibling);
    }
    // ExtDef: Specifier FunDec CompSt
    else
    {
        DoFunDec(node->l_child->r_sibling);

        DoCompSt(node->l_child->r_sibling->r_sibling);
    }
}

void IrGenerator::DoExtDecList(const KTreeNode *node)
{
    // ExtDecList: VarDec | VarDec COMMA ExtDecList
    while (node != NULL)
    {
        DoVarDec(node->l_child);
        node = node->r_child;
    }
}

void IrGenerator::DoDefList(const KTreeNode *node)
{
    // DefList: Def DefList(Nullable) | <NULL>
    while (node != NULL)
    {
        DoDef(node->l_child);
        node = node->r_child;
    }
}

void IrGenerator::DoDef(const KTreeNode *node)
{
    // Def: Specifier DecList SEMICOLON
    DoDecList(node->l_child->r_sibling);
}

void IrGenerator::DoDecList(const KTreeNode *node)
{
    // DecList: Dec | Dec COMMA DecList
    while (node != NULL)
    {
        DoDec(node->l_child);
        node = node->r_child;
    }
}

void IrGenerator::DoDec(const KTreeNode *node)
{
    // Dec: VarDec | VarDec ASSIGN Exp
    DoVarDec(node->l_child);

    if (node->l_child->r_sibling == NULL)
    {
        return;
    }

    DoExp(node->l_child->r_sibling->r_sibling);
}

void IrGenerator::DoVarDec(const KTreeNode *node)
{
    // VarDec: ID
    if (node->l_child->value->is_token &&
        node->l_child->value->ast_node_value.token->type == TOKEN_ID)
    {
        return;
    }

    // VarDec: VarDec L_SQUARE LITERAL_INT R_SQUARE

    // Note that int a[2][3] should be interpreted as array<array<int,3>,2>,
    // But ArraySymbol views it as array<array<int,2>,3>
    DoVarDec(node->l_child);
}

void IrGenerator::DoFunDec(const KTreeNode *node)
{
    std::string function_name = node->l_child->value->ast_node_value.token->value;

    // FunDec: ID L_BRACKET R_BRACKET
    if (node->l_child->r_sibling->r_sibling->value->is_token)
    {
        return;
    }
    // FunDec: ID L_BRACKET VarList R_BRACKET
    else
    {
        DoVarList(node->l_child->r_sibling->r_sibling);
    }
}

void IrGenerator::DoVarList(const KTreeNode *node)
{
    // VarList: ParamDec COMMA VarList | ParamDec
    while (node != NULL)
    {
        DoParamDec(node->l_child);
        node = node->r_child;
    }
}

void IrGenerator::DoParamDec(const KTreeNode *node)
{
    // ParamDec: Specifier VarDec
    DoVarDec(node->r_child);
}

void IrGenerator::DoCompSt(const KTreeNode *node)
{
    // CompSt: L_BRACE DefList(Nullable) StmtList(Nullable) R_BRACE

    // At least one of DefList and StmtList is not NULL
    if (!node->l_child->r_sibling->value->is_token)
    {
        // DefList is not NULL
        if (node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_DEF_LIST)
        {
            DoDefList(node->l_child->r_sibling);

            // StmtList is not NULL either
            if (!node->l_child->r_sibling->r_sibling->value->is_token)
            {
                DoStmtList(node->l_child->r_sibling->r_sibling);
            }
            // StmtList is NULL
            else
            {
                return;
            }
        }
        // DefList is NULL and StmtList is not NULL
        else
        {
            DoStmtList(node->l_child->r_sibling);
        }
    }

    // Both DefList and StmtList are NULL
}

void IrGenerator::DoStmtList(const KTreeNode *node)
{
    // StmtList: Stmt StmtList(Nullable) | <NULL>
    while (node != NULL)
    {
        DoStmt(node->l_child);
        node = node->r_child;
    }
}

void IrGenerator::DoStmt(const KTreeNode *node)
{
    if (!node->l_child->value->is_token)
    {
        // Stmt: Exp SEMICOLON
        if (node->l_child->value->ast_node_value.variable->type == VARIABLE_EXP)
        {
            DoExp(node->l_child);
            return;
        }

        // Stmt: CompSt
        DoCompSt(node->l_child);
        return;
    }

    switch (node->l_child->value->ast_node_value.token->type)
    {
    // Stmt: RETURN Exp SEMICOLON
    case TOKEN_KEYWORD_RETURN:
    {
        DoExp(node->l_child->r_sibling);
        return;
    }
    case TOKEN_KEYWORD_IF:
    {
        auto condition_exp_node = node->l_child->r_sibling->r_sibling;
        auto if_stmt_node = condition_exp_node->r_sibling->r_sibling;

        DoExp(condition_exp_node);

        // Stmt: IF L_BRACKET Exp R_BRACKET Stmt
        DoStmt(if_stmt_node);

        if (if_stmt_node->r_sibling == NULL)
        {
            return;
        }

        // Stmt: IF L_BRACKET Exp R_BRACKET Stmt ELSE Stmt
        DoStmt(if_stmt_node->r_sibling->r_sibling);

        return;
    }
    // Stmt: WHILE L_BRACKET Exp R_BRACKET Stmt
    case TOKEN_KEYWORD_WHILE:
    {
        DoExp(node->l_child->r_sibling->r_sibling);

        DoStmt(node->r_child);

        return;
    }
    default:
        return;
    }
}

// Returns the expression's final value and the leading preparation IR sequence.
// If force_singular is true, then GetFinalValue() of returned ExpValue
// is guarenteed to be either a variable(may be prefixed with & or *) or an imm.
// Otherwise, GetFinalValue() may contain a binary operation in the form of x op y.
// When translating an array indexing, calls before the final dim will have
// GetFinalValue() in return value being the starting address to next dim.
// If translation error happens, nullptr is returned.
ExpValueSharedPtr IrGenerator::DoExp(const KTreeNode *node,
                                     const bool force_singular = false)
{
    if (node->l_child->value->is_token)
    {
        // Exp: ID | LITERAL_INT | LITERAL_FP /////////////////////////////
        if (node->r_child == NULL)
        {
            std::string token_value = node->l_child->value->ast_node_value.token->value;

            switch (node->l_child->value->ast_node_value.token->type)
            {
            case TOKEN_ID:
            {
                auto symbol = symbol_table_.at(token_value);

                switch (symbol->GetVariableSymbolType())
                {
                // 1D array can be arg, after indexing can be lval/rval.
                // MD array after indexing can be lval/rval.
                // In all cases we start with its address.
                case VariableSymbolType::ARRAY:
                {
                    auto array_info = GetArrayInfo(*static_cast<ArraySymbol *>(symbol.get()));

                    return std::make_shared<ArrayElementExpValue>(
                        IrSequence(),
                        instruction_generator_.GenerateAddress(
                            ir_variable_table_.at(token_value)),
                        symbol,
                        0,
                        array_info.first,
                        array_info.second);
                }
                // Struct can be arg, after selecting field can be lval/rval.
                // In all cases we start with its address.
                case VariableSymbolType::STRUCT:
                {
                    return std::make_shared<ExpValue>(
                        IrSequence(),
                        instruction_generator_.GenerateAddress(
                            ir_variable_table_.at(token_value)),
                        symbol);
                }
                default:
                {
                    return std::make_shared<ExpValue>(
                        IrSequence(),
                        ir_variable_table_.at(token_value),
                        symbol);
                }
                }
            }
            case TOKEN_LITERAL_INT:
            {
                return std::make_shared<ExpValue>(
                    IrSequence(),
                    instruction_generator_.GenerateImm(token_value),
                    std::make_shared<ArithmeticSymbol>(
                        -1,
                        "",
                        ArithmeticSymbolType::INT));
            }
            case TOKEN_LITERAL_FP:
            {
                return std::make_shared<ExpValue>(
                    IrSequence(),
                    instruction_generator_.GenerateImm(token_value),
                    std::make_shared<ArithmeticSymbol>(
                        -1,
                        "",
                        ArithmeticSymbolType::FLOAT));
            }
            default:
                return nullptr;
            }
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: ID L_BRACKET R_BRACKET | ID L_BRACKET Args R_BRACKET //////
        if (node->l_child->value->ast_node_value.token->type == TOKEN_ID &&
            node->l_child->r_sibling != NULL &&
            node->l_child->r_sibling->value->is_token &&
            node->l_child->r_sibling->value->ast_node_value.token->type ==
                TOKEN_DELIMITER_L_BRACKET)
        {
            std::string function_name = node->l_child->value->ast_node_value.token->value;
            auto return_type = static_cast<FunctionSymbol *>(symbol_table_.at(function_name).get())->GetReturnType();

            if (return_type->GetVariableSymbolType() != VariableSymbolType::ARITHMETIC)
            {
                PrintError("Function can only return arithmetic type");
                return nullptr;
            }

            IrSequence preparation_sequence;

            // Call with args
            auto args_node = node->l_child->r_sibling->r_sibling;
            if (!args_node->value->is_token)
            {
                auto args = DoArgs(args_node);

                // Concatenate preparation code
                for (auto &arg : args)
                {
                    if (arg == nullptr)
                    {
                        return nullptr;
                    }

                    auto arg_preparation_sequence = arg->GetPreparationSequence();
                    preparation_sequence.insert(preparation_sequence.cend(),
                                                arg_preparation_sequence.cbegin(),
                                                arg_preparation_sequence.cend());
                }

                // Special treat: write
                if (function_name == "write")
                {
                    preparation_sequence.push_back(
                        instruction_generator_.GenerateWrite(args[0]->GetFinalValue()));
                    return std::make_shared<ExpValue>(
                        preparation_sequence,
                        instruction_generator_.GenerateImm(0),
                        return_type);
                }

                // Retrieve arg singular exp in reverse order
                for (auto i = args.rbegin(); i != args.rend(); ++i)
                {
                    preparation_sequence.push_back(
                        instruction_generator_.GenerateArg((*i)->GetFinalValue()));
                }
            }

            // Special treat: read
            if (function_name == "read")
            {
                auto read_variable_name = GetNextVariableName();
                preparation_sequence.push_back(
                    instruction_generator_.GenerateRead(read_variable_name));
                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    read_variable_name,
                    return_type);
            }

            // Shared logic with call with no args
            if (force_singular)
            {
                auto return_value_variable_name = GetNextVariableName();
                preparation_sequence.push_back(
                    instruction_generator_.GenerateAssign(
                        return_value_variable_name,
                        instruction_generator_.GenerateCall(function_name)));

                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    return_value_variable_name,
                    return_type);
            }
            else
            {
                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    instruction_generator_.GenerateCall(function_name),
                    return_type);
            }
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: SUB Exp ///////////////////////////////////////////////////
        if (node->l_child->value->ast_node_value.token->type == TOKEN_OPERATOR_SUB)
        {
            auto expression = DoExp(node->r_child, true);
            IrSequence preparation_sequence = expression->GetPreparationSequence();

            if (force_singular)
            {
                auto variable_name = GetNextVariableName();
                preparation_sequence.push_back(
                    instruction_generator_.GenerateAssign(
                        variable_name,
                        instruction_generator_.GenerateBinaryOperation(
                            InstructionGenerator::kBinaryOperatorSub,
                            instruction_generator_.GenerateImm(0),
                            expression->GetFinalValue())));

                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    variable_name,
                    expression->GetSourceType());
            }
            else
            {
                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    instruction_generator_.GenerateBinaryOperation(
                        InstructionGenerator::kBinaryOperatorSub,
                        instruction_generator_.GenerateImm(0),
                        expression->GetFinalValue()),
                    expression->GetSourceType());
            }
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: NOT Exp ///////////////////////////////////////////////////
        if (node->l_child->value->ast_node_value.token->type == TOKEN_OPERATOR_LOGICAL_NOT)
        {
            auto expression = DoExp(node->r_child, true);
            auto preparation_sequence = expression->GetPreparationSequence();

            auto result_variable_name = GetNextVariableName();
            auto true_label = GetNextLabelName();
            auto exit_label = GetNextLabelName();

            preparation_sequence.push_back(instruction_generator_.GenerateIf(
                expression->GetFinalValue(),
                true_label));

            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                result_variable_name,
                instruction_generator_.GenerateImm(1)));

            preparation_sequence.push_back(instruction_generator_.GenerateGoto(exit_label));

            preparation_sequence.push_back(instruction_generator_.GenerateLabel(true_label));

            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                result_variable_name,
                instruction_generator_.GenerateImm(0)));

            preparation_sequence.push_back(instruction_generator_.GenerateLabel(exit_label));

            return std::make_shared<ExpValue>(
                preparation_sequence,
                result_variable_name,
                expression->GetSourceType());
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: L_BRACKET Exp R_BRACKET ///////////////////////////////////
        return DoExp(node->l_child->r_sibling, force_singular);
        ///////////////////////////////////////////////////////////////////
    }
    else
    {
        auto second_child = node->l_child->r_sibling;

        switch (second_child->value->ast_node_value.token->type)
        {
        // Exp : Exp L_SQUARE Exp R_SQUARE
        case TOKEN_DELIMITER_L_SQUARE:
        {
            auto expression = static_cast<ArrayElementExpValue *>(
                DoExp(node->l_child, true).get());

            if (expression == nullptr)
            {
                return nullptr;
            }

            auto index_exp = DoExp(node->l_child->r_sibling->r_sibling, true);

            if (index_exp == nullptr)
            {
                return nullptr;
            }

            // accumulate current offset
            auto preparation_sequence = expression->GetPreparationSequence();
            ConcatenateIrSequence(preparation_sequence, index_exp->GetPreparationSequence());

            size_t offset_size = expression->GetArrayElementSize();
            auto array_dim_sizes = expression->GetArrayDimSizes();
            for (size_t i = expression->GetCurrentDim() + 1;
                 i < array_dim_sizes.size();
                 i++)
            {
                offset_size *= array_dim_sizes[i];
            }

            auto mul_result_name = GetNextVariableName();
            auto next_address_base_name = GetNextVariableName();

            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                mul_result_name,
                instruction_generator_.GenerateBinaryOperation(
                    InstructionGenerator::kBinaryOperatorMul,
                    instruction_generator_.GenerateImm(offset_size),
                    index_exp->GetFinalValue())));

            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                next_address_base_name,
                instruction_generator_.GenerateBinaryOperation(
                    InstructionGenerator::kBinaryOperatorAdd,
                    expression->GetFinalValue(),
                    mul_result_name)));

            // currently at last dim
            if (expression->GetCurrentDim() + 1 == array_dim_sizes.size())
            {
                return std::make_shared<ArrayElementExpValue>(
                    preparation_sequence,
                    instruction_generator_.GenerateDereference(next_address_base_name),
                    expression->GetSourceType(),
                    expression->GetCurrentDim() + 1,
                    expression->GetArrayDimSizes(),
                    expression->GetArrayElementSize());
            }
            // currently not at last dim
            else{
                return std::make_shared<ArrayElementExpValue>(
                    preparation_sequence,
                    next_address_base_name,
                    expression->GetSourceType(),
                    expression->GetCurrentDim() + 1,
                    expression->GetArrayDimSizes(),
                    expression->GetArrayElementSize());
            }
        }

        case TOKEN_OPERATOR_DOT:
        {
            DoExp(node->l_child);

            std::string field_name = node->r_child->value->ast_node_value.token->value;

            return;
        }
        }

        // From now on, the exp can only be a binary operation

        DoExp(node->l_child);

        DoExp(node->r_child);

        switch (second_child->value->ast_node_value.token->type)
        {
        case TOKEN_OPERATOR_ASSIGN:
        {
            return;
        }

        case TOKEN_OPERATOR_LOGICAL_AND:
        case TOKEN_OPERATOR_LOGICAL_OR:
        {
            return;
        }

        case TOKEN_OPERATOR_REL_EQ:
        case TOKEN_OPERATOR_REL_GE:
        case TOKEN_OPERATOR_REL_GT:
        case TOKEN_OPERATOR_REL_LE:
        case TOKEN_OPERATOR_REL_LT:
        case TOKEN_OPERATOR_REL_NE:
        {
            return;
        }

        case TOKEN_OPERATOR_ADD:
        case TOKEN_OPERATOR_SUB:
        case TOKEN_OPERATOR_MUL:
        case TOKEN_OPERATOR_DIV:
        {
            return;
        }

        default:
            return;
        }
    }
}

std::vector<ExpValueSharedPtr> IrGenerator::DoArgs(const KTreeNode *node)
{
    std::vector<ExpValueSharedPtr> args;
    // Args: Exp COMMA Args | Exp
    while (node != NULL)
    {
        auto arg_exp = DoExp(node->l_child, true);

        if (arg_exp == nullptr)
        {
            args.push_back(arg_exp);
            continue;
        }

        switch (arg_exp->GetSourceType()->GetVariableSymbolType())
        {
        case VariableSymbolType::ARRAY:
        {
            if (static_cast<ArraySymbol *>(arg_exp->GetSourceType().get())
                    ->GetElemType()
                    ->GetVariableSymbolType() == VariableSymbolType::ARRAY)
            {
                PrintError("Multi-dim array cannot be passed to a function");
                args.push_back(nullptr);
                continue;
            }
        }
        // 1D array will fall into this block
        case VariableSymbolType::STRUCT:
        {
            args.push_back(std::make_shared<ExpValue>(
                arg_exp->GetPreparationSequence(),
                instruction_generator_.GenerateAddress(arg_exp->GetFinalValue()),
                nullptr));
            break;
        }
        default:
        {
            args.push_back(arg_exp);
            break;
        }
        }

        node = node->r_child;
    }

    return args;
}