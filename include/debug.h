#ifndef CLOXEXP_DEBUG_H 
#define CLOXEXP_DEBUG_H
#include "chunk.h"
#include "value.h"
//#define DEBUG_TRACE_EXECUTION

void disassembleChunk(Chunk* chunk, char* name);
 int disassembleInstruction(Chunk* chunk, int offset);
static int simpleInstruction(int offset, char* name);
static int constantInstruction(int offset, Chunk* chunk, char* name);
static int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset);
static int byteInstruction(int offset, Chunk* chunk, char* name);
void printObject(Value val);
void printValue(Value val);
#endif
