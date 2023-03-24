#ifndef STACK_H_
#define STACK_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "k_tree.h"

const size_t kStackInitialCapacity = 50;

typedef KTreeNode *StackElement;

typedef struct
{
    size_t size;
    size_t capacity_;
    StackElement *base_;
} Stack;

Stack *CreateStack();
void FreeStack(Stack *stack);
bool IsEmpty(const Stack *stack);
void Push(Stack *stack, StackElement *element);
StackElement Pop(Stack *stack);

#endif