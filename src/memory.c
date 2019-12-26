#include "../include/memory.h"
#include "../include/vm.h"
//extern VM vm;

void* reallocate(void* prev, size_t oldC, size_t newC) {
	if (newC == 0) {
		free(prev);
		return NULL;
	}
	else {
		void* temp = realloc(prev, newC);
		if (temp == NULL) {
			fprintf(stderr, "Realloc failed\n");
			return prev;
		}
		prev = temp;
		return prev;
	}
}
void* growLineArray(int* prev, size_t oldC, size_t newC) {
	int* newArr = realloc(prev, newC);
	if (newArr == NULL) {
		fprintf(stderr, "Realloc failed\n");
		return prev;
	}
	for (size_t i = 0; i < oldC; i++) {
		newArr[i] = *(prev + i);
	}
	//free(prev);
	return newArr;
}

void freeObj(Obj* obj) {
	switch (obj->type) {
	case T_STRING_OBJ: {
		ObjString* str = ((ObjString*)obj);
		FREE_ARRAY(char, str->chars, str->length+1);
		FREE(ObjString, obj);
		//free(str);
		vm.objSize--;
		fprintf(stdout, "freeing objs\n");
		break;
		}
	}
}

void freeObjects() {
	int size = vm.objSize;
	for (int i = 0; i < size; i++) {
		freeObj(vm.objects[i]);
	}/**
	Obj* cur = vm.objects;
	while (vm.objSize) {
		Obj* next = cur->next;
		freeObj(cur);
		fprintf(stdout, "freeing obj");
		cur = next;
	}*/
}
