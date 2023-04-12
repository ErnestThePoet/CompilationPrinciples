#include "k_tree.h"

static KTreeNodeFreeValueAction KTreeFreeNodeValue = NULL;

static void KTreeFreeNode_(KTreeNode *node, size_t, void *)
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
        KTreeNode *current_child = va_arg(children, KTreeNode *);
        // Avoid epsilon patterns
        if (current_child != NULL)
        {
            KTreeAddChildRight(root, current_child);
        }
    }

    va_end(children);

    return root;
}

void KTreeFree(KTreeNode *root, KTreeNodeFreeValueAction action)
{
    KTreeFreeNodeValue = action;
    KTreePreOrderTraverse(root, KTreeFreeNode_, NULL);
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
        root->r_child = child;
    }
}

void KTreePreOrderTraverse(KTreeNode *root, KTreeNodeTraverseAction action, void *user_arg)
{
    Stack *stack = StackCreate();

    KTreeNode *current_node = root;

    size_t current_level = 0;

    while (!StackIsEmpty(stack) || current_node != NULL)
    {
        if (current_node != NULL)
        {
            action(current_node, current_level, user_arg);
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

    StackFree(stack);
}