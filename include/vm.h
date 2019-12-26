#ifndef CLOXEXP_H
#define CLOXEXP_H
#include "chunk.h"
#include "table.h"

#define MAX_CAPACITY 256
typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
}InterpretResult;

typedef struct {
	Chunk* chunk;
	uint8_t* ip;
	Value stack[MAX_CAPACITY];
	Value* stackTop;
	Obj* objects[MAX_CAPACITY];
	size_t objSize;
	Table table;
	Table globals;
}VM;

void resetStack();
InterpretResult run();
void push(Value val);
Value pop();
static Value peek_(int dist);
bool isFalsey(Value val);
bool valuesEqual(Value v1, Value v2);
void concatenate();
bool objEquality(Value v1, Value v2);
ObjString* extractString(const char* chs, int length);
InterpretResult interpret(const char* source);
void initVM();
void freeVM();
extern VM vm;
#endif