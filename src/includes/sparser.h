#ifndef SENEGAL_SPARSER_H
#define SENEGAL_SPARSER_H

#include "sutils.h"
#include "slexer.h"
#include "sinstructions.h"
#include "svm.h"

typedef struct {
    Token current;
    Token previous;
    bool hasError;
    bool panic;

    char* currentFile;
} Parser;

typedef enum {
    NONE,

// =, ||, &&, ==
    ASSIGNMENT, PREC_OR, PREC_AND, EQUALITY,

// (< <= > >=), (| ^ &), (+, -), (*, /), (!, -, ~), (., ())
    COMPARISON, BIT_OR, BIT_XOR, BIT_AND, BIT_SHIFT, TERM, FACTOR ,UNARY, CALL,

    PRIMARY
} Precedence;

typedef struct {
    Token id;
    int depth;

    bool isCaptured;
} Local;

typedef struct Compiler {
    struct Compiler* parent;

    GCFunction* function;
    FunctionType type;
    Local locals[UINT8_COUNT];
    int localCount;
    int depth;

    Upvalue upvalues[UINT8_COUNT];
} Compiler;

typedef struct ClassCompiler {
    struct ClassCompiler* parent;
    Token id;
    bool hasSuper;
} ClassCompiler;

typedef void (*ParseFunc)(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer *lexer, Instructions* i, bool canAssign);

typedef struct {
    ParseFunc prefix;
    ParseFunc infix;
    Precedence precedence;
} ParseRule;

void initParser(Parser* parser, char* file);

bool match(Parser* parser, Lexer* lexer, SenegalTokenType type);

uint32_t hashConstant(Constant c);

void parseExpression(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i);
void parseDeclaration(VM* vm, Compiler* compiler, ClassCompiler* cc, Parser* parser, Lexer* lexer, Instructions* i);
void parseDeclarationOrStatement(VM* vm, Compiler* compiler, ClassCompiler* cc, Parser* parser, Lexer* lexer, Instructions* i);
void parseStatement(VM* vm, Compiler* compiler, ClassCompiler* cc, Parser* parser, Lexer* lexer, Instructions* i);
void parsePrecedence(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Precedence precedence, Instructions* i);

void parseAccess(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseAnd(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseBinary(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseDot(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseFunctionCall(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseGroup(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseHex(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseIdentifier(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseLiteral(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseList(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseMap(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseNumber(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseOr(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseString(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseSuper(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseTernary(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseThis(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);
void parseUnary(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign);

GCString* copyString(VM* vm, Compiler* compiler, const char* chars, int length);
GCString* getString(VM* vm, char* chars, int length);

#endif //SENEGAL_SPARSER_H