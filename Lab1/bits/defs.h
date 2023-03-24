#ifndef DEFS_H_
#define DEFS_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FAILURE 1

#define MEMORY_ALLOC_FAILURE_EXIT                    \
    do                                               \
    {                                                \
        fputs("Memory allocation failed\n", stderr); \
        exit(FAILURE);                               \
    } while (false)

#endif