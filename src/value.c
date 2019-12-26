#include "../include/value.h"
#include "../include/memory.h"

void initValueArray(ValueArray* va) {
	va->count = 0;
	va->capacity = 0;
	va->values = NULL;
}
void freeValueArray(ValueArray* va) {
	FREE_ARRAY(Value, va, va->capacity);
	initValueArray(va);
}
void writeValueArray(ValueArray* va, Value val) {
	if (va->capacity < va->count + 1) {
		int oldC = va->capacity;
		va->capacity = GROW_CAPACITY(oldC);
		va->values = GROW_ARRAY(Value, va->values, oldC, va->capacity);
	}
	va->values[va->count] = val;
	va->count++;
}