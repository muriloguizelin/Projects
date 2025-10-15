    #ifndef HASHTABLE_H
    #define HASHTABLE_H

    #include <stddef.h>

    typedef struct Entry {
        char *key;
        char *value;
        struct Entry *next;
    } Entry;

    typedef struct HashTable {
        Entry **table;    
        size_t size;      
        size_t count;     
    } HashTable;

    HashTable* create_table(void);
    void insert(HashTable *ht, const char *key, const char *value);
    char* get(HashTable *ht, const char *key);
    void free_table(HashTable *ht);

    #endif
