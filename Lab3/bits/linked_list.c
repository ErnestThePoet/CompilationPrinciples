#include "linked_list.h"

LinkedListNode *LinkedListCreateNode(LinkedListNodeValue *value)
{
    LinkedListNode *node = (LinkedListNode *)malloc(sizeof(LinkedListNode));
    if (node == NULL)
    {
        MEMORY_ALLOC_FAILURE_EXIT;
    }

    node->value = *value;
    node->prev = NULL;
    node->next = NULL;

    return node;
}

void LinkedListAddNode(LinkedListNode *before,LinkedListNode* node){
    LinkedListNode *node_next = before->next;
    before->next = node;
    node->prev = before;
    node->next = node_next;
    node_next->prev = node;
}

void LinkedListFree(LinkedListNode *start){
    LinkedListNode *current_node = start;
    while (current_node!=NULL){
        LinkedListNode *next_node = current_node->next;
        free(current_node);
        current_node = next_node;
    }
}