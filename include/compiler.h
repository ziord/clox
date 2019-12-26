#ifndef COMPILER_H
#define COMPILER_H
#include "chunk.h"
#include "lexer.h"
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#define UINT8_MAXLOCAL UINT8_MAX +1

typedef struct {
	Token name;
	int scope;
}Local;

typedef struct {
	Local locals[UINT8_MAXLOCAL];
	int scopeDepth;
	int localCount;
}Compiler;


bool compile(const char* src, Chunk* chunk);
void initCompiler(Compiler* compiler);

#endif
