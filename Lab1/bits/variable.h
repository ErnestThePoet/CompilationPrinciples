#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

#define VARIABLE_NAME_MAX_LENGTH 50

typedef struct
{
    char name[VARIABLE_NAME_MAX_LENGTH + 1];
} Variable;

Variable *CreateVariable(const char *name);

void FreeVariable(Variable *variable);

#endif