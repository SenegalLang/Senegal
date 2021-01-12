#include <stdio.h>
#include <libgen.h>
#include <string.h>

#include "includes/sutils.h"
#include "includes/scompiler.h"
#include "includes/sinstruction_utils.h"
#include "includes/smemory.h"
#include "includes/stable_utils.h"
#include "includes/sloadclib.h"

#if DEBUG_PRINT_CODE
#include "includes/sdebug.h"
#endif

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

// Prints a parsing/syntax error
//
// Senegal attempts to find as many errors as one pass allows it to
// which is why you'll notice the program generally doesnt exit after calling error.
void error(Parser* parser, Token* token, const char* message) {
  if (parser->panic)
    return;

  parser->panic = true;

  fprintf(stderr, "< Error on line %d >", token->line);

  fprintf(stderr, " `%s`\n", message);
  fprintf(stderr, "\t↳ in: %s\n", parser->currentFile);

  fprintf(stderr, "\t\t↳ at: %.*s\n", token->length, token->start);

  parser->hasError = true;
}

// Fetches the next token from the lexer showing an error wherever an ERROR token is encountered
void advance(Parser* parser, Lexer* lexer) {
  parser->previous = parser->current;

  for (;;) {
    parser->current = getNextToken(lexer);

    if (parser->current.type != SENEGAL_ERROR)
      break;

    error(parser, &parser->current, parser->current.start);
  }
}

// Consumes the current token and advances to the next if the current tokens type matches the given type,
// an error is shown if types dont match
void consume(Parser* parser, Lexer* lexer, SenegalTokenType type, const char* message) {
  if (parser->current.type == type) {
    advance(parser, lexer);
    return;
  }

  error(parser, &parser->current, message);
}

// Writes a byte to the VM's instructions
void writeByte(VM* vm, Parser* parser, Instructions* instructions, uint8_t byte) {
  writeInstructions(vm, instructions, byte, parser->previous.line);
}

// Writes 16-bits to the VM's instructions
void writeShort(VM* vm, Parser* p, Instructions* i, uint8_t byte1, uint8_t byte2) {
  writeByte(vm, p, i, byte1);
  writeByte(vm, p, i, byte2);
}

// Write an OPCODE_RET opcode to the VM's instructions
void writeRetByte(VM* vm, Compiler* compiler, Parser* parser, Instructions* instructions) {
  if (compiler->type == CONSTRUCTOR)
    writeShort(vm, parser, instructions, OPCODE_GETLOC, 0);
  else
    writeByte(vm, parser, instructions, OPCODE_NULL);

  writeByte(vm, parser, instructions, OPCODE_RET);
}

// Writes a new constant to the constant pool
uint8_t newConstant(VM* vm, Parser* parser, Compiler* compiler, Instructions* i, Constant c) {
  int constant = addConstant(vm, compiler, i, c);

  if (constant > UINT8_MAX) {
    error(parser, &parser->previous, "Senegal encountered too many constants");
    return 0;
  }

  return (uint8_t)constant;
}

// Writes an OPCODE_LOAD opcode to the VM's instructions
void writeLoad(VM* vm, Parser* parser, Compiler* compiler, Instructions* i, Constant c) {
  int index = addConstant(vm, compiler, i, c);

  if (IS_NUMBER(c) && AS_NUMBER(c) == 100) {
    printf("hi");
  }

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
    writeShort(vm, parser, i, OPCODE_LOAD, (uint8_t) index);
  } else {
    writeShort(vm, parser, i, OPCODE_LLOAD, (uint8_t) (index & 0xff));
    writeShort(vm, parser, i, (uint8_t) ((index >> 8) & 0xff), (uint8_t) ((index >> 16) & 0xff));
  }

}

// Initializes a new compiler with default values
void initCompiler(VM* vm, Parser* parser,Compiler* old, Compiler* compiler, FunctionType type, bool getPath) {

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

// Stops a compiler compiling some form of a function or instructions by writing an OPCODE_RET opcode,
// setting the compiler to its parent and returning the compiler's function.
GCFunction* endCompilation(VM* vm, Compiler* compiler, Parser* parser, Instructions* instructions) {
  writeRetByte(vm, compiler, parser, instructions);
  GCFunction* function = compiler->function;

#if DEBUG_PRINT_CODE
  if (!parser->hasError)
    disassembleInstructions(&compiler->function->instructions, function->id != NULL ? function->id->chars : "global");
#endif

  compiler = compiler->parent;
  return function;
}

// TODO: Correct and write tests for this
static void handleCImport(VM* vm, char* importSource) {
  Constant function = sysSym(vm, sysLoad(importSource, false), "initLib");
  AS_NATIVE(function)(vm, 0, vm->coroutine->stackTop);
}

static void handleImport(VM* vm, Compiler* compiler, char* senegalPath, char* dir, char* importSource) {
  // Core library
  if (importSource[3] == ':') { // We make the assumption that a regular path would not contain `:`
    Constant constant;

    Constant c;
    if (tableGetEntry(&vm->imports, GC_OBJ_CONST(copyString(vm, NULL, importSource, strlen(importSource))), &c))
      return;

    if (!tableGetEntry(&vm->corePaths, GC_OBJ_CONST(copyString(vm, compiler, importSource, strlen(importSource))), &constant))
      fprintf(stderr, "`%s` is not a core senegal library", importSource);

    // TODO: Test more thoroughly
    if (IS_STRING(constant)) {
      char* path = concat(senegalPath, AS_CSTRING(constant));

      // Save the directory path as libs/
      char* tempDir = concat(senegalPath, "libs/");
      interpret(vm, path, readFileWithPath(path), senegalPath, tempDir);
    } else {
      AS_NATIVE(constant)(vm, 0, vm->coroutine->stackTop);
    }

  } else {
    char* tempDir = strdup(dir);

    // +2 for separator and terminator
    char* filePath = (char*)malloc(strlen(dir) + strlen(importSource) + 2);
    filePath[0] = '\0';

    strcat(filePath, tempDir);
    strcat(filePath, PATH_SEPARATOR);
    strcat(filePath, importSource);

    // PATH_MAX + 1
    char absolutePath[4097];
    realpath(filePath, absolutePath);

    Constant c;
    Constant importString = GC_OBJ_CONST(copyString(vm, NULL, absolutePath, strlen(absolutePath)));
    if (!tableGetEntry(&vm->imports, importString, &c)) {
      char* fpDup = strdup(filePath);
      char* newDir = dirname(fpDup);

      if (interpret(vm, filePath, readFileWithPath(filePath), senegalPath, newDir) == OK)
        tableInsert(vm, &vm->imports, importString, NULL_CONST);
    }
  }
}

GCFunction* compile(VM* vm, Compiler* compiler, char* file, char *source, char* senegalPath, char* dir) {
  Lexer lexer;
  initLexer(&lexer, source);

  Parser parser;
  initParser(&parser, file);

  ClassCompiler* cc = NULL;

  initCompiler(vm, &parser, NULL, compiler, PROGRAM, true);

  advance(&parser, &lexer);

  while (match(&parser, &lexer, SENEGAL_CIMPORT)) {
    consume(&parser, &lexer, SENEGAL_STRING, "Senegal expected a path to import");
    char* importSource = copyString(vm, compiler, parser.previous.start + 1, parser.previous.length - 2)->chars;
    importSource = concat(concat(dir, "/"), importSource);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    importSource = concat(importSource, ".dll");
#elif __APPLE__
    importSource = concat(importSource, ".dylib");
#else
    importSource = concat(importSource, ".so");
#endif

    handleCImport(vm, importSource);
  }

  while (match(&parser, &lexer, SENEGAL_IMPORT)) {
    consume(&parser, &lexer, SENEGAL_STRING, "Senegal expected a path to import");
    char* importSource = copyString(vm, compiler, parser.previous.start + 1, parser.previous.length - 2)->chars;

   handleImport(vm, compiler, senegalPath, dir, importSource);
  }

  while (!match(&parser, &lexer, SENEGAL_EOF))
    parseDeclarationOrStatement(vm, compiler, cc, &parser, &lexer, &compiler->function->instructions);

  consume(&parser, &lexer, SENEGAL_EOF, "Senegal expected end of expression");

  GCFunction* function = endCompilation(vm, compiler, &parser, &compiler->function->instructions);

  return parser.hasError ? NULL : function;
}

void markCompilerRoots(VM* vm, Compiler* compiler) {
  if (!compiler)
    return;

  Compiler* compiler1 = compiler;

  // Walk the parents to mark them as well.
  while (compiler1 != NULL) {
    markGCObject(vm, (GCObject*)compiler1->function);
    compiler1 = compiler1->parent;
  }
}
