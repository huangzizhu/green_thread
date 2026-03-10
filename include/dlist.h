//
// Created by root on 25-9-26.
//

#ifndef DLIST_H
#define DLIST_H
#include <stddef.h>

#define get_data(ptr, type, member) ({              \
const typeof(((type *)0)->member) *__mptr = (ptr);  \
(type *)((char *)__mptr - offsetof(type, member));  \
})

#define node_init(node)  \
do { (node)->next = (node); (node)->prev = (node); } while (0)
typedef struct  dlist_node_t
{
    struct dlist_node_t* next;
    struct dlist_node_t* prev;
}dlist_node;



typedef struct dlist
{
    size_t size;
    dlist_node head;
    dlist_node* tail;
}dlist;
void dlist_init(dlist* list);
dlist* create_dlist();
int dlist_empty(dlist* list);
void destroy_dlist(dlist* list);
int insert_head(dlist* list,dlist_node* node);
int insert_tail(dlist* list,dlist_node* node);
dlist_node* remove_head(dlist* list);
dlist_node* remove_tail(dlist* list);
dlist_node* get_head(dlist* list);
dlist_node* get_tail(dlist* list);
size_t len(dlist* list);
int is_empty(dlist* list);
#endif //DLIST_H
