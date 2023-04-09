#include "semantic_analyser.h"

void SemanticAnalyser::PrintError(
    const int type, const KTreeNode *node, const std::string &message) const
{
    int line_number = node->value->is_token
                          ? node->value->ast_node_value.token->line_start
                          : node->value->ast_node_value.variable->line_start;
    std::cerr << "Error type " << type << " at Line " << line_number << ": " << message << std::endl;
}

void SemanticAnalyser::Build(KTreeNode *node, size_t, void *)
{
    if (!node->value->is_token && node->value->ast_node_value.variable->type == VARIABLE_EXT_DEF_LIST)
    {
        DoExtDefList(node);
    }
}

void SemanticAnalyser::DoExtDefList(KTreeNode *node)
{
    while (node != NULL)
    {
        DoExtDef(node->l_child);

        node = node->l_child->r_sibling;
    }
}

void SemanticAnalyser::DoExtDef(KTreeNode *node)
{
}

// Return value is used to provide type info
SymbolSharedPtr SemanticAnalyser::DoSpecifier(KTreeNode *node)
{
    if (node->l_child->value->is_token)
    {
        return std::make_shared<ArithmeticSymbol>(
            "",
            node->l_child->value->ast_node_value.token->type == TOKEN_KEYWORD_TYPE_INT
                ? ArithmeticSymbolType::INT
                : ArithmeticSymbolType::FLOAT);
    }
    else
    {
        return DoStructSpecifier(node->l_child);
    }
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

// For named struct def, check the existence of the def,
// add the def to symtable and return a StructSymbol.
// For unnamed struct def, create a new random name for it,
// add the def to symtable and return a StructSymbol.
// For named struct, check the existence of the def and
// return a StructSymbol.
SymbolSharedPtr SemanticAnalyser::DoStructSpecifier(KTreeNode *node)
{
    std::string struct_name;

    if (!node->l_child->r_sibling->value->is_token && node->l_child->r_sibling->value->ast_node_value.variable->type == VARIABLE_TAG)
    {
        struct_name =
            node->l_child->r_sibling->l_child->value->ast_node_value.token->value;
        if (symbol_table_.contains(struct_name) && symbol_table_[struct_name]->Type() == SymbolType::STRUCT_DEF)
        {
            return std::make_shared<StructSymbol>("", struct_name);
        }

        PrintError(kErrorUndefinedStruct, node, "Struct '" + struct_name + "' is not defined");
        return nullptr;
    }
    else
    {
        if (node->l_child->r_sibling->value->is_token)
        {
            struct_name = GetNewAnnoyStructName();
        }
        else
        {
            struct_name = node->l_child->r_sibling->l_child->value->ast_node_value.token->value;
        }
    }
}

SymbolSharedPtr SemanticAnalyser::DoVarDec(KTreeNode *node)
{
    if (node->l_child->value->is_token && node->l_child->value->ast_node_value.token->type == TOKEN_ID)
    {
        return std::make_shared<Symbol>(
            node->l_child->value->ast_node_value.token->value,
            SymbolType::UNKNOWN);
    }

    auto array_element = DoVarDec(node->l_child);

    return std::make_shared<ArraySymbol>(
        array_element->Name(),
        array_element,
        std::stoull(node->l_child->r_sibling->r_sibling->value->ast_node_value.token->value));
}