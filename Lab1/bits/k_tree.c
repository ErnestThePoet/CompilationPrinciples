#include "k_tree.h"

static void FreeKTreeNode(KTreeNode *node, size_t)
{
    FreeKTreeNodeValue(&node->value);
}

KTreeNode *CreateKTree(KTreeNodeValue *value)
{
    KTreeNode *root = (KTreeNode *)malloc(sizeof(KTreeNode));
    if (root == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    root->l_child = NULL;
    root->r_child = NULL;
    root->r_sibling = NULL;
    root->value = *value;

    return root;
}

void FreeKTree(KTreeNode *root, KTreeNodeFreeValueAction action)
{
    FreeKTreeNodeValue = action;
    PreOrderTraverse(root, FreeKTreeNode);
}

void AddChildRight(KTreeNode *root, KTreeNode *child)
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

void PreOrderTraverse(KTreeNode *root, KTreeNodeTraverseAction action)
{
    Stack *stack = CreateStack();

    KTreeNode *current_node = root;

    size_t current_level = 0;

    while (!IsEmpty(stack) || current_node != NULL)
    {
        if (current_node != NULL)
        {
            action(current_node, current_level);
            Push(stack, &current_node);
            current_node = current_node->l_child;
            current_level++;
        }
        else
        {
            current_node = Pop(stack)->r_sibling;
            current_level--;
        }
    }
}