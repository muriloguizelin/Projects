#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

#define TABLE_SIZE 10

unsigned long hash(const char *str) {
    unsigned long h = 5381;
    int c;
    while ((c = *str++))
        h = ((h << 5) + h) + c;
    return h % TABLE_SIZE;
}

Entry* create_entry(const char *key, const char *value) {
    Entry *e = malloc(sizeof(Entry));
    e->key = strdup(key);
    e->value = strdup(value);
    e->next = NULL;
    return e;
}

HashTable* create_table() {
    HashTable *ht = malloc(sizeof(HashTable));
    ht->table = calloc(TABLE_SIZE, sizeof(Entry*));
    return ht;
}

void insert(HashTable *ht, const char *key, const char *value) {
    unsigned long idx = hash(key);
    Entry *head = ht->table[idx];

    for (Entry *cur = head; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            free(cur->value);
            cur->value = strdup(value);
            return;
        }
    }

    Entry *new_e = create_entry(key, value);
    new_e->next = head;
    ht->table[idx] = new_e;
}

char* get(HashTable *ht, const char *key) {
    unsigned long idx = hash(key);
    for (Entry *cur = ht->table[idx]; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            return cur->value;
        }
    }
    return NULL;
}

void free_table(HashTable *ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
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
