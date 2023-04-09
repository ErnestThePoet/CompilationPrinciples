#include "symbol_table_builder.h"

void SymbolTableBuilder::Build(KTreeNode *node, size_t, void *user_arg)
{
    if (!node->value->is_token && node->value->ast_node_value.variable->type == VARIABLE_EXT_DEF_LIST)
    {
        DoExtDefList(node, *(SymbolTable *)user_arg);
    }
}

void SymbolTableBuilder::DoExtDefList(KTreeNode *node, SymbolTable &symbol_table)
{
    while (node != NULL)
    {
        auto symbol = DoExtDef(node->l_child);

        if (symbol_table.contains(symbol->Name()))
        {
            std::cerr << "Symbol '" << symbol->Name() << "' already exists.\n";
            exit(FAILURE);
        }

        symbol_table[symbol->Name()] = symbol;

        node = node->l_child->r_sibling;
    }
}

SymbolSharedPtr DoExtDef(KTreeNode *node)
{
}

// Return value contains type info
SymbolSharedPtr DoSpecifier(KTreeNode *node)
{
    if (node->l_child->value->is_token)
    {
        return std::make_shared<ArithmeticSymbol>(
            "",
            node->l_child->value->ast_node_value.token->type == TOKEN_KEYWORD_TYPE_INT
                ? ArithmeticSymbolType::INT
                : ArithmeticSymbolType::FLOAT);
    }


}

SymbolSharedPtr DoStructSpecifier(KTreeNode* node){
    
}

SymbolSharedPtr SymbolTableBuilder::DoVarDec(KTreeNode *node)
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