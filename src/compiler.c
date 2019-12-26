#include "../include/compiler.h"
#include "../include/debug.h"
//#include "value.h"
#include "../include/chunk.h"
#include "../include/object.h"

Compiler* current = NULL;

void initCompiler(Compiler* compiler) {
	compiler->localCount = 0;
	compiler->scopeDepth = 0;
	current = compiler;
}

typedef struct {
	Token previous;
	Token current;
	bool hadError;
	bool panicMode;
}Parser;

Parser parser;
typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT, //=
	PREC_OR, //and, or, 
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARIS0N, //>, <, <=, >=
	PREC_TERM, //+, -
	PREC_FACTOR, //*, /
	PREC_UNARY, //!, -, +
	PREC_CALL, //., (), [] 
	PREC_PRIMARY
}Precedence;

typedef void (*ParseFn)(bool);
Chunk* compilingChunk;
typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precendence;
}ParseRule;

static void binary(bool);
static void unary(bool);
static void grouping(bool);
static void number_(bool);
static void literal_(bool);
static void string_(bool);
static void printStatement(bool);
static void variable(bool);
static void namedVariable(Token varname, bool);
static void and_(bool canAssign);
static void or_(bool canAssign);


ParseRule rulesTable[] = {  //indexed according to TokenType arrangement for Pratt parsing
	{unary, binary, PREC_TERM}, //TOKEN_PLUS
	{unary, binary, PREC_TERM}, //TOKEN_MINUS
	{NULL, binary, PREC_FACTOR}, //TOKEN_MULT
	{NULL, binary, PREC_FACTOR}, //TOKEN_DIV
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, binary, PREC_NONE}, //TOKEN_EQUAL
	{unary, NULL, PREC_NONE}, //TOKEN_NOT
	{NULL, binary, PREC_COMPARIS0N}, //TOKEN_G_THAN
	{NULL, binary, PREC_COMPARIS0N}, //TOKEN_L_THAN
	//{NULL, binary, PREC_MOD},
	{grouping, NULL, PREC_NONE}, //
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},  //TOKEN_LBRACE
	{NULL, NULL, PREC_NONE},
	{NULL, binary, PREC_EQUALITY}, //TOKEN_EQUAL_EQUAL
	{NULL, binary, PREC_EQUALITY}, //TOKEN_NOT_EQUAL
	{NULL, binary, PREC_COMPARIS0N}, //TOKEN_L_THAN_EQUAL
	{NULL, binary, PREC_COMPARIS0N}, //TOKEN_G_THAN_EQUAL
	{variable, NULL, PREC_NONE}, //TOKEN_IDENTIFIER
	{string_, NULL, PREC_NONE},
	{number_, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE},
	{NULL, or_, PREC_OR},
	{NULL, NULL, PREC_NONE},
	{NULL, and_, PREC_AND},
	{NULL, NULL, PREC_NONE},
	{printStatement, NULL, PREC_NONE}, //TOKEN_PRINT
	{NULL, NULL, PREC_NONE},
	{literal_, NULL, PREC_NONE}, //TOKEN_TRUE
	{literal_, NULL, PREC_NONE},//TOKEN_FALSE
	{literal_, NULL, PREC_NONE}, //TOKEN_NIL
	{NULL, NULL, PREC_NONE},
	{NULL, NULL, PREC_NONE}
};

static void adv();
static void errorAtCurrent(const char* msg);
static void error(const char* msg);
static void errorAt(Token* token, const char* msg);
static void consume(TokenType type, const char* msg);
static void expression();
static void parsePrecedence(Precedence);
static void storeByte(uint8_t byte);
static void storeBytes(uint8_t byte, uint8_t byte2);
static void storeConstant(Value val);
static void block();

static void expressionStatement();
static void varDeclaration();
static void statement();
static void declaration();
static bool check(TokenType);
static bool match_(TokenType);
static uint8_t parseVariable(const char* msg);
static uint8_t identifierConstant(Token*);
static void defineVariable(uint8_t);
static void synchronize();
static void markInitialized();
static bool identifiersEqual(Token*, Token*);
static void declareVariable();
static int resolveLocal(Compiler* current, Token* name);
static void beginScope();
static void endScope();
static void addLocal(Token);
static void ifStatement();
static int storeJump(uint8_t instruction);
static void storeLoop(int loopStart);
static uint8_t makeConstant(Value val);
static void forStatement();
static Chunk* currentChunk();
ParseRule* getRule(TokenType);

static Chunk* currentChunk() {
	return compilingChunk;
}
static void storeByte(uint8_t byte) {
	writeChunk(currentChunk(), byte, parser.previous.line);
}
static void storeBytes(uint8_t byte, uint8_t byte2) {
	storeByte(byte);
	storeByte(byte2);
}

static void storeConstant(Value val) {
	//AS_OBJ(val)->type = T_STRING_OBJ;
	storeBytes(OP_CONSTANT, makeConstant(val));
}

static uint8_t makeConstant(Value val) {
	int ind = addConstant(compilingChunk, val);
	if (ind > UINT8_MAX) {
		fprintf(stderr, "Too many values in a chunk\n");
		return 0;
	}
	return (uint8_t)ind;
}

static int storeJump(uint8_t instruction) {
	storeByte(instruction);
	storeByte(0xff);
	storeByte(0xff);
	return currentChunk()->count - 2;
}


static void patchJump(int offset) {
	int jump = currentChunk()->count - offset - 2;
	if (jump > UINT16_MAX) {
		error("Too much code to jump over.");
	}
	currentChunk()->code[offset] = (jump >> 8) & 0xff;
	currentChunk()->code[offset + 1] = jump & 0xff;
}

static void storeLoop(int loopStart) {
	storeByte(OP_LOOP);
	int offset = currentChunk()->count - loopStart + 2;
	if (offset > UINT16_MAX) error("Loop body too large");
	storeByte((offset >> 8) & 0xff);
	storeByte(offset & 0xff);
}

static void consume(TokenType type, const char* msg) {
	if (parser.current.type != type) {
		errorAtCurrent(msg);
		return;
	}
	adv();
}

static void errorAtCurrent(const char* msg) {
	errorAt(&parser.current, msg);
}
static void error(const char* msg) {
	errorAt(&parser.previous, msg);
}
static void errorAt(Token* token, const char* msg) {
	if (parser.panicMode) return;
	parser.panicMode = true;
	fprintf(stderr, "[line: %d] Error: ", token->line);
	if (token->type == TOKEN_EOF) {
		fprintf(stderr, "at end\n");
	}
	else if (token->type == TOKEN_ERROR) {

	}
	else {
		fprintf(stderr, "%s -> %.*s\n", msg, token->length, token->start);
	}
	parser.hadError = true;
}

static void adv() {
	parser.previous = parser.current;
	for (;;) {
		parser.current = getToken();
		if (parser.current.type != TOKEN_EOF) break;
		//errorAtCurrent(parser.current.start);
		return;
	}
}

ParseRule* getRule(TokenType type) {
	return &rulesTable[type];
}

void parsePrecedence(Precedence precedence) {
	adv();
	ParseFn prefix = getRule(parser.previous.type)->prefix;
	if (prefix == NULL) {
		error(parser.previous.start);
		return;
	}
	bool canAssign = precedence <= PREC_ASSIGNMENT;
	prefix(canAssign);
	while (precedence <= getRule(parser.current.type)->precendence) {
		adv();
		ParseFn infix = getRule(parser.previous.type)->infix;
		infix(canAssign);
	}
	if (canAssign && match_(TOKEN_EQUAL)) {
		error("Invalid assignment target");
		expression(); 
	}
}
void expression() {
	parsePrecedence(PREC_ASSIGNMENT);
}

void grouping(bool canAssign) {
	expression();
	consume(TOKEN_RPAREN, "Expected left parenthesis");
}

void unary(bool canAssign) {
	TokenType operatorType = parser.previous.type;
	parsePrecedence(PREC_UNARY);
	switch (operatorType) {
	case TOKEN_MINUS: storeByte(OP_NEGATE); break;
	case TOKEN_PLUS: storeBytes(OP_NEGATE, OP_NEGATE); break;
	case TOKEN_NOT: storeByte(OP_NOT); break;
	default: return;
	}
}
void number_(bool canAssign) {
	Value val = NUMBER_VAL(strtod(parser.previous.start, NULL));
	storeConstant(val);
}

void string_(bool canAssign) {
	storeConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}
void literal_(bool canAssign) {
	switch (parser.previous.type) {
	case TOKEN_FALSE: storeByte(OP_FALSE); break;
	case TOKEN_TRUE: storeByte(OP_TRUE); break;
	case TOKEN_NIL: storeByte(OP_NIL); break;
	}
}
void binary(bool canAssign) {
	TokenType operatorType = parser.previous.type;
	ParseRule* rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->precendence + 1));
	switch (operatorType) {
	case TOKEN_PLUS: storeByte(OP_ADD); break;
	case TOKEN_MINUS: storeByte(OP_SUBTRACT); break;
	case TOKEN_DIV: storeByte(OP_DIVIDE); break;
	case TOKEN_MULT: storeByte(OP_MULTIPLY); break;
	case TOKEN_EQUAL_EQUAL: storeByte(OP_EQUAL); break;
	case TOKEN_G_THAN: storeByte(OP_GREATER); break;
	case TOKEN_L_THAN: storeByte(OP_LESS); break;
	case TOKEN_G_THAN_EQUAL: storeBytes(OP_LESS, OP_NOT); break;
	case TOKEN_L_THAN_EQUAL: storeBytes(OP_GREATER, OP_NOT); break;
	case TOKEN_NOT_EQUAL: storeBytes(OP_EQUAL, OP_NOT);
	default: return;
	}
}

void and_(bool canAssign) {
	int endJump = storeJump(OP_JUMP_IF_FALSE); //skip other conditions if first condition is false (vm.ip += 2 in vm.c helps to move one past the jump indices (2 indices) hence landing on the first instruction of a new code
	storeByte(OP_POP);
	parsePrecedence(PREC_AND);
	patchJump(endJump);
}

void or_(bool canAssign) {
	int elseJump = storeJump(OP_JUMP_IF_FALSE);
	int endJump = storeJump(OP_JUMP);
	patchJump(elseJump);
	storeByte(OP_POP);
	parsePrecedence(PREC_OR);
	patchJump(endJump);
}
//LOX GRAMMAR
//statement   - printStatement
//				|expressionStatement
//declaration - varDecl
//				|statement

static void printStatement(bool canAssign) {
	expression();
	consume(TOKEN_SEMI, "Expected ';' at end of statement");
	storeByte(OP_PRINT);
}
static void expressionStatement() {
	expression();
	consume(TOKEN_SEMI, "Expected ';' at end of statement");
	storeByte(OP_POP);
}

void block() {
	while (!check(TOKEN_RBRACE) && !check(TOKEN_EOF)) {
		declaration();
}
	consume(TOKEN_RBRACE, "Expected '}' at end of block");
}

static void varDeclaration() {
	uint8_t globIndex = parseVariable("Expected variable identifier(name)");
	if (match_(TOKEN_EQUAL)) { //var a = 5;
		expression();
	}
	else {
		storeByte(OP_NIL); //var a;
	}
	consume(TOKEN_SEMI, "Expected ';' at end of declaration");
	defineVariable(globIndex);
}
void addLocal(Token name) {
	if (current->localCount == UINT8_MAXLOCAL) {
		error("Too many variables in block");
		return;
	}
	Local* local = &current->locals[current->localCount++];
	local->name = name;
	local->scope = -1;
}
void declareVariable() {
	if (current->scopeDepth == 0) return;
	Token* name = &parser.previous;
	for (int i = current->localCount-1; i >= 0; i--) {
		Local* local = &current->locals[i];
		if (local->scope != -1 && local->scope < current->scopeDepth)
			break;
		if (identifiersEqual(name, &local->name)) {
			error("Redefinition of already existing variable in the same scope");
		}
	}
	addLocal(*name);
}
uint8_t parseVariable(const char* msg) {
	consume(TOKEN_IDENTIFIER, msg);
	declareVariable();
	if (current->scopeDepth > 0) return 0;  //return dummy index
	return identifierConstant(&parser.previous);
}

uint8_t identifierConstant(Token* token) {
	return makeConstant(OBJ_VAL(copyString(token->start, token->length)));
}

void defineVariable(uint8_t gi) {
	if (current->scopeDepth > 0) {
		markInitialized();
		return;
	}
	storeBytes(OP_DEFINE_GLOBAL, gi);
}

static void variable(bool canAssign) {
	namedVariable(parser.previous, canAssign);
}
bool identifiersEqual(Token* t1, Token* t2) {
	if (t1->length != t2->length) return false;
	return memcmp(t1->start, t2->start, t1->length)==0;
}

int resolveLocal(Compiler* current, Token* name) {
	for (int i = current->localCount - 1; i >= 0; i--) { //start from end of locals array to allow name shadowing
		Local* local = &current->locals[i];
		if (identifiersEqual(name, &local->name)) {
			if (local->scope == -1) {
				error("Variable referenced in its own definition");
			}
			return i;
		}
	}
	return -1;
}

void markInitialized() {
	current->locals[current->localCount - 1].scope = current->scopeDepth;
}
static void namedVariable(Token varname, bool canAssign) {
	uint8_t setOp, getOp;
	int arg = resolveLocal(current, &varname);
	if (arg != -1) {
		setOp = OP_SET_LOCAL;
		getOp = OP_GET_LOCAL;
	}
	else {
		setOp = OP_SET_GLOBAL;
		getOp = OP_GET_GLOBAL;
		arg = identifierConstant(&varname);
	}
	if (canAssign && match_(TOKEN_EQUAL)) { //setter
		expression(); 
		storeBytes(setOp, (uint8_t)arg);
	}
	else { //getter
		storeBytes(getOp, (uint8_t)arg);
	}
}
void beginScope() {
	current->scopeDepth++;
}

void endScope() {
	current->scopeDepth--;
	while (current->localCount > 0 && 
		current->scopeDepth < current->locals[current->localCount-1].scope) { //walking the local var out of scope
		current->localCount--; //remove the var from count
		storeByte(OP_POP); //runtime component of exiting the scope
	}
}

static void ifStatement() {
	consume(TOKEN_LPAREN, "Expected '(' after 'if'.");
	expression();
	consume(TOKEN_RPAREN, "Expected ')' after condition expression");

	int thenJump = storeJump(OP_JUMP_IF_FALSE);
	storeByte(OP_POP);
	statement();
	//if (expr){	OP_JUMP_IF_FALSE - start-index
	//	statement()
	//}
	//else{			OP_JUMP -	end-index   //OP_JUMP serves as the marker to know where to jump to
	//	statement()
	//	}
	int elseJump = storeJump(OP_JUMP);

	patchJump(thenJump);
	storeByte(OP_POP);
	if (match_(TOKEN_ELSE)) {
		statement();
	}
	patchJump(elseJump);

}

void whileStatement() {
	int loopStart = currentChunk()->count;
	consume(TOKEN_LPAREN, "Expected '(' after 'while'");
	expression();
	consume(TOKEN_RPAREN, "Expected ')' after condition expression");

	int exitJump = storeJump(OP_JUMP_IF_FALSE);
	storeByte(OP_POP);
	statement();
	storeLoop(loopStart);
	patchJump(exitJump);
	storeByte(OP_POP);
}

void forStatement() {
	beginScope();
	consume(TOKEN_LPAREN, "Expected '(' after 'for'");
	if (match_(TOKEN_SEMI)) {

	}
	else if (match_(TOKEN_VAR)) {
		varDeclaration();
	}
	else {
		expressionStatement();
	}
	
	int loopStart = currentChunk()->count;
	int exitJump = -1;
	if (!match_(TOKEN_SEMI)) {
		expression();
		consume(TOKEN_SEMI, "Expected ';' after loop condition.");
		exitJump = storeJump(OP_JUMP_IF_FALSE);
		storeByte(OP_POP);
	}
	
	if (!match_(TOKEN_RPAREN)) {
		int bodyJump = storeJump(OP_JUMP);
		int increStart = currentChunk()->count;
		expression();
		storeByte(OP_POP);
		consume(TOKEN_RPAREN, "Expected ')' after for clause.");
		storeLoop(loopStart);
		loopStart = increStart;
		patchJump(bodyJump);
	}
	statement();
	storeLoop(loopStart);
	if (exitJump != -1) {
		patchJump(exitJump);
		storeByte(OP_POP);
	}
	endScope();
}
static void statement() {
	if (match_(TOKEN_PRINT))
		printStatement(false);
	else if (match_(TOKEN_LBRACE)) {
		beginScope();
		block();
		endScope();
	}
	else if (match_(TOKEN_IF)) {
		ifStatement();
	}
	else if (match_(TOKEN_WHILE)) {
		whileStatement();
	}
	else if (match_(TOKEN_FOR)) {
		forStatement();
	}
	else
		expressionStatement();
}



bool check(TokenType type) {
	if (parser.current.type == type) return true;
	return false;
}
static bool match_(TokenType type) {
	if (!check(type)) return false;
	adv();
	return true;
}
static void synchronize() {
	parser.panicMode = false;
	while (parser.current.type != TOKEN_EOF) {
		if (parser.current.type == TOKEN_SEMI) return;
		switch (parser.current.type) {
		case TOKEN_CLASS:
		case TOKEN_FUN:
		case TOKEN_VAR:
		case TOKEN_PRINT:
		case TOKEN_WHILE:
		case TOKEN_FOR:
			return;
		default:;
		}
		adv();
	}
}

static void declaration() {
	//while (parser.current.type != TOKEN_EOF) {
		if (match_(TOKEN_VAR)) {
			varDeclaration();
		}
		else {
			statement();
		}
		if (parser.panicMode) synchronize();
	//}
}

static void endCompiler() {
	//storeByte(OP_RETURN);
#ifdef DEBUG_PRINT_TRACE
	disassembleChunk(currentChunk(), "test");
#endif
}
bool compile(const char* src, Chunk* chunk) {
	initLexer(src);
	Compiler compiler;
	initCompiler(&compiler);
	compilingChunk = chunk;
	parser.hadError = false;
	parser.panicMode = false;
	adv(); //inits parser.current & parser.previous
	declaration();
	consume(TOKEN_EOF, "Expected end of expression");
	endCompiler();
	return !parser.hadError;
}