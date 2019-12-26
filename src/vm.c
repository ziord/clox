#include "../include/vm.h"
#include "../include/debug.h"
#include "../include/compiler.h"
#include <stdarg.h>
#include "../include/value.h"
#include <string.h>
#include "../include/memory.h"


VM vm;

void resetStack() {
	vm.stackTop = vm.stack;
}

void push(Value val) {
	*vm.stackTop = val;
	vm.stackTop++;
}
Value pop() {
	vm.stackTop--;
	return *vm.stackTop;
}
void initVM() {
	resetStack();
	//vm.objects = NULL;
	vm.objSize = 0;
	initTable(&vm.table);
	initTable(&vm.globals);
}
void freeVM(){
	freeTable(&vm.globals);
	freeTable(&vm.table);
	freeObjects();
	//resetStack();
}

InterpretResult interpret(const char* src) {
	Chunk chunk;
	initChunk(&chunk);
	if (!compile(src, &chunk)) {
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}
	vm.chunk = &chunk;
	vm.ip = chunk.code;
	InterpretResult res = run();
	return INTERPRET_OK;
}
static Value peek_(int dist) {
	return vm.stackTop[-1 - dist];
}

void runtimeError(const char* fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	vprintf(stderr, fmt);  //argp
	va_end(argp);
	putc('\n', stderr);
	size_t offset = vm.ip - vm.chunk->code;
	fprintf(stderr, "[line %d] in script\n", offset);
}

bool isFalsey(Value val) {
	return IS_NIL(val) || (IS_BOOL(val) && !AS_BOOL(val));
}
bool valuesEqual(Value v1, Value v2) {
	if (v1.type != v2.type) return false;
	switch (v1.type) {
	case T_BOOL: return AS_BOOL(v1) == AS_BOOL(v2);
	case T_NIL: return true;
	case T_NUMBER: return AS_NUMBER(v1) == AS_NUMBER(v2);
	case T_OBJ: return AS_OBJ(v1) == AS_OBJ(v2); //objEquality(v1, v2);
	}
}

bool objEquality(Value v1, Value v2) {
	switch (OBJ_TYPE(v1)) {
	case T_STRING_OBJ: return (AS_STRING(v1)->length == AS_STRING(v2)->length
		&& (memcmp(AS_CSTRING(v1), AS_CSTRING(v2), AS_STRING(v1)->length) == 0));
	}
}
ObjString* extractString(const char* chs, int length) {
	uint32_t hash = hashFunc(chs, length);
	ObjString* internedStr = tableFindString(&vm.table, chs, length, hash);
	if (internedStr != NULL) { //if the string is interned use the interned one instead, and free the other string
		FREE_ARRAY(char, chs, length);
		return internedStr;
	}
	return allocateString(chs, length, hash);
}

void concatenate() {
	Value v2 = pop();
	Value v1 = pop();
	int l1 = AS_STRING(v1)->length;
	int l2 = AS_STRING(v2)->length;
	int length = l1 + l2;
	char* chs = (char*)malloc((length+1)*sizeof(char));//ALLOCATE(char, length + 1);
	memcpy(chs, AS_CSTRING(v1), l1);
	memcpy(chs + l1, AS_CSTRING(v2), l2);
	chs[length] = '\0';
	push(OBJ_VAL(extractString(chs, length)));
	//free(chs);
}

InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() vm.chunk->constants.values[READ_BYTE()]
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT() (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8 ) | vm.ip[-1]))
#define BINARY_OP(valueType, OP) \
					do{\
						if (!IS_NUMBER(peek_(0)) || !IS_NUMBER(peek_(1))){ \
							runtimeError("Operands must be numbers"); \
							return INTERPRET_RUNTIME_ERROR; \
						} \
						double b = AS_NUMBER(pop());  \
						double a = AS_NUMBER(pop());  \
						push(valueType(a OP b));  \
					}while(false) 
	//to debug or not to debug


	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		printf("[");
		for (Value* st = vm.stack; st < vm.stackTop; st++) {
			printValue(*st);
		}printf("]\n");
		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
		uint8_t INSTRUCTION;
		switch (INSTRUCTION = READ_BYTE()) {
		case OP_RETURN: {
			return INTERPRET_OK;
		}
		case OP_CONSTANT: push(READ_CONSTANT()); break;
		case OP_ADD: {
			if (IS_NUMBER(peek_(0)) && IS_NUMBER(peek_(1))) {
				BINARY_OP(NUMBER_VAL, +); break;
			}
			else if (IS_STRING(peek_(0)) && IS_STRING(peek_(1))) {
				concatenate(); break;
			}
			else {
				runtimeError("Operands must be strings or numbers!");
				return INTERPRET_RUNTIME_ERROR;
			}
			
		}
		case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
		case OP_DIVIDE: BINARY_OP(NUMBER_VAL, / ); break;
		case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
		case OP_NEGATE: {
			if (!IS_NUMBER(peek_(0))) {
				runtimeError("Operand must be a number!");
				return INTERPRET_RUNTIME_ERROR;
			}
			push(NUMBER_VAL(-AS_NUMBER(pop()))); break;
		}
		case OP_NOT: {
			push(BOOL_VAL(isFalsey(pop()))); break;
		}
		case OP_GREATER: BINARY_OP(BOOL_VAL, > ); break;
		case OP_LESS: BINARY_OP(BOOL_VAL, < ); break;
		case OP_EQUAL: push(BOOL_VAL(valuesEqual(pop(), pop()))); break;
		case OP_TRUE: push(BOOL_VAL(true)); break;
		case OP_FALSE: push(BOOL_VAL(false)); break;
		case OP_NIL: push(NIL_VAL); break;
		case OP_POP: pop(); break;
		case OP_PRINT: {
			printValue(pop());
			printf("\n");
			break;
		}
		case OP_DEFINE_GLOBAL: {
			ObjString* name = READ_STRING();
			tableSet(&vm.globals, name, peek_(0));
			pop();
			break;
		}
		case OP_GET_GLOBAL: {
			ObjString* name = READ_STRING();
			Value value;
			if (!tableGet(&vm.globals, name, &value)) { 
				runtimeError("Undefined variable referenced before assignment->'%s'", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			push(value);
			break;
		}
		case OP_SET_GLOBAL: {
			ObjString* name = READ_STRING();
			Value value;
			if (tableSet(&vm.globals, name, peek_(0))) { //returns isNewKey
				tableDelete(&vm.globals, name);
				runtimeError("Variable referenced before definition->'%s'", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_SET_LOCAL: {
			uint8_t slotInd = READ_BYTE();
			vm.stack[slotInd] = peek_(0); //don't pop() - result of assignment expr is the assigned value itself
			//that needs to remain on the stack
			break;
		}
		case OP_GET_LOCAL: {
			uint8_t slotInd = READ_BYTE();
			push(vm.stack[slotInd]); //push value at slotInd to stack top
			break;
		}
		case OP_JUMP_IF_FALSE: {
			uint16_t offset = READ_SHORT();
			if (isFalsey(peek_(0))) {
				vm.ip += offset;
			}
			break;
		}
		case OP_JUMP: {
			uint16_t offset = READ_SHORT();
			vm.ip += offset;
			break;
		}
		case OP_LOOP: {
			uint16_t offset = READ_SHORT();
			vm.ip -= offset;
			break;
		}
		}

		//#endif
#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
	}
}