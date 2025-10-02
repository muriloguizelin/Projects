#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct Entry {
    char *key;
    char *value;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **table;
} HashTable;

HashTable* create_table();
void insert(HashTable *ht, const char *key, const char *value);
char* get(HashTable *ht, const char *key);
void free_table(HashTable *ht);

#endif
