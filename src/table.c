#include "../include/table.h"
#include "../include/memory.h"
#include <string.h>

void initTable(Table* table) {
	table->capacity = 0;
	table->count = 0;
	table->entries = NULL;
}

void freeTable(Table* table) {
	FREE_ARRAY(Entry, table->entries, table->capacity);
	initTable(table);
}

uint32_t hashFunc(char* chr, int length) {
	uint32_t hash = 2166136261;
	uint32_t fnv_prime = 16777619;
	for (int i = 0; i < length; i++) {
		hash = hash ^ chr[i];
		hash = hash * fnv_prime;
	}
	return hash;
}
Entry* findEntry(Entry* entries, int cap, ObjString* key) {
	int index = key->hash & (cap-1);
	Entry* tombstone = NULL;
	for (;;) {
		Entry* entry = &entries[index];
		if (entry->key == NULL) { //may be empty bucket or tombstone
			if (IS_NIL(entry->value)) { //empty bucket
				return tombstone != NULL ? tombstone : entry;
			}
			else {//tombstone encountered. set tombstone flag
				if (tombstone == NULL) tombstone = entry;
			}
		}
		if (entry->key == key ) { //key found, return  bucket
			return entry;
		}
		index = ((index + 1) & (cap-1));
	}
}

void adjustCapacity(Table* table, int capacity) {
	Entry* n_entries = ALLOCATE(Entry, capacity);
	table->count = 0;
	for (int i = 0; i < capacity; i++) {
		n_entries[i].key = NULL;
		n_entries[i].value = NIL_VAL;
	}
	for (int i = 0; i < table->capacity; i++) {
		Entry* entry = &table->entries[i];
		if (entry->key == NULL) continue;
		Entry* ent = findEntry(n_entries, capacity, entry->key);
		ent->key = entry->key;
		ent->value = entry->value;
		table->count++;
	}
	FREE_ARRAY(Entry, table->entries, table->capacity);
	table->entries = n_entries;
	table->capacity = capacity;
}
bool tableSet(Table* table, ObjString* key, Value value) {
	if (table->count + 1 > table->capacity* TLOAD_FACTOR) {
		int ncap = GROW_CAPACITY(table->capacity);
		adjustCapacity(table, ncap);
	}
	Entry* entry = findEntry(table->entries, table->capacity, key);
	bool isNewKey = entry->key == NULL;
	if (isNewKey && IS_NIL(entry->value)) table->count++;
	entry->key = key;
	entry->value = value;
	return isNewKey;
}
bool tableGet(Table* table, ObjString* key, Value* value) {
	if (table->count == 0) return false;
	Entry* entry = findEntry(table->entries, table->capacity, key);
	if (entry->key == NULL) {
		return false;
	}
	if (entry->key == key) *value = entry->value;
	return true;
}

bool tableDelete(Table* table, ObjString* key) {
	if (table->count == 0) return false;
	Entry* entry = findEntry(table->entries, table->capacity, key);
	if (entry->key == NULL) return false;
	//tombstone
	entry->key = NULL;
	entry->value = BOOL_VAL(true);
}

bool tableAddAll(Table* from, Table* to) {
	for (int i = 0; i < from->capacity; i++) {
		Entry* entry = &from->entries[i];
		if (entry->key == NULL) continue;
		tableSet(to, entry->key, entry->value);
	}
}

ObjString* tableFindString(Table* table,char* chars, int length, uint32_t hash) { //for string interning
	if (table->count == 0) return NULL;
	int index = hash & (table->capacity - 1);
	for (;;) {
		Entry* entry = &table->entries[index];
		if (entry->key == NULL) {
			if (IS_NIL(entry->value)) return NULL;
		}
		if (entry->key->length == length 
			&& entry->key->hash == hash 
			&& memcmp(entry->key->chars, chars, length) == 0) return entry->key;
		index = (index + 1) & (table->capacity-1);
	}
}
