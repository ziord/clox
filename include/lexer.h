#ifndef LEXER_H
#define LEXER_H
typedef struct {
	const char* start;
	const char* current;
	int line;
}Lexer;

typedef enum {
	//single char tokens
	TOKEN_PLUS, TOKEN_MINUS, TOKEN_MULT, TOKEN_DIV,
	TOKEN_DOT, TOKEN_SEMI, TOKEN_EQUAL, TOKEN_NOT,
	TOKEN_G_THAN, TOKEN_L_THAN,
	TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,


	//two char tokens
	TOKEN_EQUAL_EQUAL, TOKEN_NOT_EQUAL,
	TOKEN_L_THAN_EQUAL, TOKEN_G_THAN_EQUAL,

	//literals
	TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

	//reserved
	TOKEN_FOR, TOKEN_WHILE, TOKEN_SUPER, TOKEN_VAR,
	TOKEN_CLASS, TOKEN_IF, TOKEN_IF_ELSE, TOKEN_ELSE,
	TOKEN_OR, TOKEN_THIS, TOKEN_AND, TOKEN_RETURN, TOKEN_PRINT,
	TOKEN_FUN, TOKEN_TRUE, TOKEN_FALSE, TOKEN_NIL,

	//others
	TOKEN_ERROR, TOKEN_EOF
}TokenType;


typedef struct {
	int line;
	const char* start;	//value
	int length;			//value
	TokenType type;
}Token;

Lexer lexer;
void initLexer(const char* src);
Token getToken();
static char advance();
static _Bool match(char);
static Token makeToken(TokenType type);
void skipWhitespace();
void skipComment();
static Token identifier();
static Token number();
static char peek();
static char peekNext();
static _Bool isAlpha(char);
static _Bool isDigit(char);
static TokenType identifierType();
TokenType matchend(int b_ind, int r_ind, const char* rem, TokenType type);
static _Bool isAtEnd();
static Token string();
Token errorToken(const char*);
#endif // !LEXER_H
