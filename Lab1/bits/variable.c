#include "variable.h"

Variable *CreateVariable(const char *name)
{
    Variable *variable = (Variable *)malloc(sizeof(Variable));
    if (variable == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    if (name != NULL)
    {
        size_t name_length = strlen(name);

        if (name_length > VARIABLE_NAME_MAX_LENGTH)
        {
            fputs("Variable name length too long: Line %d, Column %d\n", stderr);
            exit(FAILURE);
        }

        strcpy(variable->name, name);
    }

    return variable;
}

void FreeVariable(Variable *variable)
{
    if (variable != NULL)
    {
        free(variable);
    }
}