#include <stdlib.h>
#include "includes/sparser.h"
#include "includes/smemory.h"
#include "includes/scompiler.h"
#include "includes/sgcobject_utils.h"
#include "includes/stable_utils.h"

int deepestLoopStart = -1;
int deepestLoopDepth = 0;

static void markInitialized(Compiler* compiler);

ParseRule rules[] = {
    [LPAREN] = {parseGroup, parseFunctionCall, CALL},
    [RPAREN] = {NULL, NULL, NONE},
    [LBRACE] = {parseMap, NULL, NONE},
    [RBRACE] = {NULL, NULL, NONE},
    [LBRACKET] = {parseList, parseAccess, CALL},
    [RBRACKET] = {NULL, NULL, NONE},
    [COMMA] = {NULL, NULL, NONE},
    [COLON] = {NULL, NULL, NONE},
    [CASE] = {NULL, NULL, NONE},
    [DEFAULT] = {NULL, NULL, NONE},
    [SWITCH] = {NULL, NULL, NONE},
    [DOT] = {NULL, parseDot, CALL},
    [MINUS] = {parseUnary, parseBinary, TERM},
    [MINUS_EQUAL] = {NULL, NULL, TERM},
    [MINUS_MINUS] = {NULL, NULL, TERM},
    [PLUS] = {NULL, parseBinary, TERM},
    [PLUS_EQUAL] = {NULL, NULL, TERM},
    [PLUS_PLUS] = {NULL, NULL, TERM},
    [SEMI] = {NULL, NULL, NONE},
    [SLASH] = {NULL, parseBinary, FACTOR},
    [SLASH_EQUAL] = {NULL, NULL, FACTOR},
    [STAR] = {NULL, parseBinary, FACTOR},
    [STAR_EQUAL] = {NULL, NULL, FACTOR},
    [STAR_STAR] = {NULL, NULL, FACTOR},
    [BANG] = {parseUnary, NULL, NONE},
    [BANG_EQUAL] = {NULL, parseBinary, EQUALITY},
    [EQUAL] = {NULL, NULL, NONE},
    [EQUAL_EQUAL] = {NULL, parseBinary, EQUALITY},
    [GREATER] = {NULL, parseBinary, COMPARISON},
    [GREATER_GREATER] = {NULL, parseBinary, BIT_SHIFT},
    [GREATER_EQUAL] = {NULL, parseBinary, ASSIGNMENT},
    [LESSER] = {NULL, parseBinary, COMPARISON},
    [LESSER_LESSER] = {NULL, parseBinary, BIT_SHIFT},
    [LESSER_EQUAL] = {NULL, parseBinary, ASSIGNMENT},
    [ID] = {parseIdentifier, NULL, NONE},
    [STRING] = {parseString, NULL, NONE},
    [NUMBER] = {parseNumber, NULL, NONE},
    [AMP] = {NULL, parseBinary, BIT_AND},
    [AMP_AMP] = {NULL, parseAnd, PREC_AND},
    [CLASS] = {NULL, NULL, NONE},
    [CARET] = {NULL, parseBinary, BIT_XOR},
    [ELSE] = {NULL, NULL, NONE},
    [FALSE] = {parseLiteral, NULL, NONE},
    [FOR] = {NULL, NULL, NONE},
    [FUNCTION] = {NULL, NULL, NONE},
    [IF] = {NULL, NULL, NONE},
    [SENEGAL_NULL] = {parseLiteral, NULL, NONE},
    [PIPE] = {NULL, parseBinary, BIT_OR},
    [PIPE_PIPE] = {NULL, parseOr, PREC_OR},
    [RETURN] = {NULL, NULL, NONE},
    [SUPER] = {parseSuper, NULL, NONE},
    [THIS] = {parseThis, NULL, NONE},
    [TILDE] = {parseUnary, NULL, UNARY},
    [TRUE] = {parseLiteral, NULL, NONE},
    [VAR] = {NULL, NULL, NONE},
    [WHILE] = {NULL, NULL, NONE},
    [ERROR] = {NULL, NULL, NONE},
    [SENEGAL_EOF] = {NULL, NULL, NONE},
};

void initParser(Parser *parser) {
  parser->hasError = false;
  parser->panic = false;
}

static bool check(Parser* parser, TokenType type) {
  return parser->current.type == type;
}

bool match(Parser* parser, Lexer* lexer, TokenType type) {
  if (!check(parser, type))
    return false;

  advance(parser, lexer);
  return true;
}

static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

void parsePrecedence(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Precedence precedence, Instructions* i) {
  advance(parser, lexer);

  ParseFunc prefixRule = getRule(parser->previous.type)->prefix;
  if (prefixRule == NULL) {
    error(parser, &parser->previous, "Senegal expected an expression.");
    return;
  }

  bool canAssign = precedence <= ASSIGNMENT;
  prefixRule(vm, parser, compiler, cc, lexer, i, canAssign);


  while (precedence <= getRule(parser->current.type)->precedence) {
    advance(parser, lexer);
    ParseFunc infixRule = getRule(parser->previous.type)->infix;
    infixRule(vm, parser, compiler, cc, lexer, i, canAssign);
  }

  if (canAssign && match(parser, lexer, EQUAL)) {
    error(parser, &parser->previous, "Invalid assignment target.");
  }
}

static void parseExpressionStatement(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i) {
  parseExpression(vm, parser, compiler, cc, lexer, i);
  consume(parser, lexer, SEMI, "Senegal expected `;` after an expression");
  writeByte(vm, parser, i, OPCODE_POP);
}

void parseExpression(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i) {
  parsePrecedence(vm, parser, compiler, cc, lexer, ASSIGNMENT, i);
}

static void sync(Parser* parser, Lexer* lexer) {
  parser->panic = false;

  while (parser->current.type != SENEGAL_EOF) {
    if (parser->previous.type == SEMI)
      return;

    switch (parser->current.type) {
      case CLASS:
      case FUNCTION:
      case IF:
      case WHILE:
      case FOR:
      case VAR:
      case RETURN:
        return;

      default:
        break;
    }

    advance(parser, lexer);
  }
}


// == Identifiers ==
static uint8_t idConstant(VM* vm, Parser* parser, Compiler* compiler, Instructions* i, Token* id) {
  return newConstant(vm, parser, compiler, i, GC_OBJ_CONST(copyString(vm, compiler, id->start, id->length)));
}

static bool idsEqual(Token* a, Token* b) {
  if (a->length != b->length)
    return false;

  return memcmp(a->start, b->start, a->length) == 0;
}


// == Local Variables ==
static int resolveLocal(Parser* parser, Compiler* compiler, Token* name) {
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];

    if (idsEqual(name, &local->id)) {
      if (local->depth == -1)
        error(parser, &parser->previous, "Senegal tried to access a local variable in its own initializer");

      return i;
    }
  }

  return -1;
}

static void addLocal(Parser* parser, Compiler* compiler, Token id) {

  if (compiler->localCount == UINT8_COUNT) {
    error(parser, &parser->previous, "Senegal reached the local variable limit.");
    return;
  }

  Local* local = &compiler->locals[compiler->localCount++];
  local->id = id;
  local->depth = -1;
  local->isCaptured = false;
}

static void declareVariable(Parser* parser, Compiler* compiler) {
  if (compiler->depth == 0)
    return;

  Token* id = &parser->previous;

  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    if (local->depth != -1 && local->depth < compiler->depth)
      break;

    if (idsEqual(id, &local->id))
      error(parser, &parser->previous, "Senegal tried to declare an existent variable within the same scope.");

  }

  addLocal(parser, compiler, *id);
}

static uint8_t parseVariable(VM* vm, Parser* parser, Compiler* compiler, Lexer* lexer, Instructions* i, const char* msg) {
  consume(parser, lexer, ID, msg);

  declareVariable(parser, compiler);

  if (compiler->depth > 0)
    return 0;

  return idConstant(vm, parser, compiler, i, &parser->previous);
}

static void defineVariable(VM* vm, Parser* parser, Compiler* compiler, Instructions* i, uint8_t global) {
  if(compiler->depth > 0) {
    markInitialized(compiler);
    return;
  }

  writeShort(vm, parser, i, OPCODE_NEWGLOB, global);
}

static void parseVariableDeclaration(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i) {
  uint8_t global = parseVariable(vm, parser, compiler, lexer, i, "Senegal expected an identifier.");

  if (match(parser, lexer, EQUAL))
    parseExpression(vm, parser, compiler, cc, lexer, i);
  else
    writeByte(vm, parser, i, OPCODE_NULL);

  consume(parser, lexer, SEMI, "Senegal expected `;` after variable declaration.");
  defineVariable(vm, parser, compiler, i, global);
}

// == Functions ==
static void startScope(Compiler* compiler) {
  compiler->depth++;
}

static void endScope(VM* vm, Parser* parser, Compiler* compiler, Instructions* i) {
  compiler->depth--;

  uint8_t popCount = 0;

  while (compiler->depth > 0 && compiler->locals[compiler->localCount - 1].depth > compiler->depth) {

    if (compiler->locals[compiler->localCount - 1].isCaptured) {
      writeShort(vm, parser, i, OPCODE_POPN, popCount);
      popCount = 0;
      writeByte(vm, parser, i, OPCODE_CLOSEUPVAL);
    }
    else
      popCount++;

    compiler->localCount--;
  }

  writeShort(vm, parser, i, OPCODE_POPN, popCount);

}

static void parseBlock(VM* vm, Compiler* compiler, ClassCompiler* cc, Parser* parser, Lexer* lexer, Instructions* i) {
  while (!check(parser, RBRACE) && !check(parser, SENEGAL_EOF)) {
    parseDeclarationOrStatement(vm, compiler, cc, parser, lexer, i);
  }

  consume(parser, lexer, RBRACE, "Senegal expected block to be closed with `}`");
}

static void parseFunction(VM* vm, Parser* parser, Compiler* oldCompiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, FunctionType type) {
  Compiler compiler;
  initCompiler(vm, parser, oldCompiler, &compiler, type);
  startScope(&compiler);

  consume(parser, lexer, LPAREN, "Senegal expected `(` after a function name.");

  if (!check(parser, RPAREN)) {
    do {
      compiler.function->arity++;
      if (compiler.function->arity > 255) {
        error(parser, &parser->current, "Senegal functions cannot have more than 255 parameters");
      }

      uint8_t paramConstant = parseVariable(vm, parser, &compiler, lexer, &compiler.function->instructions, "Senegal expected a parameter name.");
      defineVariable(vm, parser, &compiler, &compiler.function->instructions, paramConstant);
    } while (match(parser, lexer, COMMA));
  }

  consume(parser, lexer, RPAREN, "Senegal expected `)` after function arguments.");

  if (check(parser, EQUAL_GREATER)) {
    advance(parser, lexer);

    // TODO(Calamity210): Move to a parseReturn function
    if (compiler.type == PROGRAM) {
      error(parser, &parser->previous, "Senegal can't return from a global scope.");
    }

    if (match(parser, lexer, SEMI)) {
      writeRetByte(vm, &compiler, parser, &compiler.function->instructions);
    } else {
      if (compiler.type == CONSTRUCTOR) {
        error(parser, &parser->previous, "Senegal cannot return from a constructor.");
      }

      parseExpression(vm, parser, &compiler, cc, lexer, &compiler.function->instructions);
      consume(parser, lexer, SEMI, "Senegal expected `;` after return statement.");
      writeByte(vm, parser, &compiler.function->instructions, OPCODE_RET);
    }
  } else {
    consume(parser, lexer, LBRACE, "Senegal expected `{` before function body.");
    parseBlock(vm, &compiler, cc, parser, lexer, &compiler.function->instructions);
  }

  GCFunction* function = endCompilation(vm, &compiler, parser, &compiler.function->instructions);
  writeShort(vm, parser, i, OPCODE_CLOSURE, newConstant(vm, parser, &compiler, i, GC_OBJ_CONST(function)));

  for (int j = 0; j < function->upvalueCount; j++) {
    writeShort(vm, parser, i, &compiler.upvalues[j].isLocal ? 1 : 0, compiler.upvalues[j].index);
  }
}

static int addUpvalue(Parser* parser, Compiler* compiler, uint8_t index, bool isLocal) {
  int upvalueCount = compiler->function->upvalueCount;

  for (int i = 0; i < upvalueCount; i++) {
    Upvalue* upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) {
      return i;
    }
  }

  if (upvalueCount == UINT8_COUNT) {
    error(parser, &parser->previous, "Senegal found too many upvalues in function");
    return 0;
  }

  compiler->upvalues[upvalueCount].isLocal = isLocal;
  compiler->upvalues[upvalueCount].index = index;

  return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Parser* parser, Compiler* compiler, Token* id) {
  if (compiler->parent == NULL)
    return -1;

  int local = resolveLocal(parser, compiler->parent, id);

  if (local != -1) {
    compiler->parent->locals[local].isCaptured = true;
    return addUpvalue(parser, compiler, (uint8_t)local, true);
  }

  int upvalue = resolveUpvalue(parser, compiler->parent, id);

  if (upvalue != -1) {
    return addUpvalue(parser, compiler, (uint8_t)upvalue, false);
  }

  return -1;
}

static void parseVariableAccess(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, Token name, bool canAssign) {
  uint8_t getOP, setOP = 0;

  int id = resolveLocal(parser, compiler, &name);

  if (id != -1) {

    switch (id) {
      case 0:
        getOP = OPCODE_GETLOC0;
        setOP = OPCODE_SETLOC0;
        break;

      case 1:
        getOP = OPCODE_GETLOC1;
        setOP = OPCODE_SETLOC1;
        break;

      case 2:
        getOP = OPCODE_GETLOC2;
        setOP = OPCODE_SETLOC2;
        break;

      case 3:
        getOP = OPCODE_GETLOC3;
        setOP = OPCODE_SETLOC3;
        break;

      case 4:
        getOP = OPCODE_GETLOC4;
        setOP = OPCODE_SETLOC4;
        break;

      case 5:
        getOP = OPCODE_GETLOC5;
        setOP = OPCODE_SETLOC5;
        break;
    }

    if (getOP != 0) {
      if (canAssign && match(parser, lexer, EQUAL)) {
        parseExpression(vm, parser, compiler, cc, lexer, i);
        writeByte(vm, parser, i, setOP);
      } else if (canAssign && match(parser, lexer, PLUS_PLUS)) {

        // TODO(Calamity): Correct
        writeByte(vm, parser, i, getOP);
        writeByte(vm, parser, i, OPCODE_INC);
        writeByte(vm, parser, i, i->bytes[3]);
        writeByte(vm, parser, i, setOP);
      } else {
        writeByte(vm, parser, i, getOP);
      }
      return;
    }

    getOP = OPCODE_GETLOC;
    setOP = OPCODE_SETLOC;
  } else if ((id = resolveUpvalue(parser, compiler, &name)) != -1) {
    getOP = OPCODE_GETUPVAL;
    setOP = OPCODE_SETUPVAL;
  } else {
    id = idConstant(vm, parser, compiler, i, &name);
    getOP = OPCODE_GETGLOB;
    setOP = OPCODE_SETGLOB;
  }

  if (canAssign) {
    switch (parser->current.type) {
      case EQUAL:
        advance(parser, lexer);

        parseExpression(vm, parser, compiler, cc, lexer, i);
        writeShort(vm, parser, i, setOP, (uint8_t) id);
        break;

      case PLUS_PLUS:
        advance(parser, lexer);

        writeShort(vm, parser, i, getOP, (uint8_t) id);
        writeByte(vm, parser, i, OPCODE_DUP);
        writeByte(vm, parser, i, OPCODE_INC);
        writeShort(vm, parser, i, setOP, (uint8_t) id);
        writeByte(vm, parser, i, OPCODE_POP);
        break;

      case MINUS_MINUS:
        advance(parser, lexer);

        writeShort(vm, parser, i, getOP, (uint8_t) id);
        writeByte(vm, parser, i, OPCODE_DUP);
        writeByte(vm, parser, i, OPCODE_DEC);
        writeShort(vm, parser, i, setOP, (uint8_t) id);
        writeByte(vm, parser, i, OPCODE_POP);
        break;

      case STAR_STAR:
        advance(parser, lexer);

        if (!check(parser, NUMBER))
          error(parser, &parser->previous, "Senegal can only raise to the power of a number.");

        writeShort(vm, parser, i, getOP, (uint8_t) id);
        parseExpression(vm, parser, compiler, cc, lexer, i);
        writeByte(vm, parser, i, OPCODE_POW);
        writeShort(vm, parser, i, setOP, (uint8_t) id);
        break;

      case PLUS_EQUAL:
        advance(parser, lexer);

        writeShort(vm, parser, i, getOP, (uint8_t) id);
        parseExpression(vm, parser, compiler, cc, lexer, i);
        writeByte(vm, parser, i, OPCODE_ADD);
        writeShort(vm, parser, i, setOP, (uint8_t) id);
        break;

      case MINUS_EQUAL:
        advance(parser, lexer);

        writeShort(vm, parser, i, getOP, (uint8_t) id);
        parseExpression(vm, parser, compiler, cc, lexer, i);
        writeByte(vm, parser, i, OPCODE_SUB);
        writeShort(vm, parser, i, setOP, (uint8_t) id);
        break;

      case STAR_EQUAL:
        advance(parser, lexer);

        writeShort(vm, parser, i, getOP, (uint8_t) id);
        parseExpression(vm, parser, compiler, cc, lexer, i);
        writeByte(vm, parser, i, OPCODE_MUL);
        writeShort(vm, parser, i, setOP, (uint8_t) id);
        break;


      case SLASH_EQUAL:
        advance(parser, lexer);

        writeShort(vm, parser, i, getOP, (uint8_t) id);
        parseExpression(vm, parser, compiler, cc, lexer, i);
        writeByte(vm, parser, i, OPCODE_DIV);
        writeShort(vm, parser, i, setOP, (uint8_t) id);
        break;

      default:
        writeShort(vm, parser, i, getOP, (uint8_t) id);
        break;
    }
  } else {
    writeShort(vm, parser, i, getOP, (uint8_t) id);
  }
}

static void parseFunctionDeclaration(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i) {
  uint8_t global = parseVariable(vm, parser, compiler, lexer, i, "Senegal expected an id after `function`");

  markInitialized(compiler);

  parseFunction(vm, parser, compiler, cc, lexer, i, TYPE_FUNCTION);
  defineVariable(vm, parser, compiler, i, global);
}

static void parseMethodDeclaration(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i) {
  consume(parser, lexer, ID, "Senegal expected a method name");

  uint8_t constant = idConstant(vm, parser, compiler, i, &parser->previous);

  FunctionType type = METHOD;

  if (strncmp(parser->previous.start, cc->id.start, cc->id.length) == 0)
    type = CONSTRUCTOR;

  parseFunction(vm, parser, compiler, cc, lexer, i, type);
  writeShort(vm, parser, i, OPCODE_METHOD, constant);
}

static uint8_t argumentList(VM* vm, Parser* parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i) {
  uint8_t argCount = 0;
  if (!check(parser, RPAREN)) {
    do {
      parseExpression(vm, parser, compiler, cc, lexer, i);

      if (argCount == 255) {
        error(parser, &parser->previous, "Senegal cannot have over 255 function call arguments yet");
      }

      argCount++;
    } while (match(parser, lexer, COMMA));
  }

  consume(parser, lexer, RPAREN, "Senegal expected `)` after function call arguments.");
  return argCount;
}


// == Classes ==
static Token syntheticToken(const char* text) {
  Token token;
  token.start = text;
  token.length = (int)strlen(text);
  return token;
}

static void parseClassDeclaration(VM* vm, Compiler* compiler, ClassCompiler* cc, Parser* parser, Lexer* lexer, Instructions* i, bool isFinal, bool isStrict) {
  consume(parser, lexer, ID, "Senegal expected an identifier after `class` keyword");

  Token classId = parser->previous;
  uint8_t idConst = idConstant(vm, parser, compiler, i, &parser->previous);

  declareVariable(parser, compiler);

  if (isFinal)
    writeShort(vm, parser, i, OPCODE_NEWFINALCLASS, idConst);
  else if (isStrict)
    writeShort(vm, parser, i, OPCODE_NEWSTRICTCLASS, idConst);
  else
    writeShort(vm, parser, i, OPCODE_NEWCLASS, idConst);

  defineVariable(vm, parser, compiler, i, idConst);

  ClassCompiler newCC;
  newCC.id = parser->previous;
  newCC.hasSuper = false;
  newCC.parent = cc;
  cc = &newCC;

  if (match(parser, lexer, EXTENDS)) {
    consume(parser, lexer, ID, "Senegal expected a superclass after `extends`");
    parseVariableAccess(vm, parser, compiler, cc, lexer, i, parser->previous, false);

    if (idsEqual(&classId, &parser->previous)) {
      error(parser, &parser->previous, "Senegal classes cannot inherit themselves.");
    }

    startScope(compiler);
    addLocal(parser, compiler, syntheticToken("super"));
    defineVariable(vm, parser, compiler, i, 0);

    parseVariableAccess(vm, parser, compiler, cc, lexer, i, classId, false);
    writeByte(vm, parser, i, OPCODE_INHERIT);
    newCC.hasSuper = true;
  }

  parseVariableAccess(vm, parser, compiler, cc, lexer, i, classId, false);

  consume(parser, lexer, LBRACE, "Senegal expected `{` after class identifier");

  while (!check(parser, RBRACE) && !check(parser, SENEGAL_EOF)) {
    if (match(parser, lexer, FUNCTION)
    || (check(parser, ID) && strncmp(classId.start, parser->current.start, classId.length) == 0))
      parseMethodDeclaration(vm, parser, compiler, cc, lexer, i);
  }

  consume(parser, lexer, RBRACE, "Senegal expected `}` after class body");
  writeByte(vm, parser, i, OPCODE_POP);

  if (cc->hasSuper) {
    endScope(vm, parser, compiler, i);
  }

  cc = cc->parent;
}

void parseDeclaration(VM* vm, Compiler* compiler, ClassCompiler* cc, Parser* parser, Lexer* lexer, Instructions* i) {

  bool isFinal = match(parser, lexer, FINAL);
  bool isStrict = false;

  if (!isFinal)
    isStrict = match(parser, lexer, STRICT);

  if (match(parser, lexer, CLASS))
    parseClassDeclaration(vm, compiler, cc, parser, lexer, i, isFinal, isStrict);
  else if (match(parser, lexer, FUNCTION))
    parseFunctionDeclaration(vm, parser, compiler, cc, lexer, i);
  else if (match(parser, lexer, VAR))
    parseVariableDeclaration(vm, parser, compiler, cc, lexer, i);
  else
    advance(parser, lexer);

  if (parser->panic)
    sync(parser, lexer);
}

void parseDeclarationOrStatement(VM* vm, Compiler* compiler, ClassCompiler* cc, Parser* parser, Lexer* lexer, Instructions* i) {

  bool isFinal = match(parser, lexer, FINAL);
  bool isStrict = false;

  if (!isFinal)
    isStrict = match(parser, lexer, STRICT);

  if (match(parser, lexer, CLASS))
    parseClassDeclaration(vm, compiler, cc, parser, lexer, i, isFinal, isStrict);
  else if (match(parser, lexer, FUNCTION))
    parseFunctionDeclaration(vm, parser, compiler, cc, lexer, i);
  else if (match(parser, lexer, VAR))
    parseVariableDeclaration(vm, parser, compiler, cc, lexer, i);
  else
    parseStatement(vm, compiler, cc, parser, lexer, i);

  if (parser->panic)
    sync(parser, lexer);
}

static int writeJMP(VM* vm, Parser* parser, Instructions* i, uint8_t opcode) {
  writeByte(vm, parser,i, opcode);
  writeShort(vm, parser, i, 0xff, 0xff);

  return i->bytesCount - 2;
}

static void patchJMP(Parser* parser, Instructions* i, int offset) {
  int jump = i->bytesCount - offset - 2;

  if (jump > UINT16_MAX) {
    error(parser, &parser->previous, "Senegal attempted to jump beyond 16 bytes");
  }


  i->bytes[offset] = (jump >> 8) & 0xff;
  i->bytes[offset + 1] = jump & 0xff;
}

// == Tokens ==
void parseAccess(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  parseExpression(vm, parser, compiler, cc, lexer, i);
  consume(parser, lexer, RBRACKET, "Senegal expected access to be closed by `]`");

  if (match(parser, lexer, EQUAL)) {
    parseExpression(vm, parser, compiler, cc, lexer, i);
    writeByte(vm, parser, i, OPCODE_SETACCESS);
    return;
  }

  writeByte(vm, parser, i, OPCODE_ACCESS);
}

void parseAnd(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  int endJMP = writeJMP(vm, parser, i, OPCODE_JF);

  writeByte(vm, parser, i, OPCODE_POP);
  parsePrecedence(vm, parser, compiler, cc, lexer, PREC_AND, i);

  patchJMP(parser, i, endJMP);
}

void parseBinary(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  TokenType op = parser->previous.type;
  ParseRule* rule = getRule(op);
  parsePrecedence(vm, parser, compiler, cc, lexer, (Precedence)(rule->precedence + 1), i);

  switch (op) {

    case AMP:
      writeByte(vm, parser, i, OPCODE_AND);
      break;

    case PIPE:
      writeByte(vm, parser, i, OPCODE_OR);
      break;

    case CARET:
      writeByte(vm, parser, i, OPCODE_XOR);
      break;

    case LESSER_LESSER:
      writeByte(vm, parser, i, OPCODE_LSHIFT);
      break;

    case GREATER_GREATER:
      writeByte(vm, parser, i, OPCODE_RSHIFT);
      break;

    case PLUS:
      writeByte(vm, parser, i, OPCODE_ADD);
      break;

    case MINUS:
      writeByte(vm, parser, i, OPCODE_SUB);
      break;

    case STAR:
      if (AS_NUMBER(vm->stackTop[0]) == 1) {
        pop(vm);
        break;
      }

      writeByte(vm, parser, i, OPCODE_MUL);
      break;

    case SLASH:


      writeByte(vm, parser, i, OPCODE_DIV);
      break;

    case BANG_EQUAL:
      writeByte(vm, parser, i, OPCODE_NOTEQ);
      break;

    case EQUAL_EQUAL:
      writeByte(vm, parser, i, OPCODE_EQUAL);
      break;

    case GREATER:
      writeByte(vm, parser, i, OPCODE_GREATER);
      break;

    case LESSER:
      writeByte(vm, parser, i, OPCODE_LESSER);
      break;

    case GREATER_EQUAL:
      writeByte(vm, parser, i, OPCODE_GE);
      break;

    case LESSER_EQUAL:
      writeByte(vm, parser, i, OPCODE_LE);
      break;

    default:
      return;
  }
}

void parseDot(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  consume(parser, lexer, ID, "Senegal expected a property or method after `.`");
  uint8_t id = idConstant(vm, parser, compiler, i, &parser->previous);

  if (canAssign && match(parser, lexer, EQUAL)) {

    parseExpression(vm, parser, compiler, cc, lexer, i);
    writeShort(vm, parser, i, OPCODE_SETFIELD, id);

  } else if (match(parser, lexer, LPAREN)) {

    uint8_t arity = argumentList(vm, parser, compiler, cc, lexer, i);
    writeShort(vm, parser, i, OPCODE_INVOKE, id);
    writeByte(vm, parser, i, arity);

  } else {
    writeShort(vm, parser, i, OPCODE_GETFIELD, id);
  }
}

void parseFunctionCall(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  uint8_t arity = argumentList(vm, parser, compiler, cc, lexer, i);

  if (arity == 0)
    writeByte(vm, parser, i, OPCODE_CALL0);
  else if (arity == 1)
    writeByte(vm, parser, i, OPCODE_CALL1);
  else if (arity == 2)
    writeByte(vm, parser, i, OPCODE_CALL2);
  else if (arity == 3)
    writeByte(vm, parser, i, OPCODE_CALL3);
  else if (arity == 4)
    writeByte(vm, parser, i, OPCODE_CALL4);
  else if (arity == 5)
    writeByte(vm, parser, i, OPCODE_CALL5);
  else if (arity == 6)
    writeByte(vm, parser, i, OPCODE_CALL6);
  else if (arity == 7)
    writeByte(vm, parser, i, OPCODE_CALL7);
  else if (arity == 8)
    writeByte(vm, parser, i, OPCODE_CALL8);
  else
    writeShort(vm, parser, i, OPCODE_CALL, arity);
}

void parseGroup(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  parseExpression(vm, parser, compiler, cc, lexer, i);
  consume(parser, lexer, RPAREN, "Senegal expected `)` after a grouped expression");
}

void parseIdentifier(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  parseVariableAccess(vm, parser, compiler, cc, lexer, i, parser->previous, canAssign);
}

void parseLiteral(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer *lexer, Instructions *i, bool canAssign) {
  switch (parser->previous.type) {
    case TRUE:
      writeByte(vm, parser, i, OPCODE_TRUE);
      break;

    case FALSE:
      writeByte(vm, parser, i, OPCODE_FALSE);
      break;

    case SENEGAL_NULL:
      writeByte(vm, parser, i, OPCODE_NULL);
      break;

    default:
      return;
  }
}

void parseList(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {

  if (match(parser, lexer, RBRACKET))
    writeLoad(vm, parser, compiler, i, GC_OBJ_CONST(newList(vm, 0)));

  uint8_t entryCount = 0;
  do {
    parseExpression(vm, parser, compiler, cc, lexer, i);
    entryCount++;
  } while (match(parser, lexer, COMMA));

  consume(parser, lexer, RBRACKET, "Senegal expected list to be closed with `]`");

  writeShort(vm, parser, i, OPCODE_NEWLIST, entryCount);
}

void parseMap(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {

  if (match(parser, lexer, RBRACE))
    writeLoad(vm, parser, compiler, i, GC_OBJ_CONST(newMap(vm)));

  uint8_t entryCount = 0;
  do {
    parseExpression(vm, parser, compiler, cc, lexer, i);
    if (!match(parser, lexer, COLON))
      error(parser, &parser->previous, "Senegal expected `:` after map key");
    parseExpression(vm, parser, compiler, cc, lexer, i);

    entryCount++;
  } while (match(parser, lexer, COMMA));

  consume(parser, lexer, RBRACE, "Senegal expected map to be closed with `}`");

  writeShort(vm, parser, i, OPCODE_NEWMAP, entryCount);
}

void parseNumber(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  writeLoad(vm, parser, compiler, i, NUM_CONST(strtod(parser->previous.start, NULL)));
}

void parseOr(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  int elseJMP = writeJMP(vm, parser, i, OPCODE_JF);
  int endJMP = writeJMP(vm, parser, i, OPCODE_JMP);

  patchJMP(parser, i, elseJMP);
  writeByte(vm, parser, i, OPCODE_POP);

  parsePrecedence(vm, parser, compiler, cc, lexer, PREC_OR, i);
  patchJMP(parser, i, endJMP);
}

void parseString(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  writeLoad(vm, parser, compiler, i, GC_OBJ_CONST(
      copyString(vm, compiler, parser->previous.start + 1, parser->previous.length - 2)
  ));
}

void parseSuper(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  if (cc == NULL) {
    error(parser, &parser->previous, "Senegal cannot access `super` outside of a class.");
  } else if (!cc->hasSuper) {
    error(parser, &parser->previous, "Senegal cannot access `super` without a superclass");
  }

  consume(parser, lexer, DOT, "Senegal expected `.` after `super`");
  consume(parser, lexer, ID, "A superclass field identifier");

  uint8_t id = idConstant(vm, parser, compiler, i, &parser->previous);

  parseVariableAccess(vm, parser, compiler, cc, lexer, i, syntheticToken("this"), false);

  if (match(parser, lexer, LPAREN)) {
    uint8_t arity = argumentList(vm, parser,compiler, cc, lexer, i);
    parseVariableAccess(vm, parser, compiler, cc, lexer, i, syntheticToken("super"), false);
    writeShort(vm, parser, i, OPCODE_SUPERINVOKE, id);
    writeByte(vm, parser, i, arity);
  } else {
    parseVariableAccess(vm, parser, compiler, cc, lexer, i, syntheticToken("super"), false);
    writeShort(vm, parser, i, OPCODE_GETSUPER, id);
  }
}

void parseThis(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  if (cc == NULL) {
    error(parser, &parser->previous, "Senegal could not gain access to `this` in a global scope");
    return;
  }

  parseVariableAccess(vm, parser, compiler, cc, lexer, i, parser->previous, false);
}

void parseUnary(VM* vm, Parser *parser, Compiler* compiler, ClassCompiler* cc, Lexer* lexer, Instructions* i, bool canAssign) {
  TokenType op = parser->previous.type;

  parsePrecedence(vm, parser, compiler, cc, lexer, UNARY, i);

  switch (op) {
    case BANG:
      writeByte(vm, parser, i, OPCODE_NOT);
      break;

    case MINUS:
      writeByte(vm, parser, i, OPCODE_NEG);
      break;

    case TILDE:
      writeByte(vm, parser, i, OPCODE_BITNOT);
      break;

    default:
      return;
  }
}

static void writeLoop(VM* vm, Parser* parser, Instructions* i, int start) {
  writeByte(vm, parser, i, OPCODE_LOOP);

  int offset = i->bytesCount - start + 2;

  if (offset > UINT16_MAX)
    error(parser, &parser->previous, "Senegal attempted to jump beyond 16 bytes");

  writeByte(vm, parser, i, (offset >> 8) & 0xff);
  writeByte(vm, parser, i, offset & 0xff);
}

void parseStatement(VM* vm, Compiler* compiler, ClassCompiler* cc, Parser* parser, Lexer* lexer, Instructions* i) {

  switch (parser->current.type) {

    case BREAK: {
      advance(parser, lexer);

      if (deepestLoopStart == -1) {
        error(parser, &parser->previous, "Senegal is unable to `break` outside loops.");
      }

      consume(parser, lexer, SEMI, "Senegal expected `;` after continue");

      for (int j = compiler->localCount - 1; j >= 0 && compiler->locals[j].depth > deepestLoopDepth; j--)
        writeByte(vm, parser, i, OPCODE_POP);


      writeJMP(vm, parser, i, OPCODE_BREAK);

      break;
    }

    case CONTINUE: {
      advance(parser, lexer);

      if (deepestLoopStart == -1) {
        error(parser, &parser->previous, "Senegal is unable to `continue` outside loops.");
      }

      consume(parser, lexer, SEMI, "Senegal expected `;` after continue");

      for (int j = compiler->localCount - 1; j >= 0 && compiler->locals[j].depth > deepestLoopDepth; j--)
        writeByte(vm, parser, i, OPCODE_POP);

      writeLoop(vm, parser, i, deepestLoopStart);
      break;
    }

    case LBRACE:
      advance(parser, lexer);
      startScope(compiler);
      parseBlock(vm, compiler, cc, parser, lexer, i);
      endScope(vm, parser, compiler, i);
      break;

    case IF: {
      advance(parser, lexer);
      consume(parser, lexer, LPAREN, "Senegal expected if condition to be enclosed in parenthesis.");
      parseExpression(vm, parser, compiler, cc, lexer, i);
      consume(parser, lexer, RPAREN, "Senegal expected if condition to be followed by a closing parenthesis.");

      int thenJmp = writeJMP(vm, parser, i, OPCODE_JF);
      writeByte(vm, parser, i, OPCODE_POP);

      parseStatement(vm, compiler, cc, parser, lexer, i);

      int elseJMP = writeJMP(vm, parser, i, OPCODE_JMP);

      patchJMP(parser, i, thenJmp);
      writeByte(vm, parser, i, OPCODE_POP);

      if (match(parser, lexer, ELSE))
        parseStatement(vm, compiler, cc, parser, lexer, i);

      patchJMP(parser, i, elseJMP);
      break;
    }

    case RETURN: {
      // TODO(Calamity210): Move to a parseReturn function
      advance(parser, lexer);

      if (compiler->type == PROGRAM) {
        error(parser, &parser->previous, "Senegal can't return from a global scope.");
      }

      if (match(parser, lexer, SEMI)) {
        writeRetByte(vm, compiler, parser, i);
      } else {
        if (compiler->type == CONSTRUCTOR) {
          error(parser, &parser->previous, "Senegal cannot return from a constructor.");
        }

        parseExpression(vm, parser, compiler, cc, lexer, i);
        consume(parser, lexer, SEMI, "Senegal expected `;` after return statement.");
        writeByte(vm, parser, i, OPCODE_RET);
      }

      break;
    }

    case SWITCH: {
      // TODO(Calamity210): Implement switch using a jump table

      break;
    }

    case WHILE: {
      advance(parser, lexer);

      int enclosingLoopStart = deepestLoopStart;
      int enclosingLoopDepth = deepestLoopDepth;
      deepestLoopStart = i->bytesCount;
      deepestLoopDepth = compiler->depth;

      consume(parser, lexer, LPAREN, "Senegal expected while condition to be enclosed in parenthesis.");
      parseExpression(vm, parser, compiler, cc, lexer, i);
      consume(parser, lexer, RPAREN, "Senegal expected while condition to be followed by a closing parenthesis.");

      int breakJMP = writeJMP(vm, parser, i, OPCODE_JF);

      writeByte(vm, parser, i, OPCODE_POP);

      int j = i->bytesCount;

      parseStatement(vm, compiler, cc, parser, lexer, i);

      writeLoop(vm, parser, i, deepestLoopStart);

      patchJMP(parser, i, breakJMP);
      writeByte(vm, parser, i, OPCODE_POP);

      while (j < i->bytesCount) {
        if (i->bytes[j] == OPCODE_BREAK) {
          i->bytes[j] = OPCODE_JMP;
          patchJMP(parser, i, j + 1);

          j += 3;
        }
        j++;
      }

      deepestLoopStart = enclosingLoopStart;
      deepestLoopDepth = enclosingLoopDepth;
      break;
    }

    case FOR: {
      advance(parser, lexer);

      startScope(compiler);

      consume(parser, lexer, LPAREN, "Senegal expected for statement to be enclosed in parenthesis.");

      if (match(parser, lexer, VAR)) {
        parseVariableDeclaration(vm, parser, compiler, cc, lexer, i);
      } else if (match(parser, lexer, SEMI)) {

      } else {
        parseExpressionStatement(vm, parser, compiler, cc, lexer, i);
      }

      int enclosingLoopStart = deepestLoopStart;
      int enclosingLoopDepth = deepestLoopDepth;
      deepestLoopStart = i->bytesCount;
      deepestLoopDepth = compiler->depth;

      int exitJMP = -1;

      if (!match(parser, lexer, SEMI)) {
        parseExpression(vm, parser, compiler, cc, lexer, i);
        consume(parser, lexer, SEMI, "Senegal expected a semi colon after a for loops condition statement.");


        exitJMP = writeJMP(vm, parser, i, OPCODE_JF);
        writeByte(vm, parser, i, OPCODE_POP);
      }

      if (!match(parser, lexer, RPAREN)) {
        int bodyJMP = writeJMP(vm, parser, i, OPCODE_JMP);

        int incStart = i->bytesCount;
        parseExpression(vm, parser, compiler, cc, lexer, i);
        writeByte(vm, parser, i, OPCODE_POP);

        consume(parser, lexer, RPAREN, "Senegal expected for statement to be followed by a closing parenthesis.");

        writeLoop(vm, parser, i, deepestLoopStart);
        deepestLoopStart = incStart;
        patchJMP(parser, i, bodyJMP);
      }

      int j = i->bytesCount;

      parseStatement(vm, compiler, cc, parser, lexer, i);

      writeLoop(vm, parser, i, deepestLoopStart);

      if (exitJMP != -1) {
        patchJMP(parser, i, exitJMP);
        writeByte(vm, parser, i, OPCODE_POP);
      }

      while (j < i->bytesCount) {
        if (i->bytes[j] == OPCODE_BREAK) {
          i->bytes[j] = OPCODE_JMP;
          patchJMP(parser, i, j + 1);

          j += 3;
        }
        j++;
      }

      deepestLoopStart = enclosingLoopStart;
      deepestLoopDepth = enclosingLoopDepth;

      endScope(vm, parser, compiler, i);
      break;
    }

    default:
      parseExpressionStatement(vm, parser, compiler, cc, lexer, i);
      break;
  }
}

// == GCObjects ==

void markInitialized(Compiler *compiler) {
  if (compiler->depth == 0)
    return;

  compiler->locals[compiler->localCount - 1].depth = compiler->depth;
}

static GCString* allocateString(VM* vm, char* chars, int length, uint32_t hash) {
  GCString* string = ALLOCATE_GC_OBJ(vm, GCString, GC_STRING);
  string->length = length;
  string->chars = chars;
  string->hash = hash;

  push(vm, GC_OBJ_CONST(string));

  tableInsert(vm, &vm->strings, string, NULL_CONST);

  pop(vm);

  return string;
}

// TODO(calamity): use this when we allow keys of other types
static uint32_t hashDouble(double constant) {
  union BitCast {
      double constant;
      uint32_t ints[2];
  };

  union BitCast cast;
  cast.constant = constant + 1.0;
  return cast.ints[0] + cast.ints[1];
}

uint32_t hashConstant(Constant c) {
  if (IS_BOOL(c))
    return AS_BOOL(c) ? 3 : 5;

  if (IS_NULL(c))
    return 7;

  if (IS_NUMBER(c))
    return hashDouble(AS_NUMBER(c));

  if (IS_GC_OBJ(c))
    return AS_STRING(c)->hash;

  return 0;
}

static uint32_t hashString(const char* string, int length) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= string[i];
    hash *= 16777619;
  }

  return hash;
}

GCString* copyString(VM* vm, Compiler* compiler, const char *chars, int length) {
  uint32_t hash = hashString(chars, length);
  GCString* internalized = tableFindString(&vm->strings, chars, length, hash);

  if (internalized != NULL) {
    return internalized;
  }

  char* heapChars = ALLOCATE(vm, compiler, char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';

  return allocateString(vm, heapChars, length, hash);
}

GCString* getString(VM* vm, char *chars, int length) {
  uint32_t hash = hashString(chars, length);
  GCString* internalized = tableFindString(&vm->strings, chars, length, hash);

  if (internalized != NULL) {
    FREE_ARRAY(vm, NULL, char, chars, length + 1);
    return internalized;
  }

  return allocateString(vm, chars, length, hash);
}