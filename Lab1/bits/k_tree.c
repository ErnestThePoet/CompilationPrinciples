#include "k_tree.h"

static void KTreeFreeNode_(KTreeNode *node, size_t)
{
    KTreeFreeNodeValue(&node->value);
}

KTreeNode *KTreeCreateNode(KTreeNodeValue *value)
{
    KTreeNode *node = (KTreeNode *)malloc(sizeof(KTreeNode));
    if (node == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    node->l_child = NULL;
    node->r_child = NULL;
    node->r_sibling = NULL;
    node->value = *value;

    return node;
}

KTreeNode *KTreeCreateNodeWithChidren(KTreeNodeValue *value, int argc, ...)
{
    KTreeNode *root = KTreeCreateNode(value);

    va_list children;
    va_start(children, argc);

    for (int i = 0; i < argc; i++)
    {
        KTreeAddChildRight(root, va_arg(children, KTreeNode *));
    }

    va_end(children);

    return root;
}

void FreeKTree(KTreeNode *root, KTreeNodeFreeValueAction action)
{
    KTreeFreeNodeValue = action;
    KTreePreOrderTraverse(root, KTreeFreeNode_);
}

void KTreeAddChildRight(KTreeNode *root, KTreeNode *child)
{
    // Empty root
    if (root->l_child == NULL)
    {
        root->l_child = child;
    }
    // Root has only one child(on the left)
    else if (root->r_child == NULL)
    {
        root->r_child = child;
        root->l_child->r_sibling = child;
    }
    else
    {
        root->r_child->r_sibling = child;
    }
}

void KTreePreOrderTraverse(KTreeNode *root, KTreeNodeTraverseAction action)
{
    Stack *stack = StackCreate();

    KTreeNode *current_node = root;

    size_t current_level = 0;

    while (!StackIsEmpty(stack) || current_node != NULL)
    {
        if (current_node != NULL)
        {
            action(current_node, current_level);
            StackPush(stack, &current_node);
            current_node = current_node->l_child;
            current_level++;
        }
        else
        {
            current_node = StackPop(stack)->r_sibling;
            current_level--;
        }
    }
}