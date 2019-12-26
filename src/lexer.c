#include "../include/lexer.h"
#include <stdbool.h>
#include <string.h>

void initLexer(const char* src) {
	lexer.start = src;
	lexer.current = src;
	lexer.line = 1;
}

static char advance() {
	lexer.current++;
	return *(lexer.current - 1);
}

static char peek() { return *lexer.current; }
static char peekNext() { return *(lexer.current + 1); }

void skipWhitespace() {
	for (;;) {
		char ch = peek();
		switch (ch) {
		case ' ':
		case '\t':
		case '\r':
			advance(); break;
		case '\n': lexer.line++; advance(); break;
		case '/': skipComment(); return;
		default: return;
		}
	}
}
void skipComment() {
	if (peek() == '/' && peekNext() == '/') {
		while (peek() != '\n')
			advance();
		lexer.line++;
		advance();
	}
	
}

static Token makeToken(TokenType type) {
	Token token;
	token.type = type;
	token.line = lexer.line;
	token.start = lexer.start;
	token.length = (int)(lexer.current - lexer.start);
	return token;
}

static _Bool match(char ch) {
	if (isAtEnd()) return false;
	if (*lexer.current != ch) return false; //advance() pushes lexer.current 1 step forward
	lexer.current++;
	return true;
}

TokenType matchend(int b_ind, int r_ind, const char* rem, TokenType type) {
	if ((int)(lexer.current - lexer.start) == b_ind + r_ind && memcmp(lexer.start + b_ind, rem, r_ind) == 0)
		return type;
	return TOKEN_IDENTIFIER;
}

static Token identifier() {
	while (isAlpha(*lexer.current) || isDigit(*lexer.current)) advance();
	return makeToken(identifierType());
}

static TokenType identifierType() {
	switch (*lexer.start) {
	case 'a': return matchend(1, 2, "nd", TOKEN_AND);
	case 'c': return matchend(1, 4, "lass", TOKEN_CLASS);
	case 'e': return matchend(1, 3, "lse", TOKEN_ELSE);
	case 'i': return matchend(1, 1, "f", TOKEN_IF);
	case 'n': return matchend(1, 2, "il", TOKEN_NIL);
	case 'o': return matchend(1, 1, "r", TOKEN_OR);
	case 'p': return matchend(1, 4, "rint", TOKEN_PRINT);
	case 'r': return matchend(1, 5, "eturn", TOKEN_RETURN);
	case 's': return matchend(1, 4, "uper", TOKEN_SUPER);
	case 'v': return matchend(1, 2, "ar", TOKEN_VAR);
	case 'w': return matchend(1, 4, "hile", TOKEN_WHILE);
	case 'f': {
		switch (*(lexer.start+1)) {
		case 'o': return matchend(2, 1, "r", TOKEN_FOR);
		case 'u': return matchend(2, 1, "n", TOKEN_FUN);
		case 'a': return matchend(2, 3, "lse", TOKEN_FALSE);
		}
	}
	case 't':
		switch (*(lexer.start + 1)) {
		case 'h': return matchend(2, 2, "is", TOKEN_THIS);
		case 'r': return matchend(2, 2, "ue", TOKEN_TRUE);
		}
	default: return TOKEN_IDENTIFIER;
	}
}

static Token number() {
	while (isDigit(*lexer.current) && !isAtEnd()) {
		advance();
	}
	if (peek() == '.' && (peekNext() == ' ' || isDigit(peekNext()))) advance();
	while (isDigit(*lexer.current) && !isAtEnd()) advance();
	return makeToken(TOKEN_NUMBER);
}

static _Bool isAtEnd() { return *lexer.current == '\0'; }

static _Bool isAlpha(char ch) {
	//if (isAtEnd()) return false;
	if ('a' <= ch && 'z' >= ch || 'A' <= ch && 'Z' >= ch || ch == '_')
		return true;
	return false;
}

static _Bool isDigit(char ch) {
	//if (isAtEnd()) return false;
	if (ch >= '0' && ch <= '9') return true;
	return false;
}

static Token string() {
	while (peek() != '\'' && !isAtEnd()) {
		advance();
		if (peek() == '\n') lexer.line++;
	}
	if (isAtEnd()) return errorToken("Unterminated string!");
	advance();
	return makeToken(TOKEN_STRING);
}

Token errorToken(const char* msg) {
	Token token;
	token.start = msg;
	token.length = strlen(msg);
	token.type = TOKEN_STRING;
	token.line = lexer.line;
	return token;
}
Token getToken() {
	skipWhitespace();
	lexer.start = lexer.current;
	if (isAlpha(*lexer.current)) { return identifier(); }
	if (isDigit(*lexer.current)) { return number(); }
	char ch = advance();
	
	switch (ch) {
	case '+': return makeToken(TOKEN_PLUS);
	case '-': return makeToken(TOKEN_MINUS);
	case '*': return makeToken(TOKEN_MULT);
	case '/': return makeToken(TOKEN_DIV);
	//case '%': return makeToken(TOKEN_MOD);
	case '(': return makeToken(TOKEN_LPAREN);
	case ')': return makeToken(TOKEN_RPAREN);
	case '{': return makeToken(TOKEN_LBRACE);
	case '}': return makeToken(TOKEN_RBRACE);
	case ';': return makeToken(TOKEN_SEMI);
	case '.': return makeToken(TOKEN_DOT);
	case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
	case '>': return makeToken(match('=') ? TOKEN_G_THAN_EQUAL : TOKEN_G_THAN);
	case '<': return makeToken(match('=') ? TOKEN_L_THAN_EQUAL : TOKEN_L_THAN);
	case '!': return makeToken(match('=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);
	case '\'': return string();
	default: return makeToken(TOKEN_EOF);
	}
}