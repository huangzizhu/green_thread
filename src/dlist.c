//
// Created by root on 25-9-26.
//

#include "../include/dlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define offsetof(TYPE, MEMBER)  __builtin_offsetof(TYPE, MEMBER)


void dlist_init(dlist* list)
{
    if (!list) return;
    list->size = 0;
    list->head.next = &list->head;
    list->head.prev = &list->head;
}

int dlist_empty(dlist* list)
{
    if (!list) return 1;
    return list->size == 0;
}

dlist* create_dlist()
{
    dlist* list = calloc(1,sizeof(dlist));
    if (!list)
    {
        perror("calloc");
        return NULL;
    }
    dlist_init(list);
    return list;
}

void destroy_dlist(dlist* list)
{
    for (dlist_node* node = list->head.next, *temp = node->next; node != &list->head ; node = temp,temp = temp->next )
    {
        node->prev = node;
        node->next = node;
    }
    free(list);
}

int insert_head(dlist* list,dlist_node* node)
{
    if (!list || !node) return -2;
    node->next = list->head.next;
    node->prev = &list->head;
    list->head.next->prev = node;
    list->head.next = node;
    list->size++;
    if (list->tail == NULL) list->tail = node;
    return 0;
}

int insert_tail(dlist* list,dlist_node* node)
{
    if (!list || !node) return -2;
    if (list->tail == NULL) return insert_head(list,node);
    list->tail->next = node;
    node->prev = list->tail;
    node->next = &list->head;
    list->head.prev = node;
    list->tail = node;
    list->size++;
    return 0;
}

dlist_node* remove_head(dlist* list)
{
    if (!list) return NULL;
    if (list->size == 0) return NULL;
    dlist_node* node = list->head.next;
    list->head.next = node->next;
    list->head.next->prev = &list->head;
    node_init(node);
    list->size--;
    if (list->size == 0) list->tail = NULL;
    return node;
}

dlist_node* remove_tail(dlist* list)
{
    if (!list) return NULL;
    if (list->size == 0) return NULL;
    if (list->size == 1) return remove_head(list);
    dlist_node* node = list->tail;
    list->tail = node->prev;
    list->tail->next = &list->head;
    list->head.prev = list->tail;
    node_init(node);
    list->size--;
    return node;
}

dlist_node* get_head(dlist* list)
{
    if (!list || list->size == 0) return NULL;
    return list->head.next;
}

dlist_node* get_tail(dlist* list)
{
    if (!list) return NULL;
    return list->tail;
}

size_t len(dlist* list)
{
    if (!list) return 0;
    return list->size;
}
