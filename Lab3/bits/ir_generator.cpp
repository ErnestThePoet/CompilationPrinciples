#include "ir_generator.h"

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
    auto specifier = DoSpecifier(node->l_child);

    // ExtDef: Specifier SEMICOLON
    if (node->l_child->r_sibling->value->is_token)
    {
        return;
    }

    // ExtDef: Specifier ExtDecList SEMICOLON
    if (node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_EXT_DEC_LIST)
    {
        if (specifier == nullptr)
        {
            return;
        }

        auto dec_list = DoExtDecList(node->l_child->r_sibling);

        auto defs = DoDecListDefCommon(specifier, dec_list);

        for (auto &def : defs)
        {
            InsertVariableSymbol(def);
        }
    }
    // ExtDef: Specifier FunDec CompSt
    else
    {
        // we do not check specifier!=nullptr or we will be unable to check the whole function body
        auto fun_dec = DoFunDec(node->l_child->r_sibling);
        if (fun_dec == nullptr)
        {
            return;
        }

        InsertVariableSymbol(std::make_shared<FunctionSymbol>(
            fun_dec->GetLineNumber(),
            fun_dec->GetName(),
            fun_dec->GetArgs(),
            specifier // may be nullptr
            ));

        for (auto &arg : fun_dec->GetArgs())
        {
            InsertVariableSymbol(arg);
        }

        auto return_types = DoCompSt(node->l_child->r_sibling->r_sibling);

        // all parallel return values must match the declared return type
        if (specifier != nullptr)
        {
            for (auto &return_type : return_types)
            {
                if (return_type == nullptr)
                {
                    continue;
                }

                if (!IsAssignmentValid(*specifier, *return_type))
                {
                    PrintError(kErrorReturnTypeMismatch,
                               return_type->GetLineNumber(),
                               "Should return '" + GetVariableSymbolTypeName(specifier) + '\'');

                    // do not break
                }
            }
        }
    }
}

void IrGenerator::DoDecListDefCommon(
    const VariableSymbolSharedPtr &specifier,
    const std::vector<VariableSymbolSharedPtr> &dec_list)
{
    std::vector<VariableSymbolSharedPtr> defs;

    if (specifier == nullptr)
    {
        return defs;
    }

    for (auto &dec : dec_list)
    {
        if (dec == nullptr)
        {
            defs.push_back(nullptr);
        }
        else if (dec->GetVariableSymbolType() == VariableSymbolType::UNKNOWN)
        {
            // Arithmetic
            if (specifier->GetVariableSymbolType() == VariableSymbolType::ARITHMETIC)
            {
                if (dec->GetIsInitialized() &&
                    !IsAssignmentValid(*specifier, *(dec->GetInitialValue())))
                {
                    PrintError(
                        kErrorAssignTypeMismatch,
                        dec->GetLineNumber(),
                        "Cannot assign '" +
                            GetVariableSymbolTypeName(dec) +
                            "' to a variable of type '" +
                            GetVariableSymbolTypeName(specifier));

                    continue;
                }

                defs.push_back(std::make_shared<ArithmeticSymbol>(
                    dec->GetLineNumber(),
                    dec->GetName(),
                    static_cast<ArithmeticSymbol *>(specifier.get())->GetArithmeticSymbolType(),
                    dec->GetIsInitialized(),
                    dec->GetInitialValue()));
            }
            // Struct
            else
            {
                std::string struct_name = static_cast<StructSymbol *>(specifier.get())->GetStructName();
                if (struct_def_symbol_table_.find(struct_name) == struct_def_symbol_table_.end())
                {
                    PrintError(kErrorUndefinedStruct, dec->GetLineNumber(),
                               "Undefined struct type: " + GetVariableSymbolTypeName(specifier));
                    continue;
                }

                // Check assignment compatibility
                if (dec->GetIsInitialized() &&
                    !IsAssignmentValid(*specifier, *(dec->GetInitialValue())))
                {
                    PrintError(
                        kErrorAssignTypeMismatch,
                        dec->GetLineNumber(),
                        "Cannot assign '" +
                            GetVariableSymbolTypeName(dec) +
                            "' to a variable of type '" +
                            GetVariableSymbolTypeName(specifier));

                    continue;
                }

                defs.push_back(std::make_shared<StructSymbol>(
                    dec->GetLineNumber(),
                    dec->GetName(),
                    struct_name,
                    dec->GetIsInitialized(),
                    dec->GetInitialValue()));
            }
        }
        // array
        else
        {
            ArraySymbol *array_dec = static_cast<ArraySymbol *>(dec.get());
            while (array_dec->GetElemType()->GetVariableSymbolType() ==
                   VariableSymbolType::ARRAY)
            {
                array_dec = static_cast<ArraySymbol *>(array_dec->GetElemType().get());
            }

            if (specifier->GetVariableSymbolType() == VariableSymbolType::ARITHMETIC)
            {
                array_dec->SetElemType(std::make_shared<ArithmeticSymbol>(
                    array_dec->GetElemType()->GetLineNumber(),
                    array_dec->GetElemType()->GetName(),
                    static_cast<ArithmeticSymbol *>(specifier.get())->GetArithmeticSymbolType(),
                    array_dec->GetElemType()->GetIsInitialized(),
                    array_dec->GetElemType()->GetInitialValue()));
            }
            else
            {
                array_dec->SetElemType(std::make_shared<StructSymbol>(
                    array_dec->GetElemType()->GetLineNumber(),
                    array_dec->GetElemType()->GetName(),
                    static_cast<StructSymbol *>(specifier.get())->GetStructName(),
                    array_dec->GetElemType()->GetIsInitialized(),
                    array_dec->GetElemType()->GetInitialValue()));
            }

            // Check assignment compatibility
            if (dec->GetIsInitialized() &&
                !IsAssignmentValid(*specifier, *(dec->GetInitialValue())))
            {
                PrintError(
                    kErrorAssignTypeMismatch,
                    dec->GetLineNumber(),
                    "Cannot assign '" +
                        GetVariableSymbolTypeName(dec) +
                        "' to a variable of type '" +
                        GetVariableSymbolTypeName(specifier));

                continue;
            }

            defs.push_back(dec);
        }
    }

    return defs;
}

void IrGenerator::DoExtDecList(const KTreeNode *node)
{
    // ExtDecList: VarDec | VarDec COMMA ExtDecList
    std::vector<VariableSymbolSharedPtr> decs;

    while (node != NULL)
    {
        decs.push_back(DoVarDec(node->l_child));
        node = node->r_child;
    }

    return decs;
}

void IrGenerator::DoSpecifier(const KTreeNode *node)
{
    // Specifier: TYPE_INT | TYPE_FLOAT
    if (node->l_child->value->is_token)
    {
        return std::make_shared<ArithmeticSymbol>(
            GetKTreeNodeLineNumber(node->l_child),
            "",
            node->l_child->value->ast_node_value.token->type == TOKEN_KEYWORD_TYPE_INT
                ? ArithmeticSymbolType::INT
                : ArithmeticSymbolType::FLOAT);
    }
    // Specifier: StructSpecifier
    else
    {
        return DoStructSpecifier(node->l_child);
    }
}

void IrGenerator::DoStructSpecifier(const KTreeNode *node)
{
    std::string struct_name;

    // StructSpecifier: STRUCT Tag
    if (!node->l_child->r_sibling->value->is_token &&
        node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_TAG)
    {
        KTreeNode *struct_id_node = node->l_child->r_sibling->l_child;
        struct_name =
            struct_id_node->value->ast_node_value.token->value;
        if (struct_def_symbol_table_.find(struct_name) != struct_def_symbol_table_.end())
        {
            return std::make_shared<StructSymbol>(
                GetKTreeNodeLineNumber(struct_id_node), "", struct_name);
        }

        PrintError(kErrorUndefinedStruct,
                   GetKTreeNodeLineNumber(struct_id_node),
                   "Struct '" + struct_name + "' is not defined");
        return nullptr;
    }
    // StructSpecifier: STRUCT OptTag L_BRACE DefList(Nullable) R_BRACE
    else
    {
        KTreeNode *def_list_node = nullptr;
        // Unnamed struct def
        // StructSpecifier: STRUCT L_BRACE DefList(Nullable) R_BRACE
        if (node->l_child->r_sibling->value->is_token)
        {
            struct_name = GetNewAnnoyStructName();
            def_list_node = node->l_child->r_sibling->r_sibling;
        }
        // Named struct def
        // StructSpecifier: STRUCT OptTag L_BRACE DefList(Nullable) R_BRACE
        else
        {
            struct_name = node->l_child->r_sibling->l_child->value->ast_node_value.token->value;
            def_list_node = node->l_child->r_sibling->r_sibling->r_sibling;

            if (struct_def_symbol_table_.find(struct_name) != struct_def_symbol_table_.end())
            {
                PrintError(kErrorDuplicateStructName,
                           GetKTreeNodeLineNumber(node->l_child->r_sibling->l_child),
                           "Duplicate struct name '" + struct_name + '\'');
                return nullptr;
            }
        }

        std::vector<VariableSymbolSharedPtr> fields;

        if (!def_list_node->value->is_token)
        {
            fields = DoDefList(def_list_node, false);
        }

        // Checks kErrorDuplicateStructFieldName
        std::unordered_set<std::string> field_names;
        for (auto &field : fields)
        {
            if (field == nullptr)
            {
                continue;
            }

            if (field_names.find(field->GetName()) != field_names.end())
            {
                PrintError(kErrorDuplicateStructFieldName,
                           field->GetLineNumber(),
                           "Duplicate field '" + field->GetName() + "'");
                return nullptr;
            }
            else
            {
                field_names.insert(field->GetName());
            }
        }

        // Checks kErrorStructFieldInitialized
        for (auto &field : fields)
        {
            if (field == nullptr)
            {
                continue;
            }

            if (field->GetIsInitialized())
            {
                PrintError(kErrorStructFieldInitialized,
                           field->GetLineNumber(),
                           "Struct field '" + field->GetName() + "' is initialized");

                return nullptr;
            }
        }

        struct_def_symbol_table_[struct_name] = std::make_shared<StructDefSymbol>(
            GetKTreeNodeLineNumber(node->l_child), struct_name, fields);

        return std::make_shared<StructSymbol>(GetKTreeNodeLineNumber(node->l_child), "", struct_name);
    }
}

void IrGenerator::DoDefList(
    const KTreeNode *node, const bool should_insert)
{
    // DefList: Def DefList(Nullable) | <NULL>
    std::vector<VariableSymbolSharedPtr> defs;
    while (node != NULL)
    {
        auto current_defs = DoDef(node->l_child, should_insert);
        defs.insert(defs.cend(), current_defs.begin(), current_defs.end());
        node = node->r_child;
    }

    return defs;
}

void IrGenerator::DoDef(
    const KTreeNode *node, const bool should_insert)
{
    // Def: Specifier DecList SEMICOLON
    auto specifier = DoSpecifier(node->l_child);
    auto dec_list = DoDecList(node->l_child->r_sibling);

    auto defs = DoDecListDefCommon(specifier, dec_list);

    if (should_insert)
    {
        for (auto &def : defs)
        {
            InsertVariableSymbol(def);
        }
    }

    return defs;
}

void IrGenerator::DoDecList(const KTreeNode *node)
{
    // DecList: Dec | Dec COMMA DecList
    std::vector<VariableSymbolSharedPtr> decs;

    while (node != NULL)
    {
        decs.push_back(DoDec(node->l_child));
        node = node->r_child;
    }

    return decs;
}

void IrGenerator::DoDec(const KTreeNode *node)
{
    // Dec: VarDec | VarDec ASSIGN Exp
    auto var_dec = DoVarDec(node->l_child);
    if (var_dec == nullptr)
    {
        return nullptr;
    }

    if (node->l_child->r_sibling == NULL)
    {
        return var_dec;
    }

    auto initial_value = DoExp(node->l_child->r_sibling->r_sibling);

    // When initial value is erroneous, just report the variable as uninitialized
    return std::make_shared<VariableSymbol>(
        var_dec->GetLineNumber(),
        var_dec->GetName(),
        var_dec->GetVariableSymbolType(),
        initial_value.first != nullptr,
        initial_value.first);
}

void IrGenerator::DoVarDec(const KTreeNode *node)
{
    // VarDec: ID
    if (node->l_child->value->is_token &&
        node->l_child->value->ast_node_value.token->type == TOKEN_ID)
    {
        return std::make_shared<VariableSymbol>(
            GetKTreeNodeLineNumber(node->l_child),
            node->l_child->value->ast_node_value.token->value,
            VariableSymbolType::UNKNOWN);
    }

    // VarDec: VarDec L_SQUARE LITERAL_INT R_SQUARE

    // Note that int a[2][3] should be interpreted as array<array<int,3>,2>,
    // But ArraySymbol views it as array<array<int,2>,3>
    auto array_element = DoVarDec(node->l_child);
    if (array_element == nullptr)
    {
        return nullptr;
    }

    return std::make_shared<ArraySymbol>(
        array_element->GetLineNumber(),
        array_element->GetName(),
        array_element,
        std::stoull(node->l_child->r_sibling->r_sibling->value->ast_node_value.token->value));
}

void IrGenerator::DoFunDec(const KTreeNode *node)
{
    std::string function_name = node->l_child->value->ast_node_value.token->value;

    // FunDec: ID L_BRACKET R_BRACKET
    if (node->l_child->r_sibling->r_sibling->value->is_token)
    {
        return std::make_shared<FunctionSymbol>(
            GetKTreeNodeLineNumber(node->l_child),
            function_name,
            std::vector<VariableSymbolSharedPtr>(),
            nullptr);
    }
    // FunDec: ID L_BRACKET VarList R_BRACKET
    else
    {
        std::vector<VariableSymbolSharedPtr> filtered_args;

        auto args = DoVarList(node->l_child->r_sibling->r_sibling);

        for (auto &arg : args)
        {
            if (arg != nullptr)
            {
                filtered_args.push_back(arg);
            }
        }

        return std::make_shared<FunctionSymbol>(
            GetKTreeNodeLineNumber(node->l_child),
            function_name,
            filtered_args,
            nullptr);
    }
}

void IrGenerator::DoVarList(const KTreeNode *node)
{
    // VarList: ParamDec COMMA VarList | ParamDec
    std::vector<VariableSymbolSharedPtr> vars;

    while (node != NULL)
    {
        vars.push_back(DoParamDec(node->l_child));
        node = node->r_child;
    }

    return vars;
}

void IrGenerator::DoParamDec(const KTreeNode *node)
{
    // ParamDec: Specifier VarDec
    auto specifier = DoSpecifier(node->l_child);
    auto var_dec = DoVarDec(node->r_child);

    return DoDecListDefCommon(specifier, {var_dec})[0];
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
            DoDefList(node->l_child->r_sibling, true);

            // StmtList is not NULL either
            if (!node->l_child->r_sibling->r_sibling->value->is_token)
            {
                return DoStmtList(node->l_child->r_sibling->r_sibling);
            }
            // StmtList is NULL
            else
            {
                return std::vector<VariableSymbolSharedPtr>();
            }
        }
        // DefList is NULL and StmtList is not NULL
        else
        {
            return DoStmtList(node->l_child->r_sibling);
        }
    }

    // Both DefList and StmtList are NULL
    return std::vector<VariableSymbolSharedPtr>();
}

void IrGenerator::DoStmtList(const KTreeNode *node)
{
    // StmtList: Stmt StmtList(Nullable) | <NULL>
    std::vector<VariableSymbolSharedPtr> return_types;

    while (node != NULL)
    {
        auto current_return_types = DoStmt(node->l_child);

        return_types.insert(return_types.cend(), current_return_types.begin(), current_return_types.end());

        node = node->r_child;
    }

    return return_types;
}

void IrGenerator::DoStmt(const KTreeNode *node)
{
    if (!node->l_child->value->is_token)
    {
        // Stmt: Exp SEMICOLON
        if (node->l_child->value->ast_node_value.variable->type == VARIABLE_EXP)
        {
            DoExp(node->l_child);
            return std::vector<VariableSymbolSharedPtr>();
        }

        // Stmt: CompSt
        return DoCompSt(node->l_child);
    }

    switch (node->l_child->value->ast_node_value.token->type)
    {
    // Stmt: RETURN Exp SEMICOLON
    case TOKEN_KEYWORD_RETURN:
    {
        return {DoExp(node->l_child->r_sibling).first};
    }
    case TOKEN_KEYWORD_IF:
    {
        auto condition_exp_node = node->l_child->r_sibling->r_sibling;
        auto if_stmt_node = condition_exp_node->r_sibling->r_sibling;

        auto condition_exp = DoExp(condition_exp_node);

        // condition expression errors do not stop further analysis or we will lose tons of information
        if (condition_exp.first != nullptr)
        {
            if (!IsIntArithmeticSymbol(*condition_exp.first))
            {
                PrintError(kErrorOperandTypeMismatch,
                           GetKTreeNodeLineNumber(condition_exp_node),
                           "if condition expression must have 'int' type");
            }
        }

        // Stmt: IF L_BRACKET Exp R_BRACKET Stmt
        auto if_return_types = DoStmt(if_stmt_node);

        if (if_stmt_node->r_sibling == NULL)
        {
            return if_return_types;
        }

        // Stmt: IF L_BRACKET Exp R_BRACKET Stmt ELSE Stmt
        auto else_return_types = DoStmt(if_stmt_node->r_sibling->r_sibling);

        if_return_types.insert(if_return_types.cend(), else_return_types.begin(), else_return_types.end());

        return if_return_types;
    }
    // Stmt: WHILE L_BRACKET Exp R_BRACKET Stmt
    case TOKEN_KEYWORD_WHILE:
    {
        auto condition_exp = DoExp(node->l_child->r_sibling->r_sibling);

        // condition expression errors do not stop further analysis or we will lose tons of information
        if (condition_exp.first != nullptr)
        {
            if (!IsIntArithmeticSymbol(*condition_exp.first))
            {
                PrintError(kErrorOperandTypeMismatch,
                           GetKTreeNodeLineNumber(node->l_child->r_sibling->r_sibling),
                           "while condition expression must have 'int' type");
            }
        }

        return DoStmt(node->r_child);
    }
    default:
        return std::vector<VariableSymbolSharedPtr>();
    }
}

void IrGenerator::DoExp(const KTreeNode *node)
{
    const std::pair<VariableSymbolSharedPtr, bool> kNullptrFalse = {nullptr, false};

    if (node->l_child->value->is_token)
    {
        // Exp: ID | LITERAL_INT | LITERAL_FP /////////////////////////////
        if (node->r_child == NULL)
        {
            switch (node->l_child->value->ast_node_value.token->type)
            {
            case TOKEN_ID:
            {
                std::string variable_name = node->l_child->value->ast_node_value.token->value;
                if (symbol_table_.find(variable_name) != symbol_table_.end())
                {
                    return {symbol_table_.at(variable_name), true};
                }
                else
                {
                    PrintError(kErrorUndefinedVariable,
                               GetKTreeNodeLineNumber(node->l_child),
                               "Undefined variable '" + variable_name + '\'');
                    return kNullptrFalse;
                }
            }
            case TOKEN_LITERAL_INT:
            {
                return {std::make_shared<ArithmeticSymbol>(
                            GetKTreeNodeLineNumber(node->l_child),
                            "",
                            ArithmeticSymbolType::INT),
                        false};
            }
            case TOKEN_LITERAL_FP:
            {
                return {std::make_shared<ArithmeticSymbol>(
                            GetKTreeNodeLineNumber(node->l_child),
                            "",
                            ArithmeticSymbolType::FLOAT),
                        false};
            }
            default:
                return kNullptrFalse;
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
            // No such callee symbol
            if (symbol_table_.find(function_name) == symbol_table_.end())
            {
                PrintError(kErrorUndefinedFunction,
                           GetKTreeNodeLineNumber(node->l_child),
                           "Cannot find function '" + function_name + '\'');

                return kNullptrFalse;
            }

            auto function_variable_symbol = symbol_table_.at(function_name);
            // is not a function
            if (function_variable_symbol->GetVariableSymbolType() != VariableSymbolType::FUNCTION)
            {
                PrintError(kErrorInvalidInvokeOperator,
                           GetKTreeNodeLineNumber(node->l_child->r_sibling),
                           "A(n) '" +
                               GetVariableSymbolTypeName(symbol_table_.at(function_name)) +
                               "' variable is not callable");

                return kNullptrFalse;
            }

            auto function_symbol = static_cast<FunctionSymbol *>(function_variable_symbol.get());
            auto function_args = function_symbol->GetArgs();
            // Check args
            // Call with args
            auto args_node = node->l_child->r_sibling->r_sibling;
            if (!args_node->value->is_token)
            {
                auto args = DoArgs(args_node);

                if (args.size() != function_args.size())
                {
                    PrintError(kErrorFunctionArgsMismatch,
                               GetKTreeNodeLineNumber(args_node),
                               "Expected " +
                                   std::to_string(function_args.size()) +
                                   " arguments, but " +
                                   std::to_string(args.size()) +
                                   " were given");
                    return kNullptrFalse;
                }

                for (int i = 0; i < function_args.size(); i++)
                {
                    if (!IsAssignmentValid(*function_args[i], *args[i]))
                    {
                        PrintError(kErrorFunctionArgsMismatch,
                                   args[i]->GetLineNumber(),
                                   "Expected " +
                                       std::to_string(function_args.size()) +
                                       " arguments, but " +
                                       std::to_string(args.size()) +
                                       " given");
                        return kNullptrFalse;
                    }
                }
            }
            // Call with no args
            else
            {
                if (function_args.size() != 0)
                {
                    PrintError(kErrorFunctionArgsMismatch,
                               GetKTreeNodeLineNumber(args_node),
                               "Expected " +
                                   std::to_string(function_args.size()) +
                                   " arguments, but 0 given");
                    return kNullptrFalse;
                }
            }

            return {function_symbol->GetReturnType(), false};
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: SUB Exp ///////////////////////////////////////////////////
        if (node->l_child->value->ast_node_value.token->type == TOKEN_OPERATOR_SUB)
        {
            auto expression = DoExp(node->r_child);
            if (expression.first == nullptr)
            {
                return kNullptrFalse;
            }

            if (expression.first->GetVariableSymbolType() != VariableSymbolType::ARITHMETIC)
            {
                PrintError(kErrorOperandTypeMismatch,
                           expression.first->GetLineNumber(),
                           "Not an arithmetic type");
                return kNullptrFalse;
            }

            return {expression.first, false};
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: NOT Exp ///////////////////////////////////////////////////
        if (node->l_child->value->ast_node_value.token->type == TOKEN_OPERATOR_LOGICAL_NOT)
        {
            auto expression = DoExp(node->r_child);
            if (expression.first == nullptr)
            {
                return kNullptrFalse;
            }

            if (!IsIntArithmeticSymbol(*expression.first))
            {
                PrintError(kErrorOperandTypeMismatch,
                           expression.first->GetLineNumber(),
                           "Logical operator operand is not 'int' type");
                return kNullptrFalse;
            }

            return {expression.first, false};
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: L_BRACKET Exp R_BRACKET ///////////////////////////////////
        // This keeps l/r-value type
        return DoExp(node->l_child->r_sibling);
        ///////////////////////////////////////////////////////////////////
    }
    else
    {
        auto second_child = node->l_child->r_sibling;

        switch (second_child->value->ast_node_value.token->type)
        {
        case TOKEN_DELIMITER_L_SQUARE:
        {
            auto array_exp = DoExp(node->l_child);
            if (array_exp.first == nullptr)
            {
                return kNullptrFalse;
            }

            if (array_exp.first->GetVariableSymbolType() != VariableSymbolType::ARRAY)
            {
                PrintError(kErrorInvalidIndexOperator,
                           array_exp.first->GetLineNumber(),
                           '\'' + GetVariableSymbolTypeName(array_exp.first) + "' type is not indexable");

                return kNullptrFalse;
            }

            auto index_exp = DoExp(node->l_child);
            if (index_exp.first == nullptr)
            {
                return kNullptrFalse;
            }

            if (!IsIntArithmeticSymbol(*index_exp.first))
            {
                PrintError(kErrorIndexNotInteger,
                           index_exp.first->GetLineNumber(),
                           "Array index must be type 'int'");

                return kNullptrFalse;
            }

            return {static_cast<ArraySymbol *>(array_exp.first.get())->GetElemType(), true};
        }

        case TOKEN_OPERATOR_DOT:
        {
            auto struct_exp = DoExp(node->l_child);
            if (struct_exp.first == nullptr)
            {
                return kNullptrFalse;
            }

            if (struct_exp.first->GetVariableSymbolType() != VariableSymbolType::STRUCT)
            {
                PrintError(kErrorInvalidDotOperator,
                           struct_exp.first->GetLineNumber(),
                           "Dotted expression is not a struct");
                return kNullptrFalse;
            }

            std::string struct_name = static_cast<StructSymbol *>(struct_exp.first.get())->GetStructName();

            if (struct_def_symbol_table_.find(struct_name) == struct_def_symbol_table_.end())
            {
                return kNullptrFalse;
            }

            std::string field_name = node->r_child->value->ast_node_value.token->value;

            auto struct_fields = struct_def_symbol_table_.at(struct_name)->GetFields();

            auto selected_field = std::find_if(
                struct_fields.cbegin(),
                struct_fields.cend(),
                [&field_name](const VariableSymbolSharedPtr &field)
                {
                    return field->GetName() == field_name;
                });

            if (selected_field == struct_fields.cend())
            {
                PrintError(kErrorUndefinedStructField,
                           GetKTreeNodeLineNumber(node->r_child),
                           '\'' +
                               GetVariableSymbolTypeName(struct_exp.first) +
                               "' has no field '" +
                               field_name +
                               '\'');

                return kNullptrFalse;
            }

            return {*selected_field, true};
        }
        }

        // From now on, the exp can only be a binary operation

        auto l_exp = DoExp(node->l_child);
        if (l_exp.first == nullptr)
        {
            return kNullptrFalse;
        }

        auto r_exp = DoExp(node->r_child);
        if (r_exp.first == nullptr)
        {
            return kNullptrFalse;
        }

        switch (second_child->value->ast_node_value.token->type)
        {
        case TOKEN_OPERATOR_ASSIGN:
        {
            if (!l_exp.second)
            {
                PrintError(kErrorAssignToRValue,
                           l_exp.first->GetLineNumber(),
                           "Cannot assign to a right value");

                return kNullptrFalse;
            }

            if (!IsAssignmentValid(*l_exp.first, *r_exp.first))
            {
                PrintError(
                    kErrorAssignTypeMismatch,
                    r_exp.first->GetLineNumber(),
                    "Cannot assign '" +
                        GetVariableSymbolTypeName(r_exp.first) +
                        "' to a variable of type '" +
                        GetVariableSymbolTypeName(l_exp.first));

                return kNullptrFalse;
            }

            return {r_exp.first, false};
        }

        case TOKEN_OPERATOR_LOGICAL_AND:
        case TOKEN_OPERATOR_LOGICAL_OR:
        {
            if (!IsIntArithmeticSymbol(*l_exp.first))
            {
                PrintError(kErrorOperandTypeMismatch,
                           l_exp.first->GetLineNumber(),
                           "Logical operator operand is not 'int' type");
                return kNullptrFalse;
            }

            if (!IsIntArithmeticSymbol(*r_exp.first))
            {
                PrintError(kErrorOperandTypeMismatch,
                           r_exp.first->GetLineNumber(),
                           "Logical operator operand is not 'int' type");
                return kNullptrFalse;
            }

            return {l_exp.first, false};
        }

        case TOKEN_OPERATOR_REL_EQ:
        case TOKEN_OPERATOR_REL_GE:
        case TOKEN_OPERATOR_REL_GT:
        case TOKEN_OPERATOR_REL_LE:
        case TOKEN_OPERATOR_REL_LT:
        case TOKEN_OPERATOR_REL_NE:
        {
            if (!IsSameTypeArithmeticSymbol(*l_exp.first, *r_exp.first))
            {
                PrintError(kErrorOperandTypeMismatch,
                           l_exp.first->GetLineNumber(),
                           "Relational operator must take two operands of an identical arithmetic type");

                return kNullptrFalse;
            }

            return {std::make_shared<ArithmeticSymbol>(
                        l_exp.first->GetLineNumber(),
                        "",
                        ArithmeticSymbolType::INT),
                    false};
        }

        case TOKEN_OPERATOR_ADD:
        case TOKEN_OPERATOR_SUB:
        case TOKEN_OPERATOR_MUL:
        case TOKEN_OPERATOR_DIV:
        {
            if (!IsSameTypeArithmeticSymbol(*l_exp.first, *r_exp.first))
            {
                PrintError(kErrorOperandTypeMismatch,
                           l_exp.first->GetLineNumber(),
                           "Mathematical operator must take two operands of an identical arithmetic type");

                return kNullptrFalse;
            }

            return {l_exp.first, false};
        }

        default:
            return kNullptrFalse;
        }
    }
}

void IrGenerator::DoArgs(const KTreeNode *node)
{
    // Args: Exp COMMA Args | Exp
    std::vector<VariableSymbolSharedPtr> args;

    while (node != NULL)
    {
        args.push_back(DoExp(node->l_child).first);
        node = node->r_child;
    }

    return args;
}