#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

#include "flasc.h"
#include <ctype.h>
#include <stdint.h>
#include <string.h>



void delete_linked_list(struct node *node);
int hash_key(const char* key, int cap);

int hashtable_set (struct hashtable *ht, int cap)
{
    if (!ht) return -1;
    if (cap == 0) return -1;
    ht->len = 0;
    ht->cap = cap;
    ht->array = calloc(cap, sizeof(struct node));
    if (!ht->array) return -1;
    return 0;
}

int hash_key(const char* key, int cap) {
    if (cap <= 0) return -1;
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    int index = hash % cap;
    return index;
}

int node_append(char *key, char *value, struct hashtable *ht)
{
    int index = hash_key(key, ht->cap);
    struct node *n, *ptr;

    n = malloc(sizeof(struct node));
    n->key = key;
    n->value = value;
    n->next = NULL;

    // empty slot in hash table
    if (ht->array[index] == NULL) {
        ht->array[index] = n;
        return 0;
    }
    // in case of collision
    for (ptr = ht->array[index]; ptr != NULL; ptr = ptr->next)
    {   
        if (ptr->next == NULL)
        {
            ptr->next = n;
            ht->len++;
            return 0;
        }
    }
    return -1;
}

char *node_search(char *key, struct hashtable *ht)
{
    if (!key) return NULL;
    if (!ht) return NULL;
    int index = hash_key(key, ht->cap);
    struct node *ptr;

    for (ptr = ht->array[index]; ptr != NULL; ptr = ptr->next)
    {   
        if (strcmp(ptr->key, key) == 0)
        {
            return ptr->value;
        }
    }
    // key not found
    return NULL;
}

void hashtable_delete(struct hashtable *ht)
{
    for (int i = 0; i < ht->cap; i++)
    {
        if (ht->array[i] != NULL)
        {
            delete_linked_list(ht->array[i]);
        }
    }
    free(ht->array);
}

void delete_linked_list(struct node *node)
{
    if (node == NULL)
    {
        return;
    }
    delete_linked_list(node->next);
    free(node);
}