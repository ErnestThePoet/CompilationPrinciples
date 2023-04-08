#include "variable.h"

Variable *VariableCreate(const int line_start,
                         const int column_start,
                         const int type)
{
    Variable *variable = (Variable *)malloc(sizeof(Variable));
    if (variable == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    variable->line_start = line_start;
    variable->column_start = column_start;
    variable->type = type;

    return variable;
}

void VariableFree(Variable *variable)
{
    if (variable != NULL)
    {
        free(variable);
    }
}

#define COPY_VARIABLE_NAME_BREAK(NAME) \
    strcpy(buffer, NAME);              \
    break

void GetVariableName(char *buffer, const int type)
{
    switch (type)
    {
    case VARIABLE_PROGRAM:
        COPY_VARIABLE_NAME_BREAK("Program");
    case VARIABLE_EXT_DEF_LIST:
        COPY_VARIABLE_NAME_BREAK("ExtDefList");
    case VARIABLE_EXT_DEF:
        COPY_VARIABLE_NAME_BREAK("ExtDef");
    case VARIABLE_EXT_DEC_LIST:
        COPY_VARIABLE_NAME_BREAK("ExtDecList");
    case VARIABLE_SPECIFIER:
        COPY_VARIABLE_NAME_BREAK("Specifier");
    case VARIABLE_STRUCT_SPECIFIER:
        COPY_VARIABLE_NAME_BREAK("StructSpecifier");
    case VARIABLE_OPT_TAG:
        COPY_VARIABLE_NAME_BREAK("OptTag");
    case VARIABLE_TAG:
        COPY_VARIABLE_NAME_BREAK("Tag");
    case VARIABLE_VAR_DEC:
        COPY_VARIABLE_NAME_BREAK("VarDec");
    case VARIABLE_FUN_DEC:
        COPY_VARIABLE_NAME_BREAK("FunDec");
    case VARIABLE_VAR_LIST:
        COPY_VARIABLE_NAME_BREAK("VarList");
    case VARIABLE_PARAM_DEC:
        COPY_VARIABLE_NAME_BREAK("ParamDec");
    case VARIABLE_COMP_ST:
        COPY_VARIABLE_NAME_BREAK("CompSt");
    case VARIABLE_STMT_LIST:
        COPY_VARIABLE_NAME_BREAK("StmtList");
    case VARIABLE_STMT:
        COPY_VARIABLE_NAME_BREAK("Stmt");
    case VARIABLE_DEF_LIST:
        COPY_VARIABLE_NAME_BREAK("DefList");
    case VARIABLE_DEF:
        COPY_VARIABLE_NAME_BREAK("Def");
    case VARIABLE_DEC_LIST:
        COPY_VARIABLE_NAME_BREAK("DecList");
    case VARIABLE_DEC:
        COPY_VARIABLE_NAME_BREAK("Dec");
    case VARIABLE_EXP:
        COPY_VARIABLE_NAME_BREAK("Exp");
    case VARIABLE_UNKNOWN:
    default:
        COPY_VARIABLE_NAME_BREAK("<Unknown>");
    }
}