#include "variable.h"

Variable *VariableCreate(const int line_start,
                         const int column_start,
                         const char *name)
{
    Variable *variable = (Variable *)malloc(sizeof(Variable));
    if (variable == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    variable->line_start = line_start;
    variable->column_start = column_start;

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

void VariableFree(Variable *variable)
{
    if (variable != NULL)
    {
        free(variable);
    }
}