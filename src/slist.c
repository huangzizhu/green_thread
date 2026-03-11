//
// Created by root on 2026/3/10.
//

#include "../include/slist.h"
#include <stdlib.h>



slist_node* slist_node_create(void* data)
{
    slist_node* node = (slist_node*)malloc(sizeof(slist_node));
    if (node == NULL)
    {
        return NULL;
    }
    node->data = data;
    node->next = NULL;
    return node;
}

slist_t* slist_create()
{
    slist_t* list = (slist_t*)malloc(sizeof(slist_t));
    if (list == NULL)
    {
        return NULL;
    }
    list->size = 0;
    list->head = NULL;
    return list;
}

void slist_destroy(slist_t* list)
{
    if (list == NULL)
    {
        return;
    }
    slist_node* current = list->head;
    while (current != NULL)
    {
        slist_node* temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
}

void insert_head(slist_t* slist, void* data)
{
    if (!slist) return;
    slist_node* node = slist_node_create(data);
    if (!node) return;
    node->next = slist->head;
    slist->head = node;
    slist->size++;
}

void* remove_head(slist_t* slist)
{
    if (!slist) return NULL;
    if (!slist->size) return NULL;
    slist_node* node = slist->head;
    slist->head = node->next;
    slist->size--;
    void* data = node->data;
    free(node);
    return data;
}

