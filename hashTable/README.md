# Hash Table (C)

A minimal string-to-string hash table implemented in C using:
- djb2 hash function
- Separate chaining with singly linked lists
- Automatic resize at load factor ≥ 0.7

## Files
- `hashTable.h` — public API
- `hashTable.c` — implementation
- `main.c` — quick tests and usage examples

## Build and run (Windows PowerShell)

Requires GCC (MinGW) or Clang. Example with GCC:

```powershell
gcc -std=c11 -Wall -Wextra -O2 .\hashTable.c .\main.c -o .\hashTable.exe
.\hashTable.exe
```

## API

```c
HashTable* create_table(void);
void       insert(HashTable *ht, const char *key, const char *value);
char*      get(HashTable *ht, const char *key);
void       free_table(HashTable *ht);
```

## Notes
- Keys/values are copied on insert and freed by `free_table`.
- Average-case O(1) for insert/get; worst-case O(n) if many collisions.