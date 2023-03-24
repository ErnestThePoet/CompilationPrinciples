#include "token.h"

Token *CreateToken(const int line_start,
                   const int column_start,
                   const int type,
                   const char *value)
{
    Token *token = malloc(sizeof(Token));
    if (token == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    token->line_start = line_start;
    token->column_start = column_start;
    token->type = type;

    if (value != NULL)
    {
        size_t value_length = strlen(value);

        if (value_length > TOKEN_MAX_VALUE_LENGTH)
        {
            fprintf(stderr,
                    "Token value length too long: Line %d, Column %d\n",
                    line_start,
                    column_start);
            exit(FAILURE);
        }

        strcpy(token->value, value);
    }

    return token;
}

void FreeToken(Token *token)
{
    if (token != NULL)
    {
        free(token);
    }
}

#define COPY_TOKEN_NAME_BREAK(NAME) \
    strcpy(buffer, NAME);           \
    break

void GetTokenName(char *buffer, const int type)
{
    switch (type)
    {
    case TOKEN_ID:
        COPY_TOKEN_NAME_BREAK("ID");

    case TOKEN_LITERAL_INT:
        COPY_TOKEN_NAME_BREAK("INT");
    case TOKEN_LITERAL_FP:
        COPY_TOKEN_NAME_BREAK("FLOAT");

    case TOKEN_KEYWORD_RETURN:
        COPY_TOKEN_NAME_BREAK("RETURN");
    case TOKEN_KEYWORD_IF:
        COPY_TOKEN_NAME_BREAK("IF");
    case TOKEN_KEYWORD_ELSE:
        COPY_TOKEN_NAME_BREAK("ELSE");
    case TOKEN_KEYWORD_WHILE:
        COPY_TOKEN_NAME_BREAK("WHILE");
    case TOKEN_KEYWORD_STRUCT:
        COPY_TOKEN_NAME_BREAK("STRUCT");
    case TOKEN_KEYWORD_TYPE_INT:
        COPY_TOKEN_NAME_BREAK("TYPE_INT");
    case TOKEN_KEYWORD_TYPE_FLOAT:
        COPY_TOKEN_NAME_BREAK("TYPE_FLOAT");

    case TOKEN_DELIMITER_L_BRACKET:
        COPY_TOKEN_NAME_BREAK("L_BRACKET");
    case TOKEN_DELIMITER_R_BRACKET:
        COPY_TOKEN_NAME_BREAK("R_BRACKET");
    case TOKEN_DELIMITER_L_BRACE:
        COPY_TOKEN_NAME_BREAK("L_BRACE");
    case TOKEN_DELIMITER_R_BRACE:
        COPY_TOKEN_NAME_BREAK("R_BRACE");
    case TOKEN_DELIMITER_L_SQUARE:
        COPY_TOKEN_NAME_BREAK("L_SQUARE");
    case TOKEN_DELIMITER_R_SQUARE:
        COPY_TOKEN_NAME_BREAK("R_SQUARE");
    case TOKEN_DELIMITER_SEMICOLON:
        COPY_TOKEN_NAME_BREAK("SEMICOLON");

    case TOKEN_OPERATOR_DOT:
        COPY_TOKEN_NAME_BREAK("DOT");
    case TOKEN_OPERATOR_COMMA:
        COPY_TOKEN_NAME_BREAK("COMMA");
    case TOKEN_OPERATOR_ASSIGN:
        COPY_TOKEN_NAME_BREAK("ASSIGN");

    case TOKEN_OPERATOR_LOGICAL_AND:
        COPY_TOKEN_NAME_BREAK("LOGICAL_AND");
    case TOKEN_OPERATOR_LOGICAL_OR:
        COPY_TOKEN_NAME_BREAK("LOGICAL_OR");
    case TOKEN_OPERATOR_LOGICAL_NOT:
        COPY_TOKEN_NAME_BREAK("LOGICAL_NOT");

    case TOKEN_OPERATOR_ADD:
        COPY_TOKEN_NAME_BREAK("ADD");
    case TOKEN_OPERATOR_SUB:
        COPY_TOKEN_NAME_BREAK("SUB");
    case TOKEN_OPERATOR_MUL:
        COPY_TOKEN_NAME_BREAK("MUL");
    case TOKEN_OPERATOR_DIV:
        COPY_TOKEN_NAME_BREAK("DIV");

    case TOKEN_OPERATOR_REL_EQ:
        COPY_TOKEN_NAME_BREAK("REL_EQ");
    case TOKEN_OPERATOR_REL_NE:
        COPY_TOKEN_NAME_BREAK("REL_NE");
    case TOKEN_OPERATOR_REL_GT:
        COPY_TOKEN_NAME_BREAK("REL_GT");
    case TOKEN_OPERATOR_REL_LT:
        COPY_TOKEN_NAME_BREAK("REL_LT:");
    case TOKEN_OPERATOR_REL_GE:
        COPY_TOKEN_NAME_BREAK("REL_GE");
    case TOKEN_OPERATOR_REL_LE:
        COPY_TOKEN_NAME_BREAK("REL_LE");

    case TOKEN_UNKNOWN:
    default:
        COPY_TOKEN_NAME_BREAK("<UNKNOWN>");
    }
}