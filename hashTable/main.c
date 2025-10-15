#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hashtable.h"

char* random_string(int length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char* str = malloc(length + 1);
    
    for (int i = 0; i < length; i++) {
        int index = rand() % (sizeof(charset) - 1);
        str[i] = charset[index];
    }
    str[length] = '\0';
    return str;
}

int main() {
    srand(time(NULL));
    
    HashTable *ht = create_table();
    if (!ht) {
        fprintf(stderr, "Failed to create hash table\n");
        return 1;
    }

    printf("=== Test 1: Basic insertion ===\n");
    insert(ht, "name", "Murilo");
    insert(ht, "city", "Cuiaba");
    insert(ht, "profession", "Developer");
    insert(ht, "language", "C");

    printf("name = %s\n", get(ht, "name"));
    printf("city = %s\n", get(ht, "city"));
    printf("profession = %s\n", get(ht, "profession"));
    printf("language = %s\n", get(ht, "language"));

    printf("\n=== Test 2: Updating values ===\n");
    insert(ht, "city", "Sao Paulo");
    insert(ht, "language", "C/C++");
    
    printf("city (updated) = %s\n", get(ht, "city"));
    printf("language (updated) = %s\n", get(ht, "language"));

    printf("\n=== Test 3: Resizing stress test ===\n");
    const int NUM_ENTRIES = 1000;
    char **keys = malloc(NUM_ENTRIES * sizeof(char*));
    char **values = malloc(NUM_ENTRIES * sizeof(char*));

    for (int i = 0; i < NUM_ENTRIES; i++) {
        keys[i] = random_string(10);  
        values[i] = random_string(15);
        insert(ht, keys[i], values[i]);
        
        if ((i + 1) % 100 == 0) {
            printf("Inserted %d entries...\n", i + 1);
        }
    }

    printf("\nVerifying a few random insertions:\n");
    for (int i = 0; i < 5; i++) {
        int idx = rand() % NUM_ENTRIES;
        const char* value = get(ht, keys[idx]);
        printf("Key: %s, Value: %s\n", keys[idx], value);
        if (strcmp(value, values[idx]) != 0) {
            printf("ERROR: Value does not match the inserted one!\n");
        }
    }

    printf("\n=== Test 4: Lookup of non-existent key ===\n");
    const char* not_found = get(ht, "non_existent_key");
    printf("Looking up non-existent key: %s\n", not_found ? not_found : "NULL (as expected)");

    for (int i = 0; i < NUM_ENTRIES; i++) {
        free(keys[i]);
        free(values[i]);
    }
    free(keys);
    free(values);
    free_table(ht);
    printf("\nTests completed successfully!\n");
    return 0;
}
