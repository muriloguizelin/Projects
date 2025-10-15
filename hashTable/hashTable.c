#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

#define INITIAL_SIZE 10
#define LOAD_FACTOR_THRESHOLD 0.7

/*
Simple hash function (djb2 by Dan Bernstein)
Numbers 33 and 5381 are commonly used and work well to mix bits, providing a decent distribution.
Average-case O(1), worst-case O(n) per bucket when many keys collide.
*/

unsigned long hash(const char *str, size_t table_size) {
    unsigned long h = 5381;
    int c;
    while ((c = *str++))
        h = ((h << 5) + h) + c; // h * 33 + c 
    return h % table_size;
}

Entry* create_entry(const char *key, const char *value) {
    Entry *e = malloc(sizeof(Entry));
    if (!e) return NULL;
    e->key = strdup(key);
    e->value = strdup(value);
    e->next = NULL;
    return e;
}

HashTable* create_table() {
    HashTable *ht = malloc(sizeof(HashTable));
    if (!ht) return NULL;
    
    ht->size = INITIAL_SIZE;
    ht->count = 0;
    ht->table = calloc(INITIAL_SIZE, sizeof(Entry*));
    
    if (!ht->table) {
        free(ht);
        return NULL;
    }
    return ht;
}

static int resize_table(HashTable *ht) {
    size_t new_size = ht->size * 2;
    Entry **new_table = calloc(new_size, sizeof(Entry*));
    if (!new_table) return 0;

    for (size_t i = 0; i < ht->size; i++) {
        Entry *cur = ht->table[i];
        while (cur) {
            Entry *next = cur->next;
            unsigned long new_idx = hash(cur->key, new_size);
            
            cur->next = new_table[new_idx];
            new_table[new_idx] = cur;
            
            cur = next;
        }
    }

    free(ht->table);
    ht->table = new_table;
    ht->size = new_size;
    return 1;
}

void insert(HashTable *ht, const char *key, const char *value) {
    if ((float)ht->count / ht->size >= LOAD_FACTOR_THRESHOLD) {
        if (!resize_table(ht)) {
            fprintf(stderr, "Warning: Failed to resize hash table\n");
        }
    }

    unsigned long idx = hash(key, ht->size);
    Entry *head = ht->table[idx];

    for (Entry *cur = head; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            free(cur->value);
            cur->value = strdup(value);
            return;
        }
    }

    Entry *new_e = create_entry(key, value);
    if (!new_e) {
        fprintf(stderr, "Error: Failed to create new entry\n");
        return;
    }
    new_e->next = head;
    ht->table[idx] = new_e;
    ht->count++;
}

char* get(HashTable *ht, const char *key) {
    unsigned long idx = hash(key, ht->size);
    for (Entry *cur = ht->table[idx]; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            return cur->value;
        }
    }
    return NULL;
}

void free_table(HashTable *ht) {
    if (!ht) return;
    for (size_t i = 0; i < ht->size; i++) {
        Entry *cur = ht->table[i];
        while (cur) {
            Entry *next = cur->next;
            free(cur->key);
            free(cur->value);
            free(cur);
            cur = next;
        }
    }
    free(ht->table);
    free(ht);
}
