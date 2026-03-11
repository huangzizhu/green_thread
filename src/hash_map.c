//
// Created by root on 2026/3/10.
//

#include "../include/hash_map.h"

#include <stdint.h>
#include <stdlib.h>

#include "../include/slist.h"

#define MAX_BUCKET_COUNT 65536
#define D
typedef struct hash_map_
{
    size_t cap;
    size_t size;
    slist_t** buckets;
}hash_map_t;

static inline size_t hashf(void* data)
{
    return  (uintptr_t)data ^ ((uintptr_t)data >> 32);
}

hash_map_t* hash_map_create()
{
    hash_map_t* hm = malloc(sizeof(hash_map_t));
    if (!hm)
        return NULL;
    hm->cap = 16;
    hm->size = 0;
    hm->buckets = calloc(hm->cap, sizeof(slist_t*));
    return hm;
}

void hash_map_destroy(hash_map_t* hm)
{
    if (!hm)
        return;
    for (size_t i = 0; i < hm->cap; ++i)
        slist_destroy(hm->buckets[i]);
    free(hm->buckets);
    free(hm);
}

void* hash_map_enlarge(hash_map_t* hm)
{
    
}