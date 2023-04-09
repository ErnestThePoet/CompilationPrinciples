#include "symbol_table_builder.h"

void SymbolTableBuilder::Build(KTreeNode *node, size_t, void *)
{
    if (!node->value->is_token && node->value->ast_node_value.variable->type == VARIABLE_EXT_DEF_LIST)
    {
        DoExtDefList(node);
    }
}

void SymbolTableBuilder::DoExtDefList(KTreeNode *node)
{
    while (node != NULL)
    {
        auto symbol = DoExtDef(node->l_child);

        if (symbol_table_->contains(symbol->Name()))
        {
            std::cerr << "Symbol '" << symbol->Name() << "' already exists.\n";
            exit(FAILURE);
        }

        (*symbol_table_)[symbol->Name()] = symbol;

        node = node->l_child->r_sibling;
    }
}

SymbolSharedPtr DoExtDef(KTreeNode *node)
{
}

// Return value is used to provide type info
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
    else
    {
        return DoStructSpecifier(node->l_child);
    }
}

// For named struct def, check the existence of the def,
// add the def to symtable and return a StructSymbol.
// For unnamed struct def, create a new random name for it, 
// add the def to symtable and return a StructSymbol.
// For named struct, check the existence of the def and 
// return a StructSymbol.
SymbolSharedPtr DoStructSpecifier(KTreeNode *node)
{
    if(!node->l_child->r_sibling->value->is_token
    &&node->l_child->r_sibling->value->ast_node_value.variable->type==VARIABLE_TAG){
        
    }
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