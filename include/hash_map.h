//
// Created by root on 2026/3/10.
//

#ifndef GREEN_THREAD_HASH_MAP_H
#define GREEN_THREAD_HASH_MAP_H
#include "slist.h"

typedef struct hash_map_
{
    size_t cap;
    size_t upbound;//触发扩容上限：0.75*cap
    size_t size;
    slist_t** buckets;
}hash_map_t;

typedef struct hash_map_pair
{
    void* key;
    void* data;
}hm_pair;

hash_map_t* hm_create();
void hm_insert(hash_map_t* hm,hm_pair* pair);
hm_pair* hm_remove(hash_map_t* hm,void* key);
hm_pair* hm_find_by_key(hash_map_t* hm,void* key);
void hm_destroy(hash_map_t* hm);
#endif //GREEN_THREAD_HASH_MAP_H