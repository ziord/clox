#include "../include/chunk.h"
#include "../include/memory.h"

void initChunk(Chunk* chunk) {
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	chunk->lines = NULL;
	initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);
	freeValueArray(&chunk->constants);
	initChunk(chunk);
}
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
	if (chunk->capacity < chunk->count + 1) {
		int oldC = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldC);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldC, chunk->capacity);
		chunk->lines = GROW_LINEARRAY_S(chunk->lines, oldC, chunk->capacity);
	}
	chunk->code[chunk->count] = byte;
	chunk->lines[chunk->count] = line;
	chunk->count++;
}

int addConstant(Chunk* chunk, Value constant) {
	writeValueArray(&chunk->constants, constant);
	return chunk->constants.count - 1;
}