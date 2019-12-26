#ifndef CLOXEXP_VALUE_H
#define CLOXEXP_VALUE_H
#include <stdbool.h>

#define BOOL_VAL(value) ((Value){.type=T_BOOL, {.boolean=value}})
#define IS_BOOL(value) ((value).type == T_BOOL)
#define AS_BOOL(value) ((value).as.boolean)

#define NUMBER_VAL(value) ((Value){.type=T_NUMBER, {.number=value}})
#define IS_NUMBER(value) ((value).type == T_NUMBER)
#define AS_NUMBER(value) ((value).as.number)

#define NIL_VAL ((Value){.type=T_NIL, {.number=0}})
#define IS_NIL(value) ((value).type == T_NIL)

#define OBJ_VAL(value) ((Value){.type=T_OBJ, {.obj=(Obj*)value}})
#define IS_OBJ(value) ((value).type == T_OBJ)
#define AS_OBJ(value) ((value).as.obj)
#define OBJ_TYPE(value) ((AS_OBJ(value)->type))

#define AS_STRING(value) ((ObjString*)(AS_OBJ(value)))
#define AS_CSTRING(value) (((ObjString*)(AS_OBJ(value)))->chars)
#define IS_STRING(value) isValueType(value, T_STRING_OBJ)


typedef struct sObj Obj;
typedef struct sObjString ObjString;

typedef enum {
	T_BOOL,
	T_NUMBER,
	T_NIL,
	T_OBJ
}ValueType;

typedef struct {
	ValueType type;
	 union {
		 bool boolean;
		 double number;
		 Obj* obj;
	}as;
}Value;

typedef struct {
	int count;
	int capacity;
	Value* values;
}ValueArray;


void initValueArray(ValueArray* va);
void freeValueArray(ValueArray* va);
void writeValueArray(ValueArray* va, Value val);
#endif
