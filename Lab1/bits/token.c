#include "token.h"

Token *CreateToken(const size_t line_start,
                   const size_t column_start,
                   const int type,
                   const char *name,
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

    if (name != NULL)
    {
        size_t name_length = strlen(name);

        if (name_length > TOKEN_MAX_NAME_LENGTH)
        {
            fprintf(stderr,
                    "Token name length too long: Line %d, Column %d\n",
                    line_start,
                    column_start);
            exit(FAILURE);
        }

        strcpy(token->name, name);
    }

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