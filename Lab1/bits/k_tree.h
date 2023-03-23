#ifndef K_TREE_H_
#define K_TREE_H_

#include <stdbool.h>
#include <stdlib.h>
#include "token.h"

typedef Token KTreeNodeValue;

typedef struct
{
    KTreeNodeValue value;
    KTreeNode *l_child = NULL;
    KTreeNode *r_sibling = NULL;
} KTreeNode;

KTreeNode *CreateKTree();
bool AddChild(KTreeNode *node);


#endif