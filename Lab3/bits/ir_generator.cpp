#include "ir_generator.h"

void IrGenerator::Generate(const KTreeNode *root)
{
    if (root != NULL &&
        root->l_child != NULL &&
        !root->l_child->value->is_token &&
        root->l_child->value->ast_node_value.variable->type == VARIABLE_EXT_DEF_LIST)
    {
        if (!DoExtDefList(root->l_child))
        {
            return;
        }
    }
    else
    {
        PrintError("Invalid root node");
    }
}

void IrGenerator::PrintKTreeNodeInfo(const KTreeNode *node) const
{
    if (node->value->is_token)
    {
        std::cout << "Token, Type="
                  << (node->value->ast_node_value.token->type)
                  << " Value="
                  << (node->value->ast_node_value.token->value)
                  << " LineNumber="
                  << (node->value->ast_node_value.token->line_start)
                  << " ColumnNumber="
                  << (node->value->ast_node_value.token->column_start)
                  << std::endl;
    }
    else
    {
        std::cout << "Variable, Type="
                  << (node->value->ast_node_value.variable->type)
                  << " LineNumber="
                  << (node->value->ast_node_value.variable->line_start)
                  << " ColumnNumber="
                  << (node->value->ast_node_value.variable->column_start)
                  << std::endl;
    }
}

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

std::string IrGenerator::GetBinaryOperator(const int type) const
{
    switch (type)
    {
    case TOKEN_OPERATOR_REL_EQ:
        return InstructionGenerator::kBinaryOperatorEq;
    case TOKEN_OPERATOR_REL_GE:
        return InstructionGenerator::kBinaryOperatorGe;
    case TOKEN_OPERATOR_REL_GT:
        return InstructionGenerator::kBinaryOperatorGt;
    case TOKEN_OPERATOR_REL_LE:
        return InstructionGenerator::kBinaryOperatorLe;
    case TOKEN_OPERATOR_REL_LT:
        return InstructionGenerator::kBinaryOperatorLt;
    case TOKEN_OPERATOR_REL_NE:
        return InstructionGenerator::kBinaryOperatorNe;
    case TOKEN_OPERATOR_ADD:
        return InstructionGenerator::kBinaryOperatorAdd;
    case TOKEN_OPERATOR_SUB:
        return InstructionGenerator::kBinaryOperatorSub;
    case TOKEN_OPERATOR_MUL:
        return InstructionGenerator::kBinaryOperatorMul;
    case TOKEN_OPERATOR_DIV:
        return InstructionGenerator::kBinaryOperatorDiv;
    default:
        return "";
    }
}

bool IrGenerator::ShouldPassAddress(const VariableSymbol &variable) const
{
    switch (variable.GetVariableSymbolType())
    {
    case VariableSymbolType::ARRAY:
    case VariableSymbolType::STRUCT:
        return true;
    default:
        return false;
    }
}

// Returns <array dim sizes, element type, element size>
auto IrGenerator::GetArrayInfo(const ArraySymbol &array_variable) const
    -> std::tuple<std::vector<size_t>, VariableSymbolSharedPtr, size_t>
{
    auto current_array = &array_variable;

    std::vector<size_t> dim_sizes;

    while (current_array->GetElemType()->GetVariableSymbolType() == VariableSymbolType::ARRAY)
    {
        dim_sizes.push_back(current_array->GetSize());
        current_array = static_cast<const ArraySymbol *>(current_array->GetElemType().get());
    }

    dim_sizes.push_back(current_array->GetSize());

    // Remember that our semantic analyser stores array element type in reverse order.
    std::reverse(dim_sizes.begin(), dim_sizes.end());

    return {dim_sizes,
            current_array->GetElemType(),
            GetVariableSize(*(current_array->GetElemType()))};
}

void IrGenerator::ConcatenateIrSequence(IrSequence &seq1, const IrSequence &seq2) const
{
    seq1.insert(seq1.cend(), seq2.cbegin(), seq2.cend());
}

void IrGenerator::AppendIrSequence(const IrSequence &ir_sequence)
{
    ConcatenateIrSequence(ir_sequence_, ir_sequence);
}

// Returns whether there's no translation error
bool IrGenerator::DoExtDefList(const KTreeNode *node)
{
    // ExtDefList: ExtDef ExtDefList(Nullable) | <NULL>
    while (node != NULL)
    {
        if (!DoExtDef(node->l_child))
        {
            return false;
        }
        node = node->r_child;
    }

    return true;
}

// [INSERTS-IR]
// Returns whether there's no translation error
bool IrGenerator::DoExtDef(const KTreeNode *node)
{
    // ExtDef: Specifier SEMICOLON
    // ignored

    // ExtDef: Specifier ExtDecList SEMICOLON
    if (node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_EXT_DEC_LIST)
    {
        auto ext_decs = DoExtDecList(node->l_child->r_sibling);
        if (!ext_decs.first)
        {
            return false;
        }

        AppendIrSequence(ext_decs.second);
    }
    // ExtDef: Specifier FunDec CompSt
    else if (node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_FUN_DEC)
    {
        auto fun_dec = DoFunDec(node->l_child->r_sibling);
        if (!fun_dec.first)
        {
            return false;
        }

        auto comp_statement = DoCompSt(node->l_child->r_sibling->r_sibling);
        if (!comp_statement.first)
        {
            return false;
        }

        AppendIrSequence(fun_dec.second);
        AppendIrSequence(comp_statement.second);
    }

    return true;
}

// [INSERTS-IR-VARIABLE]
IrSequenceGenerationResult IrGenerator::DoExtDecList(const KTreeNode *node)
{
    IrSequence sequence;
    // ExtDecList: VarDec | VarDec COMMA ExtDecList
    while (node != NULL)
    {
        auto symbol = symbol_table_.at(DoVarDec(node->l_child));
        auto variable_name = GetNextVariableName();
        ir_variable_table_[symbol->GetName()] = variable_name;
        is_address_symbol_[symbol->GetName()] = false;

        switch (symbol->GetVariableSymbolType())
        {
        case VariableSymbolType::ARRAY:
        case VariableSymbolType::STRUCT:
        case VariableSymbolType::ARITHMETIC:
        {
            sequence.push_back(instruction_generator_.GenerateGlobalDec(
                variable_name,
                GetVariableSize(*symbol)));
            break;
        }
        default:
            break;
        }

        node = node->r_child;
    }

    return {true, sequence};
}

IrSequenceGenerationResult IrGenerator::DoDefList(const KTreeNode *node)
{
    IrSequence sequence;
    // DefList: Def DefList(Nullable) | <NULL>
    while (node != NULL)
    {
        auto def = DoDef(node->l_child);
        if (!def.first)
        {
            return kErrorIrSequenceGenerationResult;
        }

        ConcatenateIrSequence(sequence, def.second);

        node = node->r_child;
    }

    return {true, sequence};
}

IrSequenceGenerationResult IrGenerator::DoDef(const KTreeNode *node)
{
    // Def: Specifier DecList SEMICOLON
    return DoDecList(node->l_child->r_sibling);
}

IrSequenceGenerationResult IrGenerator::DoDecList(const KTreeNode *node)
{
    IrSequence sequence;
    // DecList: Dec | Dec COMMA DecList
    while (node != NULL)
    {
        auto dec = DoDec(node->l_child);
        if (!dec.first)
        {
            return kErrorIrSequenceGenerationResult;
        }

        ConcatenateIrSequence(sequence, dec.second);

        node = node->r_child;
    }

    return {true, sequence};
}

// [INSERTS-IR-VARIABLE]
IrSequenceGenerationResult IrGenerator::DoDec(const KTreeNode *node)
{
    // Dec: VarDec | VarDec ASSIGN Exp
    auto var_dec = DoVarDec(node->l_child);

    IrSequence sequence;

    auto symbol = symbol_table_.at(var_dec);
    auto variable_name = GetNextVariableName();
    ir_variable_table_[symbol->GetName()] = variable_name;
    is_address_symbol_[symbol->GetName()] = false;

    switch (symbol->GetVariableSymbolType())
    {
    case VariableSymbolType::ARRAY:
    case VariableSymbolType::STRUCT:
    {
        sequence.push_back(instruction_generator_.GenerateDec(
            variable_name,
            GetVariableSize(*symbol)));
        break;
    }
    default:
        break;
    }

    if (node->l_child->r_sibling == NULL)
    {
        return {true, sequence};
    }

    auto expression = DoExp(node->l_child->r_sibling->r_sibling, false, false);
    if (!expression)
    {
        return {false, IrSequence()};
    }

    ConcatenateIrSequence(sequence, expression->GetPreparationSequence());

    sequence.push_back(instruction_generator_.GenerateAssign(
        variable_name,
        expression->GetFinalValue()));

    return {true, sequence};
}

// Returns the variable symbol name
std::string IrGenerator::DoVarDec(const KTreeNode *node)
{
    // VarDec: ID
    if (node->l_child->value->is_token &&
        node->l_child->value->ast_node_value.token->type == TOKEN_ID)
    {
        return node->l_child->value->ast_node_value.token->value;
    }

    // VarDec: VarDec L_SQUARE LITERAL_INT R_SQUARE
    return DoVarDec(node->l_child);
}

// [INSERTS-IR-VARIABLE] (function parameters)
IrSequenceGenerationResult IrGenerator::DoFunDec(const KTreeNode *node)
{
    IrSequence sequence;
    std::string function_name = node->l_child->value->ast_node_value.token->value;

    sequence.push_back(instruction_generator_.GenerateFunction(function_name));

    // FunDec: ID L_BRACKET R_BRACKET

    // FunDec: ID L_BRACKET VarList R_BRACKET
    if (!node->l_child->r_sibling->r_sibling->value->is_token)
    {
        auto function_symbol = static_cast<FunctionSymbol *>(
            symbol_table_.at(function_name).get());

        auto function_args = function_symbol->GetArgs();
        auto var_list = DoVarList(node->l_child->r_sibling->r_sibling);

        for (int i = 0; i < function_args.size(); i++)
        {
            auto param_variable_name = GetNextVariableName();
            ir_variable_table_[function_args[i]->GetName()] = param_variable_name;
            is_address_symbol_[function_args[i]->GetName()] =
                ShouldPassAddress(*function_args[i]);
            sequence.push_back(instruction_generator_.GenerateParam(
                param_variable_name));
        }
    }

    return {true, sequence};
}

std::vector<std::string> IrGenerator::DoVarList(const KTreeNode *node)
{
    std::vector<std::string> vars;
    // VarList: ParamDec COMMA VarList | ParamDec
    while (node != NULL)
    {
        vars.push_back(DoParamDec(node->l_child));
        node = node->r_child;
    }

    return vars;
}

std::string IrGenerator::DoParamDec(const KTreeNode *node)
{
    // ParamDec: Specifier VarDec
    return DoVarDec(node->r_child);
}

IrSequenceGenerationResult IrGenerator::DoCompSt(const KTreeNode *node)
{
    // CompSt: L_BRACE DefList(Nullable) StmtList(Nullable) R_BRACE

    IrSequence sequence;

    // At least one of DefList and StmtList is not NULL
    if (!node->l_child->r_sibling->value->is_token)
    {
        // DefList is not NULL
        if (node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_DEF_LIST)
        {
            auto def_list = DoDefList(node->l_child->r_sibling);
            if (!def_list.first)
            {
                return kErrorIrSequenceGenerationResult;
            }

            ConcatenateIrSequence(sequence, def_list.second);

            // StmtList is not NULL either
            if (!node->l_child->r_sibling->r_sibling->value->is_token)
            {
                auto statement_list = DoStmtList(node->l_child->r_sibling->r_sibling);
                if (!statement_list.first)
                {
                    return kErrorIrSequenceGenerationResult;
                }

                ConcatenateIrSequence(sequence, statement_list.second);
            }
            // StmtList is NULL
        }
        // DefList is NULL and StmtList is not NULL
        else
        {
            auto statement_list = DoStmtList(node->l_child->r_sibling);
            if (!statement_list.first)
            {
                return kErrorIrSequenceGenerationResult;
            }

            ConcatenateIrSequence(sequence, statement_list.second);
        }
    }

    // Both DefList and StmtList are NULL

    // Common return
    return {true, sequence};
}

IrSequenceGenerationResult IrGenerator::DoStmtList(const KTreeNode *node)
{
    IrSequence sequence;
    // StmtList: Stmt StmtList(Nullable) | <NULL>
    while (node != NULL)
    {
        auto statement = DoStmt(node->l_child);
        if (!statement.first)
        {
            return {
                false,
                IrSequence()};
        }

        ConcatenateIrSequence(sequence, statement.second);
        node = node->r_child;
    }

    return {true, sequence};
}

IrSequenceGenerationResult IrGenerator::DoStmt(const KTreeNode *node)
{
    if (!node->l_child->value->is_token)
    {
        // Stmt: Exp SEMICOLON
        if (node->l_child->value->ast_node_value.variable->type == VARIABLE_EXP)
        {
            auto expression = DoExp(node->l_child, false, false);
            if (!expression)
            {
                return kErrorIrSequenceGenerationResult;
            }

            // We still need the final value in case of Exp is a CALL
            auto sequence = expression->GetPreparationSequence();
            // Very bad patch
            if (expression->GetFinalValue().find("CALL ") == 0)
            {
                sequence.push_back(expression->GetFinalValue());
            }

            return {
                true,
                sequence};
        }

        // Stmt: CompSt
        return DoCompSt(node->l_child);
    }

    switch (node->l_child->value->ast_node_value.token->type)
    {
    // Stmt: RETURN Exp SEMICOLON
    case TOKEN_KEYWORD_RETURN:
    {
        auto expression = DoExp(node->l_child->r_sibling, true, false);
        if (!expression)
        {
            return kErrorIrSequenceGenerationResult;
        }

        auto sequence = expression->GetPreparationSequence();
        sequence.push_back(instruction_generator_.GenerateReturn(
            expression->GetFinalValue()));

        return {true, sequence};
    }
    case TOKEN_KEYWORD_IF:
    {
        auto condition_exp_node = node->l_child->r_sibling->r_sibling;
        auto if_stmt_node = condition_exp_node->r_sibling->r_sibling;

        auto condition = DoExp(condition_exp_node, true, false);
        if (!condition)
        {
            return kErrorIrSequenceGenerationResult;
        }

        // Stmt: IF L_BRACKET Exp R_BRACKET Stmt
        auto statement = DoStmt(if_stmt_node);
        if (!statement.first)
        {
            return kErrorIrSequenceGenerationResult;
        }

        IrSequence sequence_single_branch = condition->GetPreparationSequence();
        IrSequence sequence_dual_branch = condition->GetPreparationSequence();

        auto true_label = GetNextLabelName();
        auto exit_label = GetNextLabelName();

        sequence_single_branch.push_back(instruction_generator_.GenerateIf(
            instruction_generator_.GenerateBinaryOperation(
                InstructionGenerator::kBinaryOperatorNe,
                condition->GetFinalValue(),
                instruction_generator_.GenerateImm(0)),
            true_label));

        sequence_dual_branch.push_back(instruction_generator_.GenerateIf(
            instruction_generator_.GenerateBinaryOperation(
                InstructionGenerator::kBinaryOperatorNe,
                condition->GetFinalValue(),
                instruction_generator_.GenerateImm(0)),
            true_label));

        sequence_single_branch.push_back(instruction_generator_.GenerateGoto(exit_label));

        sequence_single_branch.push_back(instruction_generator_.GenerateLabel(true_label));

        ConcatenateIrSequence(sequence_single_branch, statement.second);

        sequence_single_branch.push_back(instruction_generator_.GenerateLabel(exit_label));

        if (if_stmt_node->r_sibling == NULL)
        {
            return {true, sequence_single_branch};
        }

        // Stmt: IF L_BRACKET Exp R_BRACKET Stmt ELSE Stmt
        auto else_statemtnt = DoStmt(if_stmt_node->r_sibling->r_sibling);
        if (!else_statemtnt.first)
        {
            return kErrorIrSequenceGenerationResult;
        }

        ConcatenateIrSequence(sequence_dual_branch, else_statemtnt.second);

        sequence_dual_branch.push_back(instruction_generator_.GenerateGoto(exit_label));

        sequence_dual_branch.push_back(instruction_generator_.GenerateLabel(true_label));

        ConcatenateIrSequence(sequence_dual_branch, statement.second);

        sequence_dual_branch.push_back(instruction_generator_.GenerateLabel(exit_label));

        return {true, sequence_dual_branch};
    }
    // Stmt: WHILE L_BRACKET Exp R_BRACKET Stmt
    case TOKEN_KEYWORD_WHILE:
    {
        auto condition = DoExp(node->l_child->r_sibling->r_sibling, true, false);
        if (!condition)
        {
            return kErrorIrSequenceGenerationResult;
        }

        auto statement = DoStmt(node->r_child);
        if (!statement.first)
        {
            return kErrorIrSequenceGenerationResult;
        }

        auto test_label = GetNextLabelName();
        auto body_label = GetNextLabelName();
        auto exit_label = GetNextLabelName();

        IrSequence sequence = {instruction_generator_.GenerateLabel(test_label)};
        ConcatenateIrSequence(sequence, condition->GetPreparationSequence());
        sequence.push_back(instruction_generator_.GenerateIf(
            instruction_generator_.GenerateBinaryOperation(
                InstructionGenerator::kBinaryOperatorNe,
                condition->GetFinalValue(),
                instruction_generator_.GenerateImm(0)),
            body_label));
        sequence.push_back(instruction_generator_.GenerateGoto(exit_label));
        sequence.push_back(instruction_generator_.GenerateLabel(body_label));

        ConcatenateIrSequence(sequence, statement.second);

        sequence.push_back(instruction_generator_.GenerateGoto(test_label));
        sequence.push_back(instruction_generator_.GenerateLabel(exit_label));

        return {true, sequence};
    }
    default:
        return kErrorIrSequenceGenerationResult;
    }
}

// Returns the expression's final value and the leading preparation IR sequence.
// If force_singular is true, then GetFinalValue() of returned ExpValue
// is guarenteed to be either a variable or an imm. Further, when singular_no_prefix is true,
// then it's guaranteed that the singular is not prefixed with & or *. Note that
// a call for a l-value expression must not have singular_no_prefix=true.
// Otherwise, GetFinalValue() may contain a binary operation in the form of x op y
// or a function call.
// When node points to an array indexing which is not at the final dim(i.e., a or a[1] for
// array a[5][5]), GetFinalValue() in return value will be the base address of next dim
// (i.e., a or a+1*5*4). This is used for translating array element and array argument.
// When node points to a struct object, GetFinalValue() in return value will be its address.
// If translation error happens, nullptr is returned.
ExpValueSharedPtr IrGenerator::DoExp(const KTreeNode *node,
                                     const bool force_singular,
                                     const bool singular_no_prefix)
{
    const auto kLogicOperationResultType = std::make_shared<ArithmeticSymbol>(
        -1,
        "",
        ArithmeticSymbolType::INT);

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
                auto ir_variable_name = ir_variable_table_.at(token_value);
                auto is_address = is_address_symbol_.at(token_value);

                auto address_final_value =
                    is_address
                        ? ir_variable_name
                        : instruction_generator_.GenerateAddress(ir_variable_name);

                switch (symbol->GetVariableSymbolType())
                {
                // 1D array can be arg, after indexing can be lval/rval.
                // MD array after indexing can be lval/rval.
                // In all cases we start with its address.
                case VariableSymbolType::ARRAY:
                {
                    auto array_info = GetArrayInfo(*static_cast<ArraySymbol *>(symbol.get()));

                    if (force_singular && singular_no_prefix)
                    {
                        auto address_name = GetNextVariableName();
                        return std::make_shared<ArrayElementExpValue>(
                            IrSequence({instruction_generator_.GenerateAssign(
                                address_name, address_final_value)}),
                            address_name,
                            symbol,
                            0,
                            std::get<0>(array_info),
                            std::get<1>(array_info),
                            std::get<2>(array_info));
                    }
                    else
                    {
                        return std::make_shared<ArrayElementExpValue>(
                            IrSequence(),
                            address_final_value,
                            symbol,
                            0,
                            std::get<0>(array_info),
                            std::get<1>(array_info),
                            std::get<2>(array_info));
                    }
                }
                // Struct can be arg, after selecting field can be lval/rval.
                // In all cases we start with its address.
                case VariableSymbolType::STRUCT:
                {
                    if (force_singular && singular_no_prefix)
                    {
                        auto address_name = GetNextVariableName();
                        return std::make_shared<ExpValue>(
                            IrSequence({instruction_generator_.GenerateAssign(
                                address_name, address_final_value)}),
                            address_name,
                            symbol);
                    }
                    else
                    {
                        return std::make_shared<ExpValue>(
                            IrSequence(),
                            address_final_value,
                            symbol);
                    }
                }
                default:
                {
                    return std::make_shared<ExpValue>(
                        IrSequence(),
                        ir_variable_name,
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
                    if (!arg)
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
            auto expression = DoExp(node->r_child, true, false);
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
            auto expression = DoExp(node->r_child, true, true);
            auto preparation_sequence = expression->GetPreparationSequence();

            auto result_variable_name = GetNextVariableName();
            auto true_label = GetNextLabelName();
            auto exit_label = GetNextLabelName();

            preparation_sequence.push_back(instruction_generator_.GenerateIf(
                instruction_generator_.GenerateBinaryOperation(
                    InstructionGenerator::kBinaryOperatorNe,
                    expression->GetFinalValue(),
                    instruction_generator_.GenerateImm(0)),
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
                kLogicOperationResultType);
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: L_BRACKET Exp R_BRACKET ///////////////////////////////////
        return DoExp(node->l_child->r_sibling, force_singular, singular_no_prefix);
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
            // We must not acquire array_expression by static_cast<>(DoExp().get())
            // in which case the shared_ptr will be destructed immediately and
            // the pointer object is also destructed.
            // We must keep a reference to the original shared_ptr.
            auto expression = DoExp(node->l_child, true, false);

            if (!expression)
            {
                return nullptr;
            }

            auto array_expression = static_cast<ArrayElementExpValue *>(expression.get());

            auto index_exp = DoExp(second_child->r_sibling, true, false);

            if (!index_exp)
            {
                return nullptr;
            }

            // accumulate current offset
            auto preparation_sequence = array_expression->GetPreparationSequence();
            ConcatenateIrSequence(preparation_sequence, index_exp->GetPreparationSequence());

            auto offset_size = array_expression->GetArrayElementSize();
            auto array_dim_sizes = array_expression->GetArrayDimSizes();
            for (size_t i = array_expression->GetCurrentDim() + 1;
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
                    array_expression->GetFinalValue(),
                    mul_result_name)));

            // currently at last dim
            if (array_expression->GetCurrentDim() + 1 == array_dim_sizes.size())
            {
                // if element is struct, still generate an address
                if (array_expression->GetArrayElementType()->GetVariableSymbolType() ==
                    VariableSymbolType::STRUCT)
                {
                    return std::make_shared<ExpValue>(
                        preparation_sequence,
                        next_address_base_name,
                        array_expression->GetArrayElementType());
                }
                else
                {
                    if (force_singular && singular_no_prefix)
                    {
                        auto variable_name = GetNextVariableName();
                        preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                            variable_name,
                            instruction_generator_.GenerateDereference(next_address_base_name)));

                        return std::make_shared<ExpValue>(
                            preparation_sequence,
                            variable_name,
                            array_expression->GetArrayElementType());
                    }
                    else
                    {
                        return std::make_shared<ExpValue>(
                            preparation_sequence,
                            instruction_generator_.GenerateDereference(next_address_base_name),
                            array_expression->GetArrayElementType());
                    }
                }
            }
            // currently not at last dim
            else
            {
                return std::make_shared<ArrayElementExpValue>(
                    preparation_sequence,
                    next_address_base_name,
                    array_expression->GetSourceType(),
                    array_expression->GetCurrentDim() + 1,
                    array_expression->GetArrayDimSizes(),
                    array_expression->GetArrayElementType(),
                    array_expression->GetArrayElementSize());
            }
        }

        case TOKEN_OPERATOR_DOT:
        {
            auto expression = DoExp(node->l_child, true, false);

            if (!expression)
            {
                return nullptr;
            }

            std::string field_name = node->r_child->value->ast_node_value.token->value;

            auto struct_def = struct_def_symbol_table_.at(
                static_cast<StructSymbol *>(expression->GetSourceType().get())->GetStructName());

            auto fields = struct_def->GetFields();

            size_t field_offset = 0;

            for (int i = 0; i < fields.size(); i++)
            {
                if (fields[i]->GetName() == field_name)
                {
                    auto preparation_sequence = expression->GetPreparationSequence();

                    // A previously-used strategy is to save an addition when field_offset=0.
                    // However, this requires calling DoExp() with singular_no_prefix=true,
                    // which will cause additional aliasing to the struct address for every
                    // field not at offset 0. This overweights the saved addition which
                    // applied to first field only.
                    // The same thing is for array.
                    auto address_name = GetNextVariableName();
                    preparation_sequence.push_back(
                        instruction_generator_.GenerateAssign(
                            address_name,
                            instruction_generator_.GenerateBinaryOperation(
                                InstructionGenerator::kBinaryOperatorAdd,
                                expression->GetFinalValue(),
                                instruction_generator_.GenerateImm(field_offset))));

                    switch (fields[i]->GetVariableSymbolType())
                    {
                    case VariableSymbolType::ARRAY:
                    {
                        auto array_info = GetArrayInfo(
                            *static_cast<ArraySymbol *>(fields[i].get()));

                        return std::make_shared<ArrayElementExpValue>(
                            preparation_sequence,
                            address_name,
                            fields[i],
                            0,
                            std::get<0>(array_info),
                            std::get<1>(array_info),
                            std::get<2>(array_info));
                    }
                    case VariableSymbolType::STRUCT:
                    {
                        return std::make_shared<ExpValue>(
                            preparation_sequence,
                            address_name,
                            fields[i]);
                    }
                    default:
                    {
                        if (force_singular && singular_no_prefix)
                        {
                            auto variable_name = GetNextVariableName();
                            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                                variable_name,
                                instruction_generator_.GenerateDereference(address_name)));

                            return std::make_shared<ExpValue>(
                                preparation_sequence,
                                variable_name,
                                fields[i]);
                        }
                        else
                        {
                            return std::make_shared<ExpValue>(
                                preparation_sequence,
                                instruction_generator_.GenerateDereference(address_name),
                                fields[i]);
                        }
                    }
                    }
                }
                else
                {
                    field_offset += GetVariableSize(*fields[i]);
                }
            }

            return nullptr;
        }
        }

        // From now on, the exp can only be a binary operation

        auto l_exp = DoExp(node->l_child, true, false);

        if (!l_exp)
        {
            return nullptr;
        }

        if (l_exp->GetSourceType()->GetVariableSymbolType() != VariableSymbolType::ARITHMETIC)
        {
            PrintError("Assignment can only be performed between arithmetic variables");
            return nullptr;
        }

        auto r_exp = DoExp(node->r_child, true, false);
        if (!r_exp)
        {
            return nullptr;
        }

        IrSequence preparation_sequence;
        ConcatenateIrSequence(preparation_sequence, l_exp->GetPreparationSequence());
        ConcatenateIrSequence(preparation_sequence, r_exp->GetPreparationSequence());

        auto operator_type = second_child->value->ast_node_value.token->type;
        bool is_relop = false;

        switch (operator_type)
        {
        case TOKEN_OPERATOR_ASSIGN:
        {
            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                l_exp->GetFinalValue(),
                r_exp->GetFinalValue()));

            // Note that r_exp.FinalValue() may be prefixed
            if (force_singular && singular_no_prefix)
            {
                auto variable_name = GetNextVariableName();

                preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                    variable_name,
                    r_exp->GetFinalValue()));

                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    variable_name,
                    r_exp->GetSourceType());
            }
            else
            {
                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    r_exp->GetFinalValue(),
                    r_exp->GetSourceType());
            }
        }

        case TOKEN_OPERATOR_LOGICAL_AND:
        {
            auto result_variable_name = GetNextVariableName();
            auto true_label = GetNextLabelName();
            auto next_test_label = GetNextLabelName();
            auto false_label = GetNextLabelName();
            auto exit_label = GetNextLabelName();

            preparation_sequence.push_back(instruction_generator_.GenerateIf(
                instruction_generator_.GenerateBinaryOperation(
                    InstructionGenerator::kBinaryOperatorNe,
                    l_exp->GetFinalValue(),
                    instruction_generator_.GenerateImm(0)),
                next_test_label));

            preparation_sequence.push_back(instruction_generator_.GenerateLabel(false_label));

            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                result_variable_name,
                instruction_generator_.GenerateImm(0)));

            preparation_sequence.push_back(instruction_generator_.GenerateGoto(exit_label));

            preparation_sequence.push_back(instruction_generator_.GenerateLabel(next_test_label));

            preparation_sequence.push_back(instruction_generator_.GenerateIf(
                instruction_generator_.GenerateBinaryOperation(
                    InstructionGenerator::kBinaryOperatorNe,
                    r_exp->GetFinalValue(),
                    instruction_generator_.GenerateImm(0)),
                true_label));

            preparation_sequence.push_back(instruction_generator_.GenerateGoto(false_label));

            preparation_sequence.push_back(instruction_generator_.GenerateLabel(true_label));

            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                result_variable_name,
                instruction_generator_.GenerateImm(1)));

            preparation_sequence.push_back(instruction_generator_.GenerateLabel(exit_label));

            return std::make_shared<ExpValue>(
                preparation_sequence,
                result_variable_name,
                kLogicOperationResultType);
        }
        case TOKEN_OPERATOR_LOGICAL_OR:
        {
            auto result_variable_name = GetNextVariableName();
            auto true_label = GetNextLabelName();
            auto exit_label = GetNextLabelName();

            preparation_sequence.push_back(instruction_generator_.GenerateIf(
                instruction_generator_.GenerateBinaryOperation(
                    InstructionGenerator::kBinaryOperatorNe,
                    l_exp->GetFinalValue(),
                    instruction_generator_.GenerateImm(0)),
                true_label));

            preparation_sequence.push_back(instruction_generator_.GenerateIf(
                instruction_generator_.GenerateBinaryOperation(
                    InstructionGenerator::kBinaryOperatorNe,
                    r_exp->GetFinalValue(),
                    instruction_generator_.GenerateImm(0)),
                true_label));

            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                result_variable_name,
                instruction_generator_.GenerateImm(0)));

            preparation_sequence.push_back(instruction_generator_.GenerateGoto(exit_label));

            preparation_sequence.push_back(instruction_generator_.GenerateLabel(true_label));

            preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                result_variable_name,
                instruction_generator_.GenerateImm(1)));

            preparation_sequence.push_back(instruction_generator_.GenerateLabel(exit_label));

            return std::make_shared<ExpValue>(
                preparation_sequence,
                result_variable_name,
                kLogicOperationResultType);
        }
        case TOKEN_OPERATOR_REL_EQ:
        case TOKEN_OPERATOR_REL_GE:
        case TOKEN_OPERATOR_REL_GT:
        case TOKEN_OPERATOR_REL_LE:
        case TOKEN_OPERATOR_REL_LT:
        case TOKEN_OPERATOR_REL_NE:
        {
            // For relop, when singular is not forced, we can also generate a final value
            // in the form of binary operation
            if (force_singular)
            {
                auto binary_operator = GetBinaryOperator(operator_type);

                auto result_variable_name = GetNextVariableName();
                auto true_label = GetNextLabelName();
                auto exit_label = GetNextLabelName();

                preparation_sequence.push_back(instruction_generator_.GenerateIf(
                    instruction_generator_.GenerateBinaryOperation(
                        binary_operator,
                        l_exp->GetFinalValue(),
                        r_exp->GetFinalValue()),
                    true_label));

                preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                    result_variable_name,
                    instruction_generator_.GenerateImm(0)));

                preparation_sequence.push_back(instruction_generator_.GenerateGoto(exit_label));

                preparation_sequence.push_back(instruction_generator_.GenerateLabel(true_label));

                preparation_sequence.push_back(instruction_generator_.GenerateAssign(
                    result_variable_name,
                    instruction_generator_.GenerateImm(1)));

                preparation_sequence.push_back(instruction_generator_.GenerateLabel(exit_label));

                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    result_variable_name,
                    kLogicOperationResultType);
            }
            else
            {
                is_relop = true;
            }
        }
        case TOKEN_OPERATOR_ADD:
        case TOKEN_OPERATOR_SUB:
        case TOKEN_OPERATOR_MUL:
        case TOKEN_OPERATOR_DIV:
        {
            auto binary_operator = GetBinaryOperator(operator_type);
            if (force_singular)
            {
                auto variable_name = GetNextVariableName();
                preparation_sequence.push_back(
                    instruction_generator_.GenerateAssign(
                        variable_name,
                        instruction_generator_.GenerateBinaryOperation(
                            binary_operator,
                            l_exp->GetFinalValue(),
                            r_exp->GetFinalValue())));

                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    variable_name,
                    l_exp->GetSourceType());
            }
            else
            {
                return std::make_shared<ExpValue>(
                    preparation_sequence,
                    instruction_generator_.GenerateBinaryOperation(
                        binary_operator,
                        l_exp->GetFinalValue(),
                        r_exp->GetFinalValue()),
                    is_relop ? kLogicOperationResultType : l_exp->GetSourceType());
            }
        }

        default:
            return nullptr;
        }
    }
}

std::vector<ExpValueSharedPtr> IrGenerator::DoArgs(const KTreeNode *node)
{
    std::vector<ExpValueSharedPtr> args;
    // Args: Exp COMMA Args | Exp
    while (node != NULL)
    {
        auto arg_exp = DoExp(node->l_child, true, false);

        if (!arg_exp)
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
                arg_exp->GetFinalValue(),
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