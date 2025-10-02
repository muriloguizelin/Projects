#include <stdio.h>
#include "hashtable.h"

int main() {
    HashTable *ht = create_table();

    insert(ht, "name", "Murilo");
    insert(ht, "city", "Cuiaba");

    printf("name = %s\n", get(ht, "name"));
    printf("city = %s\n", get(ht, "city"));

    free_table(ht);
    return 0;
}
