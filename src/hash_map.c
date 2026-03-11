//
// Created by root on 2026/3/10.
//

#include "../include/hash_map.h"
#include "../include/slist.h"
#include <stdint.h>
#include <stdlib.h>



#define MAX_BUCKET_COUNT 65536
#define MIN_BUCKET_COUNT 16
#if defined(__x86_64__)
#define SYS_BIT 64
#define HALF_SYS_BIT 32
#else
#define SYS_BIT 32
#define HALF_SYS_BIT 16
#endif


extern inline size_t hashf(void* key,size_t cap)
{
    return  ((uintptr_t)key ^ ((uintptr_t)key >> HALF_SYS_BIT)) & (cap-1);
}

hash_map_t* hm_create()
{
    hash_map_t* hm = malloc(sizeof(hash_map_t));
    if (!hm)
        return NULL;
    hm->cap = 16;
    hm->size = 0;
    hm->upbound = (size_t)hm->cap * 0.75;
    hm->buckets = calloc(hm->cap, sizeof(slist_t*));
    return hm;
}

void hm_insert_(slist_t** new_buckets,slist_node* node,size_t new_cap)
{
    size_t bucket_idx = hashf(((hm_pair*)node->data)->key,new_cap);
    slist_t* bucket = new_buckets[bucket_idx];
    if (!bucket) bucket = slist_create();
    node->next = bucket->head;
    bucket->head = node;
}

void* hm_enlarge(hash_map_t* hm)
{
    //创建一个双倍的hashmap
    slist_t** new_buckets = calloc(hm->cap << 1, sizeof(slist_t*));
    if (!new_buckets) return NULL;
    hm->cap << 1;
    for (int i = 0 ; i < hm->cap ; ++i)
    {
        slist_t* list = hm->buckets[i];
        if (list)
        {
            for (slist_node* node = list->head ; node ; )
            {
                slist_node* temp = node;
                hm_insert_(new_buckets,temp,hm->cap);
                node = node->next;
            }
        }
        slist_destroy(list);
    }
    free(hm->buckets);
    hm->buckets = new_buckets;
    hm->upbound = (size_t)hm->cap * 0.75;
}



void hm_insert(hash_map_t* hm,hm_pair* pair)
{
    if (!hm || !pair) return;
    hm->size++;
    if (hm->size >= hm->upbound) hm_enlarge(hm);
    size_t bucket_idx = hashf(pair->key,hm->cap);
    slist_t* bucket = hm->buckets[bucket_idx];
    if (!bucket) bucket = slist_create();
    slist_insert_head(bucket,pair);
}

//中间函数，返回找到的节点的前一个节点，如果是头结点，返回链表指针
slist_node* find_by_key_(hash_map_t* hm,void* key)
{
    size_t bucket_idx = hashf(key,hm->cap);
    slist_t* bucket = hm->buckets[bucket_idx];
    if (!bucket) return NULL;
    if (((hm_pair*)bucket->head->data)->key == key) return (slist_node*)bucket;
    /*
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
    *
    *内存布局一样的，只要访问第二个成员，就可以获得
    */
    for (slist_node* now = bucket->head ; now ; )
    {
        slist_node* prev = now->next;
        now = now->next;
        if (((hm_pair*)now)->key == key) return (slist_node*)prev;
    }
    return NULL;
}

hm_pair* hm_find_by_key(hash_map_t* hm,void* key)
{
    if (!hm || !key) return NULL;
    slist_node* prev = find_by_key_(hm,key);
    if (!prev) return NULL;
    return prev->next->data;
}

hm_pair* hm_remove(hash_map_t* hm,void* key)
{
    if (!hm || !key) return NULL;
    slist_node* prev = find_by_key_(hm,key);
    if (!prev) return NULL;
    slist_node* node = prev->next;
    prev->next = node->next;
    hm_pair* res = node->data;
    free(node);
    return res;
}

void hm_destroy(hash_map_t* hm)
{
    if (!hm)
        return;
    for (size_t i = 0; i < hm->cap; ++i)
        slist_destroy(hm->buckets[i]);
    free(hm->buckets);
    free(hm);
}