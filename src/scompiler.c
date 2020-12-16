#include <stdio.h>

#include "includes/sutils.h"
#include "includes/scompiler.h"
#include "includes/sinstruction_utils.h"
#include "includes/smemory.h"

#if DEBUG_PRINT_CODE
#include "includes/sdebug.h"
#include "includes/sinstruction_utils.h"
#include "includes/smemory.h"

#endif

void error(Parser* parser, Token* token, const char* message) {
  if (parser->panic)
    return;

  parser->panic = true;

  fprintf(stderr, "\033[1;31merror: \033[0m%s\n  \033[1;34m--> \033[0mLine %d", message, token->line);

  if (token->type == SENEGAL_EOF) {
    fprintf(stderr, " ( EOF )");
  } else if (token->type == ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " ( %.*s )", token->length, token->start);
  }

  parser->hasError = true;
}

void advance(Parser* parser, Lexer* lexer) {
  parser->previous = parser->current;

  for (;;) {
    parser->current = getNextToken(lexer);

    if (parser->current.type != ERROR)
      break;

    error(parser, &parser->current, parser->current.start);
  }
}

void consume(Parser* parser, Lexer* lexer, TokenType type, const char* message) {
  if (parser->current.type == type) {
    advance(parser, lexer);
    return;
  }
  error(parser, &parser->current, message);
}

void writeByte(VM* vm, Parser* parser, Instructions* instructions, uint8_t byte) {

  writeInstructions(vm, instructions, byte, parser->previous.line);
}

void writeTwoBytes(VM* vm, Parser* p, Instructions* i, uint8_t byte1, uint8_t byte2) {
  writeByte(vm, p, i, byte1);
  writeByte(vm, p, i, byte2);
}

void writeRetByte(VM* vm, Compiler* compiler, Parser* parser, Instructions* instructions) {
  if (compiler->type == CONSTRUCTOR)
    writeTwoBytes(vm, parser, instructions, OPCODE_GETLOC, 0);
  else
    writeByte(vm, parser, instructions, OPCODE_NULL);

  writeByte(vm, parser, instructions, OPCODE_RET);
}

uint8_t newConstant(VM* vm, Parser* parser, Compiler* compiler, Instructions* i, Constant c) {
  int constant = addConstant(vm, compiler, i, c);

  if (constant > UINT8_MAX) {
    error(parser, &parser->previous, "Senegal encountered too many constants");
    return 0;
  }

  return (uint8_t)constant;
}

void writeLoad(VM* vm, Parser* parser, Compiler* compiler, Instructions* i, Constant c) {
  int index = addConstant(vm, compiler, i, c);

  if (index == -1)
    writeByte(vm, parser, i, OPCODE_LOADN1);
  else if (index == 0)
    writeByte(vm, parser, i, OPCODE_LOAD0);
  else if (index == 1)
    writeByte(vm, parser, i, OPCODE_LOAD1);
  else if (index == 2)
    writeByte(vm, parser, i, OPCODE_LOAD2);
  else if (index == 3)
    writeByte(vm, parser, i, OPCODE_LOAD3);
  else if (index < 256) {
    writeTwoBytes(vm, parser, i, OPCODE_LOAD, (uint8_t)index);
  } else {
    writeTwoBytes(vm, parser, i, OPCODE_LLOAD, (uint8_t)(index & 0xff));
    writeTwoBytes(vm, parser, i, (uint8_t)((index >> 8) & 0xff),(uint8_t)((index >> 16) & 0xff));
  }

}

void initCompiler(VM* vm, Parser* parser,Compiler* old, Compiler* compiler, FunctionType type) {
  compiler->parent = old;
  compiler->function = NULL;
  compiler->type = type;
  compiler->localCount = 0;
  compiler->depth = 0;

  compiler->function = newFunction(vm);

  if (type != PROGRAM) {
    compiler->function->id = copyString(vm, compiler, parser->previous.start, parser->previous.length);
  }

  Local* local = &compiler->locals[compiler->localCount++];
  local->depth = 0;
  local->isCaptured = false;

  if (type != TYPE_FUNCTION) {
    local->id.start = "this";
    local->id.length = 4;
  } else {
    local->id.start = "";
    local->id.length = 0;
  }
}

GCFunction* endCompilation(VM* vm, Compiler* compiler, Parser* parser, Instructions* instructions) {
  writeRetByte(vm, compiler, parser, instructions);
  GCFunction* function = compiler->function;

#if DEBUG_PRINT_CODE
  if (!parser->hasError)
    disassembleInstructions(&compiler->function->instructions, function->id != NULL ? function->id->chars : "<global>");
#endif

  compiler = compiler->parent;
  return function;
}

GCFunction* compile(VM* vm, Compiler* compiler, const char *source) {
  Lexer lexer;
  initLexer(&lexer, source);

  Parser parser;
  initParser(&parser);

  ClassCompiler* cc = NULL;

  initCompiler(vm, &parser, NULL, compiler, PROGRAM);

  advance(&parser, &lexer);

  while (!match(&parser, &lexer, SENEGAL_EOF))
    parseDeclaration(vm, compiler, cc, &parser, &lexer, &compiler->function->instructions);

  consume(&parser, &lexer, SENEGAL_EOF, "Senegal expected end of expression");

  GCFunction* function = endCompilation(vm, compiler, &parser, &compiler->function->instructions);

  return parser.hasError ? NULL : function;
}

void markCompilerRoots(VM* vm, Compiler* compiler) {
  if (compiler == NULL)
    return;

  Compiler* compiler1 = compiler;

  while (compiler1 != NULL) {
    markGCObject(vm, (GCObject*)compiler1->function);
    compiler1 = compiler1->parent;
  }
}
