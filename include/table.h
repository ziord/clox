#ifndef TABLE_H
#define TABLE_H
#include "object.h"

#define TLOAD_FACTOR  0.75
typedef struct {
	ObjString* key;
	Value value;
}Entry;

typedef struct {
	int count;
	int capacity;
	Entry* entries;
}Table;

void initTable(Table* table);
void freeTable(Table* table);
Entry* findEntry(Entry* entries, int cap, ObjString* key);
static void adjustCapacity(Table* table, int capacity);
bool tableSet(Table* table, ObjString* key, Value value);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableDelete(Table* table, ObjString* key);
bool tableAddAll(Table* from, Table* to);
uint32_t hashFunc(char* chr, int length);
ObjString* tableFindString(Table* table, char* chars, int length, uint32_t hash);

#endif
