#ifndef K_TREE_H_
#define K_TREE_H_

#include <stdlib.h>
#include <stdarg.h>
#include "defs.h"
#include "ast_node.h"

typedef AstNode *KTreeNodeValue;

typedef struct KTreeNode_
{
    KTreeNodeValue value;
    struct KTreeNode_ *l_child;
    struct KTreeNode_ *r_child;
    struct KTreeNode_ *r_sibling;
} KTreeNode;

typedef void (*KTreeNodeTraverseAction)(KTreeNode *, size_t, void *);
typedef void (*KTreeNodeFreeValueAction)(KTreeNodeValue *);

#include "stack.h"

static KTreeNodeFreeValueAction KTreeFreeNodeValue = NULL;
static void KTreeFreeNode_(KTreeNode *node, size_t, void *);

KTreeNode *KTreeCreateNode(KTreeNodeValue *value);
KTreeNode *KTreeCreateNodeWithChidren(KTreeNodeValue *value, int argc, ...);
void FreeKTree(KTreeNode *root, KTreeNodeFreeValueAction action);
void KTreeAddChildRight(KTreeNode *root, KTreeNode *child);
void KTreePreOrderTraverse(KTreeNode *root, KTreeNodeTraverseAction action, void *user_arg);

#endif