//
// Created by root on 2026/3/10.
//

#ifndef GREEN_THREAD_SLLIST_H
#define GREEN_THREAD_SLLIST_H
#include <stddef.h>

typedef struct slist_node_
{
    void* data;
    struct slist_node_* next;
}slist_node;

typedef struct slist_
{
    size_t size;
    slist_node* head;
}slist_t;

slist_t* slist_create();
void slist_destroy(slist_t* list);
void slist_insert_head(slist_t* slist, void* data);
void* slist_remove_head(slist_t* slist);
extern inline int slist_is_empty(slist_t* slist);

#endif //GREEN_THREAD_SLLIST_H