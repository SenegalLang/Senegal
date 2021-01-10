#ifndef SENEGAL_SCOMPILER_H
#define SENEGAL_SCOMPILER_H

#include "sinstructions.h"
#include "sparser.h"

void initCompiler(VM* vm, Parser* parser, Compiler* old, Compiler* compiler, FunctionType type, bool getPath);
GCFunction* endCompilation(VM* vm, Compiler* compiler, Parser* parser, Instructions* instructions);

GCFunction* compile(VM* vm, Compiler* compiler, char* file, char* source, char* senegalPath, char* dir);
void markCompilerRoots(VM* vm, Compiler* compiler);

void advance(Parser* parser, Lexer* lexer);
void consume(Parser* parser, Lexer* lexer, SenegalTokenType type, const char* message);

void error(Parser* parser, Token* token, const char* message);

uint8_t newConstant(VM* vm, Parser* parser, Compiler* compiler, Instructions* i, Constant c);

void writeByte(VM* vm, Parser* parser, Instructions* instructions, uint8_t byte);
void writeShort(VM* vm, Parser* p, Instructions* i, uint8_t byte1, uint8_t byte2);
void writeLoad(VM* vm, Parser* parser, Compiler* compiler, Instructions* i, Constant c);
void writeRetByte(VM* vm, Compiler* compiler, Parser* parser, Instructions* instructions);

#endif //SENEGAL_SCOMPILER_H