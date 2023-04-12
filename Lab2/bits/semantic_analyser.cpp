#include "semantic_analyser.h"

void SemanticAnalyser::Analyse(KTreeNode *node, size_t, void *)
{
    if (!node->value->is_token && node->value->ast_node_value.variable->type == VARIABLE_EXT_DEF_LIST)
    {
        DoExtDefList(node);
    }
}

int SemanticAnalyser::GetKTreeNodeLineNumber(const KTreeNode *node) const
{
    return node->value->is_token
               ? node->value->ast_node_value.token->line_start
               : node->value->ast_node_value.variable->line_start;
}

// symbol cannot be nullptr
std::string SemanticAnalyser::GetVariableSymbolTypeName(const VariableSymbolSharedPtr &symbol) const
{
    return GetVariableSymbolTypeName(symbol.get());
}

// symbol cannot be nullptr
std::string SemanticAnalyser::GetVariableSymbolTypeName(const VariableSymbol *symbol) const
{
    const VariableSymbol *variable_symbol = static_cast<const VariableSymbol *>(symbol);
    switch (variable_symbol->VariableSymbolType())
    {
    case VariableSymbolType::ARITHMETIC:
        switch (static_cast<const ArithmeticSymbol *>(variable_symbol)->ArithmeticSymbolType())
        {
        case ArithmeticSymbolType::INT:
            return "int";
        case ArithmeticSymbolType::FLOAT:
            return "float";
        case ArithmeticSymbolType::UNKNOWN:
        default:
            return "<UNKNOWN ARITHMETIC>";
        }

    case VariableSymbolType::ARRAY:
    {
        std::string array_type_name;
        const ArraySymbol *array_symbol = static_cast<const ArraySymbol *>(variable_symbol);
        while (array_symbol->VariableSymbolType() == VariableSymbolType::ARRAY)
        {
            array_type_name = "[]" + array_type_name;
            array_symbol = static_cast<ArraySymbol *>(array_symbol->ElemType().get());
        }

        array_type_name = GetVariableSymbolTypeName(array_symbol) + ' ' + array_type_name;

        return array_type_name;
    }

    case VariableSymbolType::FUNCTION:
        return "function";

    case VariableSymbolType::STRUCT:
        return "struct " + static_cast<const StructSymbol *>(variable_symbol)->StructName();

    case VariableSymbolType::UNKNOWN:
    default:
        return "<UNKNOWN VARIABLE>";
    }
}

void SemanticAnalyser::PrintError(
    const int type, const int line_number, const std::string &message)
{
    has_semantic_error_ = true;
    std::cerr << "Error type " << type << " at Line " << line_number << ": " << message << std::endl;
}

std::string SemanticAnalyser::GetNewAnnoyStructName()
{
    std::string annoy_name_prefix = "[struct_annoy]";
    std::string new_annoy_name;

    do
    {
        std::ostringstream oss;
        oss << std::hex << distribution_(mt19937_);
        new_annoy_name = annoy_name_prefix + oss.str();
    } while (struct_def_symbol_table_.contains(new_annoy_name));

    return new_annoy_name;
}

bool SemanticAnalyser::IsIntArithmeticSymbol(const VariableSymbol &var) const
{
    return var.VariableSymbolType() == VariableSymbolType::ARITHMETIC &&
           static_cast<const ArithmeticSymbol *>(&var)->ArithmeticSymbolType() ==
               ArithmeticSymbolType::INT;
}

bool SemanticAnalyser::IsSameTypeArithmeticSymbol(const VariableSymbol &var1, const VariableSymbol &var2) const
{
    if (var1.VariableSymbolType() != VariableSymbolType::ARITHMETIC ||
        var2.VariableSymbolType() != VariableSymbolType::ARITHMETIC)
    {
        return false;
    }

    return static_cast<const ArithmeticSymbol *>(&var1)->ArithmeticSymbolType() ==
           static_cast<const ArithmeticSymbol *>(&var2)->ArithmeticSymbolType();
}

bool SemanticAnalyser::IsAssignmentValid(
    const VariableSymbol &var_l,
    const VariableSymbol &var_r) const
{
    if (var_l.VariableSymbolType() != var_r.VariableSymbolType())
    {
        return false;
    }

    switch (var_l.VariableSymbolType())
    {
    case VariableSymbolType::ARITHMETIC:
    {
        return IsSameTypeArithmeticSymbol(var_l, var_r);
    }
    case VariableSymbolType::ARRAY:
    {
        const ArraySymbol *array1 = static_cast<const ArraySymbol *>(&var_l);
        const ArraySymbol *array2 = static_cast<const ArraySymbol *>(&var_r);

        while (array1->VariableSymbolType() == VariableSymbolType::ARRAY &&
               array2->VariableSymbolType() == VariableSymbolType::ARRAY)
        {
            array1 = static_cast<const ArraySymbol *>(array1->ElemType().get());
            array2 = static_cast<const ArraySymbol *>(array2->ElemType().get());
        }

        // different dim
        if (array1->VariableSymbolType() == VariableSymbolType::ARRAY ||
            array2->VariableSymbolType() == VariableSymbolType::ARRAY)
        {
            return false;
        }

        return IsAssignmentValid(*array1, *array2);
    }
    case VariableSymbolType::STRUCT:
    {
        const StructSymbol *struct1 = static_cast<const StructSymbol *>(&var_l);
        const StructSymbol *struct2 = static_cast<const StructSymbol *>(&var_r);

        if (!struct_def_symbol_table_.contains(struct1->StructName()) ||
            !struct_def_symbol_table_.contains(struct2->StructName()))
        {
            return false;
        }

        return IsStructAssignmentValid(
            *struct_def_symbol_table_.at(struct1->StructName()),
            *struct_def_symbol_table_.at(struct2->StructName()));
    }
    default:
        return false;
    }
}

bool SemanticAnalyser::IsStructAssignmentValid(
    const StructDefSymbol &def_l,
    const StructDefSymbol &def_r) const
{
    auto fields1 = def_l.Fields();
    auto fields2 = def_r.Fields();
    if (fields1.size() != fields2.size())
    {
        return false;
    }

    for (int i = 0; i < fields1.size(); i++)
    {
        if (!IsAssignmentValid(*fields1[i], *fields2[i]))
        {
            return false;
        }
    }

    return true;
}

// [INSERTS] VariableSymbol
// [CHECKS] kErrorDuplicateVariableName
void SemanticAnalyser::InsertVariableSymbol(const VariableSymbolSharedPtr &symbol)
{
    if (symbol == nullptr)
    {
        return;
    }

    if (symbol_table_.contains(symbol->Name()))
    {
        PrintError(kErrorDuplicateVariableName,
                   symbol->LineNumber(),
                   "Duplicate variable name: '" + symbol->Name() + '\'');
    }
    else
    {
        symbol_table_[symbol->Name()] = symbol;
    }
}

void SemanticAnalyser::DoExtDefList(const KTreeNode *node)
{
    // ExtDefList: ExtDef ExtDefList | <NULL>
    while (node != NULL)
    {
        DoExtDef(node->l_child);
        node = node->r_child;
    }
}

// [COMBINATION] Combines specifier and dec_list <VIA DoDecListDefCommon>
// [INSERTS] VariableSymbol <VIA InsertVariableSymbol>
// [CHECKS] kErrorDuplicateVariableName <VIA InsertVariableSymbol>,
//          kErrorDuplicateFunctionName,
//          kErrorAssignTypeMismatch <VIA DoDecListDefCommon>,
//          kErrorUndefinedStruct <VIA DoDecListDefCommon>
void SemanticAnalyser::DoExtDef(const KTreeNode *node)
{
    // ExtDef: Specifier SEMICOLON
    if (node->l_child->r_sibling->value->is_token)
    {
        return;
    }

    auto specifier = DoSpecifier(node->l_child);
    if (specifier == nullptr)
    {
        return;
    }

    // ExtDef: Specifier ExtDecList SEMICOLON
    if (node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_EXT_DEC_LIST)
    {
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
    }
}

// [COMBINATION] Combines specifier and dec_list
// [CHECKS] kErrorAssignTypeMismatch,
//          kErrorUndefinedStruct
// Return a list of symbol definitions, each of which contains full symbol information.
// Type info in specifier is combined with each element in dec_list.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoDecListDefCommon(
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
        else if (dec->VariableSymbolType() == VariableSymbolType::UNKNOWN)
        {
            // Arithmetic
            if (specifier->VariableSymbolType() == VariableSymbolType::ARITHMETIC)
            {
                if (dec->IsInitialized() &&
                    !IsAssignmentValid(*specifier, *(dec->InitialValue())))
                {
                    PrintError(
                        kErrorAssignTypeMismatch,
                        dec->LineNumber(),
                        "Cannot assign '" +
                            GetVariableSymbolTypeName(dec) +
                            "' to a variable of type '" +
                            GetVariableSymbolTypeName(specifier));

                    continue;
                }

                defs.push_back(std::make_shared<ArithmeticSymbol>(
                    dec->LineNumber(),
                    dec->Name(),
                    static_cast<ArithmeticSymbol *>(specifier.get())->ArithmeticSymbolType(),
                    dec->IsInitialized(),
                    dec->InitialValue()));
            }
            // Struct
            else
            {
                std::string struct_name = static_cast<StructSymbol *>(specifier.get())->StructName();
                if (!struct_def_symbol_table_.contains(struct_name))
                {
                    PrintError(kErrorUndefinedStruct, dec->LineNumber(),
                               "Undefined struct type: " + GetVariableSymbolTypeName(specifier));
                    continue;
                }

                // Check assignment compatibility
                if (dec->IsInitialized() &&
                    !IsAssignmentValid(*specifier, *(dec->InitialValue())))
                {
                    PrintError(
                        kErrorAssignTypeMismatch,
                        dec->LineNumber(),
                        "Cannot assign '" +
                            GetVariableSymbolTypeName(dec) +
                            "' to a variable of type '" +
                            GetVariableSymbolTypeName(specifier));

                    continue;
                }

                defs.push_back(std::make_shared<StructSymbol>(
                    dec->LineNumber(),
                    dec->Name(),
                    struct_name,
                    dec->IsInitialized(),
                    dec->InitialValue()));
            }
        }
        // array
        else
        {
            ArraySymbol *array_dec = static_cast<ArraySymbol *>(dec.get());
            while (array_dec->ElemType()->VariableSymbolType() ==
                   VariableSymbolType::ARRAY)
            {
                array_dec = static_cast<ArraySymbol *>(array_dec->ElemType().get());
            }

            if (specifier->VariableSymbolType() == VariableSymbolType::ARITHMETIC)
            {
                array_dec->SetElemType(std::make_shared<ArithmeticSymbol>(
                    array_dec->ElemType()->LineNumber(),
                    array_dec->ElemType()->Name(),
                    static_cast<ArithmeticSymbol *>(specifier.get())->ArithmeticSymbolType(),
                    array_dec->ElemType()->IsInitialized(),
                    array_dec->ElemType()->InitialValue()));
            }
            else
            {
                array_dec->SetElemType(std::make_shared<StructSymbol>(
                    array_dec->ElemType()->LineNumber(),
                    array_dec->ElemType()->Name(),
                    static_cast<StructSymbol *>(specifier.get())->StructName(),
                    array_dec->ElemType()->IsInitialized(),
                    array_dec->ElemType()->InitialValue()));
            }

            // Check assignment compatibility
            if (dec->IsInitialized() &&
                !IsAssignmentValid(*specifier, *(dec->InitialValue())))
            {
                PrintError(
                    kErrorAssignTypeMismatch,
                    dec->LineNumber(),
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

// Returns a list of DoVarDec results.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoExtDecList(const KTreeNode *node)
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

// Return value contains type info.
// Returns either an ArithmeticSymbol or a StructSymbol.
VariableSymbolSharedPtr SemanticAnalyser::DoSpecifier(const KTreeNode *node)
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

// [INSERTS] StructDefSymbol
// [CHECKS] kErrorDuplicateStructName,
//          kErrorDuplicateStructFieldName,
//          kErrorStructFieldInitialized
// For named struct def, check the existence of the def,
// add the def to symtable and return a StructSymbol.
// For unnamed struct def, create a new random name for it,
// add the def to symtable and return a StructSymbol.
// For named struct, check the existence of the def and
// return a StructSymbol.
// Return value contains struct name.
std::shared_ptr<StructSymbol> SemanticAnalyser::DoStructSpecifier(const KTreeNode *node)
{
    std::string struct_name;

    // StructSpecifier: STRUCT Tag
    if (!node->l_child->r_sibling->value->is_token &&
        node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_TAG)
    {
        KTreeNode *struct_id_node = node->l_child->r_sibling->l_child;
        struct_name =
            struct_id_node->value->ast_node_value.token->value;
        if (struct_def_symbol_table_.contains(struct_name))
        {
            return std::make_shared<StructSymbol>(
                GetKTreeNodeLineNumber(struct_id_node), "", struct_name);
        }

        PrintError(kErrorUndefinedStruct,
                   GetKTreeNodeLineNumber(struct_id_node),
                   "Struct '" + struct_name + "' is not defined");
        return nullptr;
    }
    // StructSpecifier: STRUCT OptTag L_BRACE DefList R_BRACE
    else
    {
        KTreeNode *def_list_node = nullptr;
        // Unnamed struct def
        // StructSpecifier: STRUCT L_BRACE DefList R_BRACE
        if (node->l_child->r_sibling->value->is_token)
        {
            struct_name = GetNewAnnoyStructName();
            def_list_node = node->l_child->r_sibling->r_sibling;
        }
        // Named struct def
        // StructSpecifier: STRUCT OptTag L_BRACE DefList R_BRACE
        else
        {
            struct_name = node->l_child->r_sibling->l_child->value->ast_node_value.token->value;
            def_list_node = node->l_child->r_sibling->r_sibling->r_sibling;

            if (struct_def_symbol_table_.contains(struct_name))
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
            fields = DoDefList(def_list_node);
        }

        // Checks kErrorDuplicateStructFieldName
        std::unordered_set<std::string> field_names;
        for (auto &field : fields)
        {
            if (field == nullptr)
            {
                continue;
            }

            if (field_names.contains(field->Name()))
            {
                PrintError(kErrorDuplicateStructFieldName,
                           field->LineNumber(),
                           "Duplicate field '" + field->Name() + "'");
                return nullptr;
            }
            else
            {
                field_names.insert(field->Name());
            }
        }

        // Checks kErrorStructFieldInitialized
        for (auto &field : fields)
        {
            if (field == nullptr)
            {
                continue;
            }

            if (field->IsInitialized())
            {
                PrintError(kErrorStructFieldInitialized,
                           field->LineNumber(),
                           "Struct field '" + field->Name() + "' is initialized");

                return nullptr;
            }
        }

        struct_def_symbol_table_[struct_name] = std::make_shared<StructDefSymbol>(
            GetKTreeNodeLineNumber(node->l_child), struct_name, fields);

        return std::make_shared<StructSymbol>(GetKTreeNodeLineNumber(node->l_child), "", struct_name);
    }
}

// Return a list of symbol definitions, each of which contains full symbol information.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoDefList(const KTreeNode *node)
{
    // DefList: Def DefList | <NULL>
    std::vector<VariableSymbolSharedPtr> defs;
    while (node != NULL)
    {
        auto current_defs = DoDef(node->l_child);
        defs.insert(defs.cend(), current_defs.begin(), current_defs.end());
        node = node->r_child;
    }

    return defs;
}

// Return a list of symbol definitions, each of which contains full symbol information.
// Type info is obtained by DoSpecifier, which is combined with each element
// in the list returned by DoDecList with help of DoDecListDefCommon.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoDef(const KTreeNode *node)
{
    // Def: Specifier DecList SEMICOLON
    auto specifier = DoSpecifier(node->l_child);
    auto dec_list = DoDecList(node->l_child->r_sibling);

    return DoDecListDefCommon(specifier, dec_list);
}

// Returns a list of DoDec results.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoDecList(const KTreeNode *node)
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

// Return value contains variable name, is variable/array, initialization info.
VariableSymbolSharedPtr SemanticAnalyser::DoDec(const KTreeNode *node)
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
        var_dec->LineNumber(),
        var_dec->Name(),
        var_dec->VariableSymbolType(),
        initial_value.first != nullptr,
        initial_value.first);
}

// Return value contains variable name and whether is array.
// Returns either an UNKNOWN VariableSymbol containing name
// or an ArraySymbol with name and element symbol.
VariableSymbolSharedPtr SemanticAnalyser::DoVarDec(const KTreeNode *node)
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
    auto array_element = DoVarDec(node->l_child);
    if (array_element == nullptr)
    {
        return nullptr;
    }

    return std::make_shared<ArraySymbol>(
        array_element->LineNumber(),
        array_element->Name(),
        array_element,
        std::stoull(node->l_child->r_sibling->r_sibling->value->ast_node_value.token->value));
}

// Returns a function symbol containing name and arguments information.
// When one argument declaration contains semantic error(is a nullptr),
// then it is ignored as if the function didn't take that argument.
std::shared_ptr<FunctionSymbol> SemanticAnalyser::DoFunDec(const KTreeNode *node)
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

// Return a list of param declarations, each of which contains full symbol information.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoVarList(const KTreeNode *node)
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

// [COMBINATION] Combines specifier and dec_list <VIA DoDecListDefCommon>
// [CHECKS] kErrorAssignTypeMismatch <VIA DoDecListDefCommon>,
//          kErrorUndefinedStruct <VIA DoDecListDefCommon>
// Return a param declaration containing full symbol information.
VariableSymbolSharedPtr SemanticAnalyser::DoParamDec(const KTreeNode *node)
{
    // ParamDec: Specifier VarDec
    auto specifier = DoSpecifier(node->l_child);
    auto var_dec = DoVarDec(node->r_child);

    return DoDecListDefCommon(specifier, {var_dec})[0];
}

// [INSERTS] VariableSymbol <VIA InsertVariableSymbol>
// [CHECKS] kErrorDuplicateVariableName <VIA InsertVariableSymbol>
// Returns the return types of the first statement.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoCompSt(const KTreeNode *node)
{
    // CompSt: L_BRACE DefList StmtList R_BRACE
    auto defs = DoDefList(node->l_child->r_sibling);
    for (auto &def : defs)
    {
        InsertVariableSymbol(def);
    }
}

// Returns the return types of the first statement.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoStmtList(const KTreeNode *node)
{
    // StmtList: Stmt StmtList | <NULL>
    std::vector<VariableSymbolSharedPtr> statement_return_types;
    bool is_first_statement = true;

    while (node != NULL)
    {
        if (is_first_statement)
        {
            statement_return_types = DoStmt(node->l_child);
            is_first_statement = false;
        }
        else
        {
            DoStmt(node->l_child);
        }

        node = node->r_child;
    }

    return statement_return_types;
}

// [CHECKS] kErrorOperandTypeMismatch
// If the statement is a return statement, returns a vector containing the return value type.
// In the IF...ELSE... case which may contain two parallel return statements,
// both return value types are stored in the vector.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoStmt(const KTreeNode *node)
{
    if (!node->l_child->value->is_token)
    {
        // Stmt: Exp SEMICOLON
        if (node->l_child->value->ast_node_value.variable->type == VARIABLE_EXP)
        {
            DoExp(node->l_child);
            return {nullptr};
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
        return {nullptr};
    }
}

// [CHECKS] kErrorUndefinedVariable,
//          kErrorUndefinedFunction,
//          kErrorInvalidInvokeOperator,
//          kErrorFunctionArgsMismatch,
//          kErrorInvalidIndexOperator,
//          kErrorIndexNotInteger,
//          kErrorInvalidDotOperator,
//          kErrorUndefinedStructField,
//          kErrorAssignToRValue,
//          kErrorAssignTypeMismatch,
// Returns <exp-type, is-l-value>.
std::pair<VariableSymbolSharedPtr, bool> SemanticAnalyser::DoExp(const KTreeNode *node)
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
                if (symbol_table_.contains(variable_name))
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
            if (!symbol_table_.contains(function_name))
            {
                PrintError(kErrorUndefinedFunction,
                           GetKTreeNodeLineNumber(node->l_child),
                           "Cannot find function '" + function_name + '\'');

                return kNullptrFalse;
            }

            auto function_variable_symbol = symbol_table_.at(function_name);
            // is not a function
            if (function_variable_symbol->VariableSymbolType() != VariableSymbolType::FUNCTION)
            {
                PrintError(kErrorInvalidInvokeOperator,
                           GetKTreeNodeLineNumber(node->l_child->r_sibling),
                           "A(n) '" +
                               GetVariableSymbolTypeName(symbol_table_.at(function_name)) +
                               "' variable is not callable");

                return kNullptrFalse;
            }

            auto function_symbol = static_cast<FunctionSymbol *>(function_variable_symbol.get());
            auto function_args = function_symbol->Args();
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
                                   args[i]->LineNumber(),
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

            return {function_symbol->ReturnType(), false};
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

            if (expression.first->VariableSymbolType() != VariableSymbolType::ARITHMETIC)
            {
                PrintError(kErrorOperandTypeMismatch,
                           expression.first->LineNumber(),
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
                           expression.first->LineNumber(),
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

            if (array_exp.first->VariableSymbolType() != VariableSymbolType::ARRAY)
            {
                PrintError(kErrorInvalidIndexOperator,
                           array_exp.first->LineNumber(),
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
                           index_exp.first->LineNumber(),
                           "Array index must be type 'int'");

                return kNullptrFalse;
            }

            return {static_cast<ArraySymbol *>(array_exp.first.get())->ElemType(), true};
        }

        case TOKEN_OPERATOR_DOT:
        {
            auto struct_exp = DoExp(node->l_child);
            if (struct_exp.first == nullptr)
            {
                return kNullptrFalse;
            }

            if (struct_exp.first->VariableSymbolType() != VariableSymbolType::STRUCT)
            {
                PrintError(kErrorInvalidDotOperator,
                           struct_exp.first->LineNumber(),
                           "Dotted expression is not a struct");
                return kNullptrFalse;
            }

            std::string struct_name = static_cast<StructSymbol *>(struct_exp.first.get())->StructName();

            if (!struct_def_symbol_table_.contains(struct_name))
            {
                return kNullptrFalse;
            }

            std::string field_name = node->r_child->value->ast_node_value.token->value;

            auto struct_fields = struct_def_symbol_table_.at(struct_name)->Fields();

            auto selected_field = std::find_if(
                struct_fields.cbegin(),
                struct_fields.cend(),
                [&field_name](const VariableSymbolSharedPtr &field)
                {
                    return field->Name() == field_name;
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
                           l_exp.first->LineNumber(),
                           "Cannot assign to a right value");

                return kNullptrFalse;
            }

            if (!IsAssignmentValid(*l_exp.first, *r_exp.first))
            {
                PrintError(
                    kErrorAssignTypeMismatch,
                    r_exp.first->LineNumber(),
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
                           l_exp.first->LineNumber(),
                           "Logical operator operand is not 'int' type");
                return kNullptrFalse;
            }

            if (!IsIntArithmeticSymbol(*r_exp.first))
            {
                PrintError(kErrorOperandTypeMismatch,
                           r_exp.first->LineNumber(),
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
                           l_exp.first->LineNumber(),
                           "Relational operator must take two operands of an identical arithmetic type");

                return kNullptrFalse;
            }

            return {std::make_shared<ArithmeticSymbol>(
                        l_exp.first->LineNumber(),
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
                           l_exp.first->LineNumber(),
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

// Returns a list of DoExp results.
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoArgs(const KTreeNode *node)
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