#include "../include/debug.h"
#include "../include/vm.h"
static const char* readFile(const char* filename);
void repl();
void runFile(const char* filename);
const char* readFile(const char* filename);
int main(int argc, char**argv){ 
	initVM();
	if (argc == 1) {
		//repl();
		//compile("print 3+5;\nfun 56/34;\nprint fun  4+3;\n print 34/56; \n");
		interpret("{var x = 6;  while (x < 24){x = x+1; print x; print 'counting'; if (x == 20){x = 30;}} print 'x after iteration'; print x;}" ); //var x = 34; print x;
		//while (y < 10 ){y = y+1; print y;}
		//if (x == 5){print 'yes';}else{print 'no';}
		//while (y < 10 ){y = y+1;} print y;
			//print x+x; print y; print 'xanthosis';}"
	}
	else if (argc == 2) {
		runFile(argv[1]);
	}
	else {
		fprintf(stderr, "Usage cloxEXP.exe [filename]\n");
	}
	freeVM();
	return 0;
}

void repl() {
	char line[1024];
	for (;;) {
		fprintf(stdout, ">>> ");
		if (!fgets(line, 1024, stdin)) {
			fprintf(stdout, "\n");
			break;
		}
		interpret(line);
	}
}

const char* readFile(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error opening file %s\n", filename);
		exit(EXIT_FAILURE);
	}
	fseek(fp, 0L, SEEK_END);
	size_t bytecount = ftell(fp);
	rewind(fp);
	const char* buf = (char*)malloc(bytecount+1);
	if (buf == NULL) {
		fprintf(stderr, "Could not allocate enough memory to read file %s\n", filename);
		exit(70);
	}
	if (fread(buf, sizeof(char), bytecount, fp) < bytecount) {
		fprintf(stderr, "Error reading file %s", filename);
		exit(65);
	}
	return buf;
}

void runFile(const char* filename) {
	const char* source = readFile(filename);
	InterpretResult res = interpret(source);
	free(source);
	if (res == INTERPRET_COMPILE_ERROR) exit(70);
	if (res == INTERPRET_RUNTIME_ERROR) exit(65);
}