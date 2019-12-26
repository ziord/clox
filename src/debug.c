#include "../include/debug.h"
#include "../include/value.h"
#include "../include/object.h"

#define DEBUG_TRACE_EXECUTION

void printObject(Value val) {
	switch (AS_OBJ(val)->type) {
	case T_STRING_OBJ:
		printf("%s", AS_CSTRING(val)); break;
	}
}
void printValue(Value val) {
	switch (val.type) {
	case T_BOOL: AS_BOOL(val) ? printf("true") : printf("false"); break;
	case T_NUMBER: printf("%g", AS_NUMBER(val)); break;
	case T_NIL: printf("nil"); break;
	case T_OBJ: printObject(val); break;
	}
}

/*
	===test===
	0000	120		OP_CONSTANT		0  '3.4'
	0002	|		OP_RETURN
*/

void disassembleChunk(Chunk* chunk, char* name) {
	printf("===%s===\n", name);
	for (int offset = 0; offset < chunk->count;) {
		offset = disassembleInstruction(chunk, offset);
	}
}

int disassembleInstruction(Chunk* chunk, int offset) {
	printf("%04d	", offset);
	if (chunk->count > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
		printf("     |		");
	}
	else {
		printf("%5s%d		", " ", chunk->lines[offset]);
	}
	uint8_t INSTRUCTION = chunk->code[offset];
	switch (INSTRUCTION) {
	case OP_CONSTANT:
		return constantInstruction(offset, chunk, "OP_CONSTANT");
	case OP_RETURN:
		return simpleInstruction(offset, "OP_RETURN");
	case OP_ADD:
		return simpleInstruction(offset, "OP_ADD");
	case OP_SUBTRACT:
		return simpleInstruction(offset, "OP_SUBTRACT");
	case OP_DIVIDE:
		return simpleInstruction(offset, "OP_DIVIDE");
	case OP_MULTIPLY:
		return simpleInstruction(offset, "OP_MULTIPLY");
	case OP_NEGATE:
		return simpleInstruction(offset, "OP_NEGATE");
	case OP_NOT:
		return simpleInstruction(offset, "OP_NOT");
	case OP_NIL:
		return simpleInstruction(offset, "OP_NIL");
	case OP_TRUE:
		return simpleInstruction(offset, "OP_TRUE");
	case OP_FALSE:
		return simpleInstruction(offset, "OP_FALSE");
	case OP_GREATER:
		return simpleInstruction(offset, "OP_GREATER");
	case OP_LESS:
		return simpleInstruction(offset, "OP_LESS");
	case OP_EQUAL:
		return simpleInstruction(offset, "OP_EQUAL");
	case OP_R_NEGATE:
		return simpleInstruction(offset, "OP_R_NEGATE");
	case OP_POP:
		return simpleInstruction(offset, "OP_POP");
	case OP_PRINT:
		return simpleInstruction(offset, "OP_PRINT");
	case OP_DEFINE_GLOBAL:
		return constantInstruction(offset, chunk, "OP_DEFINE_GLOBAL");
	case OP_SET_GLOBAL:
		return constantInstruction(offset, chunk, "OP_SET_GLOBAL");
	case OP_GET_GLOBAL:
		return constantInstruction(offset, chunk, "OP_GET_GLOBAL");
	case OP_SET_LOCAL:
		return byteInstruction(offset, chunk, "OP_SET_LOCAL");
	case OP_GET_LOCAL:
		return byteInstruction(offset, chunk, "OP_GET_LOCAL");
	case OP_JUMP:
		return jumpInstruction("OP_JUMP", 1, chunk, offset);
	case OP_JUMP_IF_FALSE:
		return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
	case OP_LOOP:
		return jumpInstruction("OP_LOOP", -1, chunk, offset);
	default:
		printf("%s %d at line: %d", "Invalid Instruction:", INSTRUCTION, chunk->lines[offset]);
	}
}

static int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset) {
	uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
	jump |= chunk->code[offset + 2];
	printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
	return offset + 3;
}
int simpleInstruction(int offset, char* name) {
	printf("%-20s\n", name);
	return offset + 1;
}

int byteInstruction(int offset, Chunk* chunk, char* name) {
	printf("%-20s	%d\n", name, chunk->code[offset + 1]);
	return offset + 2;
}
int constantInstruction(int offset, Chunk* chunk, char* name) {
	uint8_t constantIndex = chunk->code[offset + 1]; //index of a constant is one ahead  the OP_CONSTANT instruction
	printf("%-20s", name);
	printf("	%d	'", constantIndex);
	Value constant = chunk->constants.values[constantIndex];
	printValue(constant);
	printf("'\n");
	return offset + 2;

}