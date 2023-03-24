#ifndef K_TREE_H_
#define K_TREE_H_

#include <stdbool.h>
#include <stdlib.h>
#include "token.h"

typedef Token KTreeNodeValue;

typedef struct KTreeNode_
{
    KTreeNodeValue value;
    struct KTreeNode_ *l_child;
    struct KTreeNode_ *r_sibling;
} KTreeNode;

KTreeNode *CreateKTree();
bool AddChild(KTreeNode *node);


#endif