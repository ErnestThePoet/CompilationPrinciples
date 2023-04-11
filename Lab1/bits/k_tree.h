#ifndef K_TREE_H_
#define K_TREE_H_

#include <stdlib.h>
#include <stdarg.h>
#include "defs.h"
#include "ast_node.h"

typedef AstNode *KTreeNodeValue;

// When a node has no child, both l_child and r_child will be NULL.
// When a node has only one child, l_child will point to the child and r_child will be NULL.
// When a node has more than one child, both l_child and r_child are not NULL.
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

KTreeNode *KTreeCreateNode(KTreeNodeValue *value);
KTreeNode *KTreeCreateNodeWithChidren(KTreeNodeValue *value, int argc, ...);
void FreeKTree(KTreeNode *root, KTreeNodeFreeValueAction action);
void KTreeAddChildRight(KTreeNode *root, KTreeNode *child);
void KTreePreOrderTraverse(KTreeNode *root, KTreeNodeTraverseAction action, void *user_arg);

#endif