#ifndef CLOXEXP_CHUNK_H
#define CLOXEXP_CHUNK_H
#include "common.h"
#include "value.h"

typedef enum {
	OP_RETURN,
	OP_CONSTANT,
	OP_NEGATE,
	OP_ADD,
	OP_DIVIDE,
	OP_R_NEGATE,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_NOT,
	OP_FALSE,
	OP_TRUE,
	OP_NIL,
	OP_GREATER,
	OP_LESS,
	OP_EQUAL,
	OP_PRINT,
	OP_POP,
	OP_DEFINE_GLOBAL,
	OP_GET_GLOBAL,
	OP_SET_GLOBAL,
	OP_SET_LOCAL,
	OP_GET_LOCAL,
	OP_JUMP_IF_FALSE,
	OP_JUMP,
	OP_LOOP
}op_code;

typedef struct {
	int count;
	int capacity;
	uint8_t* code;
	int* lines;
	ValueArray constants;
}Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value constant);
#endif
