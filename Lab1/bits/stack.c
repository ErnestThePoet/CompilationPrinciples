#include "stack.h"

Stack *CreateStack()
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    if (stack == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    stack->base_ = (StackElement *)malloc(kStackInitialCapacity * sizeof(StackElement));
    if (stack->base_ == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    stack->capacity_ = kStackInitialCapacity;
    stack->size = 0;

    return stack;
}

void FreeStack(Stack *stack)
{
    if (stack == NULL)
    {
        return;
    }

    if (stack->base_ != NULL)
    {
        free(stack->base_);
    }

    free(stack);
}

bool IsEmpty(const Stack *stack)
{
    return stack->size == 0;
}

void Push(Stack *stack, StackElement *element)
{
    if (stack->size + 1 > stack->capacity_)
    {
        stack->capacity_ *= 2;
        stack->base_ = realloc(stack->base_, stack->capacity_);
        if (stack->base_ == NULL)
        {
            MEMORY_ALLOC_FAILURE_EXIT;
        }
    }

    stack->base_[stack->size] = *element;
    stack->size++;
}

StackElement Pop(Stack *stack)
{
    if (stack->size == 0)
    {
        fputs("Call Pop() on an empty stack\n", stderr);
        exit(FAILURE);
    }

    StackElement top_element = stack->base_[stack->size - 1];

    if (stack->size - 1 >= kStackInitialCapacity && stack->size - 1 <= stack->capacity_ / 2)
    {
        stack->capacity_ /= 2;
        stack->base_ = realloc(stack->base_, stack->capacity_);
        if (stack->base_ == NULL)
        {
            MEMORY_ALLOC_FAILURE_EXIT;
        }
    }

    stack->size--;

    return top_element;
}
