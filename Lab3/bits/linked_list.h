#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdlib.h>
#include <stdarg.h>
#include "defs.h"

typedef int LinkedListNodeValue;

typedef struct LinkedListNode_{
    LinkedListNodeValue value;
    struct LinkedListNode_ *prev;
    struct LinkedListNode_ *next;
} LinkedListNode;

LinkedListNode *LinkedListCreateNode(LinkedListNodeValue *value);
void LinkedListAddNode(LinkedListNode *before, LinkedListNode *node);
void LinkedListFree(LinkedListNode *start);

#endif
