#ifndef SENEGAL_SLEXER_H
#define SENEGAL_SLEXER_H

typedef enum {
    // Keywords
    ASYNC, AWAIT, BREAK, CASE, CLASS, CONTINUE, DEFAULT, ELSE, EXTENDS, FALSE, FINAL, FOR, FUNCTION, IF, IMPORT,
    RETURN, SENEGAL_NULL, SUPER, SWITCH, THIS, TRUE, VAR, WHILE,

    ID,

    // WIP
    INTERPOLATION,

    STRING, NUMBER,

    AMP, AMP_AMP, BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL, EQUAL_GREATER,
    GREATER, GREATER_GREATER, GREATER_EQUAL, LESSER, LESSER_LESSER,
    LESSER_EQUAL, PIPE, PIPE_PIPE, TILDE,

    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,

    CARET, COMMA, COLON, DOT, MINUS, MINUS_EQUAL, MINUS_MINUS, PLUS,
    PLUS_EQUAL, PLUS_PLUS, QUESTION, SEMI, SLASH, SLASH_EQUAL, STAR, STAR_EQUAL, STAR_STAR,

    ERROR, SENEGAL_EOF

} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

typedef struct {
    char* start;
    char* current;
    int line;
} Lexer;


void initLexer(Lexer* lexer, char* source);
Token getNextToken(Lexer* lexer);

#endif //SENEGAL_SLEXER_H