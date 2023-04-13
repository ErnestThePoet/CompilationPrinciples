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
                return;
            }
            case TOKEN_LITERAL_INT:
            {
                return;
            }
            case TOKEN_LITERAL_FP:
            {
                return;
            }
            default:
                return;
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

            // Check args
            // Call with args
            auto args_node = node->l_child->r_sibling->r_sibling;
            if (!args_node->value->is_token)
            {
                DoArgs(args_node);

                return;
            }
            // Call with no args
            else
            {
                return;
            }
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: SUB Exp ///////////////////////////////////////////////////
        if (node->l_child->value->ast_node_value.token->type == TOKEN_OPERATOR_SUB)
        {
            DoExp(node->r_child);

            return;
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: NOT Exp ///////////////////////////////////////////////////
        if (node->l_child->value->ast_node_value.token->type == TOKEN_OPERATOR_LOGICAL_NOT)
        {
            DoExp(node->r_child);

            return;
        }
        ///////////////////////////////////////////////////////////////////

        // Exp: L_BRACKET Exp R_BRACKET ///////////////////////////////////
        // This keeps l/r-value type
        DoExp(node->l_child->r_sibling);
        return;
        ///////////////////////////////////////////////////////////////////
    }
    else
    {
        auto second_child = node->l_child->r_sibling;

        switch (second_child->value->ast_node_value.token->type)
        {
        case TOKEN_DELIMITER_L_SQUARE:
        {
            DoExp(node->l_child);

            DoExp(node->l_child);

            return;
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

void IrGenerator::DoArgs(const KTreeNode *node)
{
    // Args: Exp COMMA Args | Exp
    while (node != NULL)
    {
        DoExp(node->l_child);
        node = node->r_child;
    }
}