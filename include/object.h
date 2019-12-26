#ifndef OBJECT_H
#define OBJECT_H
#include "value.h"
#include "common.h"


typedef enum {
	T_STRING_OBJ
}ObjectType;


struct sObj {
	ObjectType type;
	struct sObj* next;
};

struct sObjString {
	Obj* obj;
	int length;
	char* chars;
	uint32_t hash;
};

bool inline isValueType(Value val, ObjectType type) { return AS_OBJ(val)->type == type; }
#define ALLOCATE_OBJ(obj, objType) (obj*)allocateObject(sizeof(obj), objType)

ObjString* copyString(const char* chr, int length);
ObjString* allocateString(const char* ch, int length, uint32_t hashcode);
Obj* allocateObject(size_t size, ObjectType type);


#endif