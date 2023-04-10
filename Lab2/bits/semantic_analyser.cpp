#include "semantic_analyser.h"

void SemanticAnalyser::Build(KTreeNode *node, size_t, void *)
{
    if (!node->value->is_token && node->value->ast_node_value.variable->type == VARIABLE_EXT_DEF_LIST)
    {
        DoExtDefList(node);
    }
}

int SemanticAnalyser::GetLineNumber(const KTreeNode *node) const
{
    return node->value->is_token
               ? node->value->ast_node_value.token->line_start
               : node->value->ast_node_value.variable->line_start;
}

std::string SemanticAnalyser::GetSymbolTypeName(const SymbolSharedPtr &symbol) const
{
    return GetSymbolTypeName(symbol.get());
}

std::string SemanticAnalyser::GetSymbolTypeName(const Symbol *symbol) const
{
    switch (symbol->SymbolType())
    {
    case SymbolType::STRUCT_DEF:
        return "structdef " + static_cast<const StructDefSymbol *>(symbol)->Name();
    case SymbolType::VARIABLE:
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

            array_type_name = GetSymbolTypeName(array_symbol) + ' ' + array_type_name;

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

    case SymbolType::UNKNOWN:
    default:
        return "<UNKNOWN>";
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
    } while (symbol_table_.contains(new_annoy_name));

    return new_annoy_name;
}

bool SemanticAnalyser::CheckAssignmentTypeCompatibility(
    const VariableSymbol &var1,
    const VariableSymbol &var2) const
{
    if (var1.VariableSymbolType() != var2.VariableSymbolType())
    {
        return false;
    }

    switch (var1.VariableSymbolType())
    {
    case VariableSymbolType::ARITHMETIC:
    {
        const ArithmeticSymbol *arithmetic1 = static_cast<const ArithmeticSymbol *>(&var1);
        const ArithmeticSymbol *arithmetic2 = static_cast<const ArithmeticSymbol *>(&var2);

        return arithmetic1->ArithmeticSymbolType() == arithmetic2->ArithmeticSymbolType();
    }
    case VariableSymbolType::ARRAY:
    {
        const ArraySymbol *array1 = static_cast<const ArraySymbol *>(&var1);
        const ArraySymbol *array2 = static_cast<const ArraySymbol *>(&var2);

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

        return CheckAssignmentTypeCompatibility(*array1, *array2);
    }
    case VariableSymbolType::STRUCT:
    {
        const StructSymbol *struct1 = static_cast<const StructSymbol *>(&var1);
        const StructSymbol *struct2 = static_cast<const StructSymbol *>(&var2);

        if (!symbol_table_.contains(struct1->StructName()) ||
            !symbol_table_.contains(struct2->StructName()))
        {
            return false;
        }

        return CheckStructAssignmentTypeCompatibility(
            *(static_cast<const StructDefSymbol *>(
                symbol_table_.at(struct1->StructName()).get())),
            *(static_cast<const StructDefSymbol *>(
                symbol_table_.at(struct2->StructName()).get())));
    }
    default:
        return false;
    }
}

bool SemanticAnalyser::CheckStructAssignmentTypeCompatibility(
    const StructDefSymbol &def1,
    const StructDefSymbol &def2) const
{
    auto fields1 = def1.Fields();
    auto fields2 = def2.Fields();
    if (fields1.size() != fields2.size())
    {
        return false;
    }

    for (int i = 0; i < fields1.size(); i++)
    {
        if (!CheckAssignmentTypeCompatibility(*fields1[i], *fields2[i]))
        {
            return false;
        }
    }

    return true;
}

void SemanticAnalyser::DoExtDefList(KTreeNode *node)
{
    // ExtDefList: ExtDef ExtDefList
    while (node != NULL)
    {
        DoExtDef(node->l_child);

        node = node->l_child->r_sibling;
    }
}

// [CHECKS] kErrorDuplicateVariableName,
//          kErrorDuplicateFunctionName
void SemanticAnalyser::DoExtDef(KTreeNode *node)
{
    // ExtDef: Specifier SEMICOLON
    if (node->l_child->r_sibling->value->is_token)
    {
        return;
    }

    auto specifier = DoSpecifier(node->l_child);

    // ExtDef: Specifier ExtDecList SEMICOLON
    if (node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_EXT_DEC_LIST)
    {
        auto dec_list = DoExtDecList(node->l_child->r_sibling);

        auto defs = DoDecListDefCommon(specifier, dec_list);

        for (auto &def : defs)
        {
            if (symbol_table_.contains(def->Name()))
            {
                PrintError(kErrorDuplicateVariableName,
                           def->LineNumber(),
                           "Duplicate variable name: '" + def->Name() + '\'');
            }
            else
            {
                symbol_table_[def->Name()] = def;
            }
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
std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoDecListDefCommon(
    const VariableSymbolSharedPtr &specifier,
    const std::vector<VariableSymbolSharedPtr> &dec_list)
{
    std::vector<VariableSymbolSharedPtr> defs;

    for (auto &dec : dec_list)
    {
        if (dec->VariableSymbolType() == VariableSymbolType::UNKNOWN)
        {
            // Arithmetic
            if (specifier->VariableSymbolType() == VariableSymbolType::ARITHMETIC)
            {
                if (dec->IsInitialized() &&
                    !CheckAssignmentTypeCompatibility(*specifier, *(dec->InitialValue())))
                {
                    PrintError(
                        kErrorAssignTypeMismatch,
                        dec->LineNumber(),
                        "Cannot assign '" +
                            GetSymbolTypeName(dec) +
                            "' to a variable of type '" +
                            GetSymbolTypeName(specifier) +
                            '\n');

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
                if (!symbol_table_.contains(struct_name) ||
                    symbol_table_[struct_name]->SymbolType() != SymbolType::STRUCT_DEF)
                {
                    PrintError(kErrorUndefinedStruct, dec->LineNumber(),
                               "Undefined struct type: " + GetSymbolTypeName(specifier));
                    continue;
                }

                // Check assignment compatibility
                if (dec->IsInitialized() &&
                    !CheckAssignmentTypeCompatibility(*specifier, *(dec->InitialValue())))
                {
                    PrintError(
                        kErrorAssignTypeMismatch,
                        dec->LineNumber(),
                        "Cannot assign '" +
                            GetSymbolTypeName(dec) +
                            "' to a variable of type '" +
                            GetSymbolTypeName(specifier) +
                            '\n');

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
                !CheckAssignmentTypeCompatibility(*specifier, *(dec->InitialValue())))
            {
                PrintError(
                    kErrorAssignTypeMismatch,
                    dec->LineNumber(),
                    "Cannot assign '" +
                        GetSymbolTypeName(dec) +
                        "' to a variable of type '" +
                        GetSymbolTypeName(specifier) +
                        '\n');

                continue;
            }

            defs.push_back(dec);
        }
    }

    return defs;
}

std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoExtDecList(KTreeNode *node)
{
    // DecList: Dec | Dec COMMA DecList
    std::vector<VariableSymbolSharedPtr> decs;

    while (node->l_child->r_sibling != NULL)
    {
        auto dec = DoVarDec(node->l_child);
        if (dec != nullptr)
        {
            decs.push_back(dec);
        }
        node = node->l_child->r_sibling->r_sibling;
    }

    return decs;
}

// Return value is used to provide type info
// Returns either an ArithmeticSymbol or a StructSymbol
VariableSymbolSharedPtr SemanticAnalyser::DoSpecifier(KTreeNode *node)
{
    // Specifier: TYPE_INT | TYPE_FLOAT
    if (node->l_child->value->is_token)
    {
        return std::make_shared<ArithmeticSymbol>(
            GetLineNumber(node->l_child),
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
std::shared_ptr<StructSymbol> SemanticAnalyser::DoStructSpecifier(KTreeNode *node)
{
    std::string struct_name;

    // StructSpecifier: STRUCT Tag
    if (!node->l_child->r_sibling->value->is_token &&
        node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_TAG)
    {
        KTreeNode *struct_id_node = node->l_child->r_sibling->l_child;
        struct_name =
            struct_id_node->value->ast_node_value.token->value;
        if (symbol_table_.contains(struct_name) &&
            symbol_table_[struct_name]->SymbolType() == SymbolType::STRUCT_DEF)
        {
            return std::make_shared<StructSymbol>(
                GetLineNumber(struct_id_node), "", struct_name);
        }

        PrintError(kErrorUndefinedStruct,
                   GetLineNumber(struct_id_node),
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

            if (symbol_table_.contains(struct_name))
            {
                PrintError(kErrorDuplicateStructName,
                           GetLineNumber(node->l_child->r_sibling->l_child),
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
            if (field->IsInitialized())
            {
                PrintError(kErrorStructFieldInitialized,
                           field->LineNumber(),
                           "Struct field '" + field->Name() + "' is initialized");

                return nullptr;
            }
        }

        symbol_table_[struct_name] = std::make_shared<StructDefSymbol>(
            GetLineNumber(node->l_child), struct_name, fields);

        return std::make_shared<StructSymbol>(GetLineNumber(node->l_child), "", struct_name);
    }
}

std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoDefList(KTreeNode *node)
{
    // DefList: Def DefList
    std::vector<VariableSymbolSharedPtr> defs;
    while (node != NULL)
    {
        auto current_defs = DoDef(node->l_child);
        defs.insert(defs.cend(), current_defs.begin(), current_defs.end());
        node = node->l_child->r_sibling;
    }

    return defs;
}

std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoDef(KTreeNode *node)
{
    // Def: Specifier DecList SEMICOLON
    auto specifier = DoSpecifier(node->l_child);
    auto dec_list = DoDecList(node->l_child->r_sibling);

    return DoDecListDefCommon(specifier, dec_list);
}

std::vector<VariableSymbolSharedPtr> SemanticAnalyser::DoDecList(KTreeNode *node)
{
    // DecList: Dec | Dec COMMA DecList
    std::vector<VariableSymbolSharedPtr> decs;

    while (node->l_child->r_sibling != NULL)
    {
        auto dec = DoDec(node->l_child);
        if (dec != nullptr)
        {
            decs.push_back(dec);
        }
        node = node->l_child->r_sibling->r_sibling;
    }

    return decs;
}

VariableSymbolSharedPtr SemanticAnalyser::DoDec(KTreeNode *node)
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
        initial_value != nullptr,
        initial_value);
}

// Returns either an UNKNOWN VariableSymbol containing name
// or an ArraySymbol with name and element symbol
VariableSymbolSharedPtr SemanticAnalyser::DoVarDec(KTreeNode *node)
{
    // VarDec: ID
    if (node->l_child->value->is_token &&
        node->l_child->value->ast_node_value.token->type == TOKEN_ID)
    {
        return std::make_shared<VariableSymbol>(
            GetLineNumber(node->l_child),
            node->l_child->value->ast_node_value.token->value,
            VariableSymbolType::UNKNOWN);
    }

    // VarDec: VarDec L_SQUARE LITERAL_INT R_SQUARE
    auto array_element = DoVarDec(node->l_child);

    return std::make_shared<ArraySymbol>(
        array_element->LineNumber(),
        array_element->Name(),
        array_element,
        std::stoull(node->l_child->r_sibling->r_sibling->value->ast_node_value.token->value));
}

VariableSymbolSharedPtr DoExp(KTreeNode *node)
{
}