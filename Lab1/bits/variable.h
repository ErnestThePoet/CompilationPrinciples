#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

#define VARIABLE_NAME_BUFFER_SIZE 30

// Variable types
#define VARIABLE_UNKNOWN -1
#define VARIABLE_PROGRAM 0
#define VARIABLE_EXT_DEF_LIST 1
#define VARIABLE_EXT_DEF 2
#define VARIABLE_EXT_DEC_LIST 3
#define VARIABLE_SPECIFIER 4
#define VARIABLE_STRUCT_SPECIFIER 5
#define VARIABLE_OPT_TAG 6
#define VARIABLE_TAG 7
#define VARIABLE_VAR_DEC 8
#define VARIABLE_FUN_DEC 9
#define VARIABLE_VAR_LIST 10
#define VARIABLE_PARAM_DEC 11
#define VARIABLE_COMP_ST 12
#define VARIABLE_STMT_LIST 13
#define VARIABLE_STMT 14
#define VARIABLE_DEF_LIST 15
#define VARIABLE_DEF 16
#define VARIABLE_DEC_LIST 17
#define VARIABLE_DEC 18
#define VARIABLE_EXP 19
#define VARIABLE_ARGS 20

typedef struct
{
    int line_start;
    int column_start;
    int type;
} Variable;

Variable *VariableCreate(const int line_start,
                         const int column_start,
                         const int type);

void VariableFree(Variable *variable);

void GetVariableName(char *buffer, const int type);

#endif