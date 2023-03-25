#ifndef K_TREE_H_
#define K_TREE_H_

#include <stdlib.h>
#include <stdarg.h>
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

static KTreeNodeFreeValueAction KTreeFreeNodeValue = NULL;
static void KTreeFreeNode_(KTreeNode *node, size_t);

KTreeNode *KTreeCreateNode(KTreeNodeValue *value);
KTreeNode *KTreeCreateNodeWithChidren(KTreeNodeValue *value, int argc, ...);
void FreeKTree(KTreeNode *root, KTreeNodeFreeValueAction action);
void KTreeAddChildRight(KTreeNode *root, KTreeNode *child);
void KTreePreOrderTraverse(KTreeNode *root, KTreeNodeTraverseAction action);

#endif