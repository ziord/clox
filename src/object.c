#include "../include/object.h"
#include "../include/memory.h"
#include <string.h>
#include "../include/vm.h"
#include "../include/table.h"

ObjString* copyString(const char* chr, int length) {
	uint32_t hashcode = hashFunc(chr, length);
	ObjString* obStrInterned = tableFindString(&vm.table, chr, length, hashcode); //the interned string
	if (obStrInterned != NULL) {
		return obStrInterned; //is returned if found, to allow multiple pointers point to the same string
	}
	char* heapChars = (char*)malloc((length+1) * sizeof(char));//ALLOCATE(char, length+1);
	if (heapChars == NULL) return NULL;
	memcpy(heapChars, chr, length);
	heapChars[length] = '\0';
	return allocateString(heapChars, length, hashcode);
}
ObjString* allocateString(const char* ch, int length, uint32_t hashcode) {
	ObjString* obstr = ALLOCATE_OBJ(ObjString, T_STRING_OBJ);
	obstr->chars = ch;
	obstr->length = length;
	obstr->hash = hashcode;
	tableSet(&vm.table, obstr, NIL_VAL); //string interning
	return obstr;
}
Obj* allocateObject(size_t size, ObjectType type) {
	//extern VM vm;
	Obj* obj = (Obj*)reallocate(NULL, 0, size);
	obj->type = type;
	//obj->next = vm.objects;        --------- bug
	vm.objects[vm.objSize] = obj; //add to head
	vm.objSize++;
	return obj;
}
