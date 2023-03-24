#ifndef K_TREE_H_
#define K_TREE_H_

#include <stdlib.h>
#include "defs.h"
#include "ast_node.h"
#include "stack.h"

typedef AstNode *KTreeNodeValue;
typedef void (*KTreeNodeTraverseAction)(KTreeNode *, size_t);
typedef void (*KTreeNodeFreeValueAction)(KTreeNodeValue *);

typedef struct KTreeNode_
{
    KTreeNodeValue value;
    struct KTreeNode_ *l_child;
    struct KTreeNode_ *r_child;
    struct KTreeNode_ *r_sibling;
} KTreeNode;

static KTreeNodeFreeValueAction FreeKTreeNodeValue = NULL;
static void FreeKTreeNode(KTreeNode *node, size_t);

KTreeNode *CreateKTree(KTreeNodeValue *value);
void FreeKTree(KTreeNode *root, KTreeNodeFreeValueAction action);
void AddChildRight(KTreeNode *root, KTreeNode *child);
void PreOrderTraverse(KTreeNode *root, KTreeNodeTraverseAction action);

#endif