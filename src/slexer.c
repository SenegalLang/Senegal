#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "includes/sutils.h"
#include "includes/slexer.h"

void initLexer(Lexer* lexer, char *source) {
  lexer->start = source;
  lexer->current = source;
  lexer->line = 1;
}

static bool isAtEnd(Lexer* lexer) {
  return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
  lexer->current++;
  return lexer->current[-1];
}

static char peek(Lexer* lexer) {
  return *lexer->current;
}

static char peekNext(Lexer* lexer) {
  if (isAtEnd(lexer))
    return '\0';

  return lexer->current[1];
}

static bool match(Lexer* lexer, char expected) {
  if (isAtEnd(lexer)) return false;
  if (*lexer->current != expected) return false;

  lexer->current++;
  return true;
}

static Token newToken(Lexer* lexer, TokenType type) {
  Token token;
  token.type = type;
  token.start = lexer->start;
  token.length = (int)(lexer->current - lexer->start);
  token.line = lexer->line;

  return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
  Token token;
  token.type = ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = lexer->line;

  return token;
}

static void removeNextCharFromSource(Lexer* lexer, int index) {
  int length = (int)(lexer->current - lexer->start);
  removeCharFromIndex(lexer->start, lexer->start, length + index);
  lexer->current = lexer->start;
  lexer->current += length;
}

static void skipWhitespaceAndComment(Lexer* lexer) {
  for (;;) {
    char c = peek(lexer);
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance(lexer);
        break;

      case '\n':
        lexer->line++;
        advance(lexer);
        break;

      case '/': {
        char n = peekNext(lexer);

        if (n == '/') {
          while (peek(lexer) != '\n' && !isAtEnd(lexer))
            advance(lexer);
          break;
        }

        else if (n == '*') {
          while (peek(lexer) != '*' && peekNext(lexer) != '/' && !isAtEnd(lexer))
            advance(lexer);
          break;
        }

        return;
      }

      default:
        return;
    }
  }
}

static TokenType collectKeyword(Lexer* lexer, int start, int length, const char* rest, TokenType type) {
  if (lexer->current - lexer->start == start + length
      && memcmp(lexer->start + start, rest, length) == 0)
    return type;

  return ID;
}

static TokenType idToken(Lexer* lexer) {

  switch (lexer->start[0]) {
    case 'a':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 's':
            return collectKeyword(lexer, 2, 3, "ync", ASYNC);

          case 'w':
            return collectKeyword(lexer, 2, 3, "ait", AWAIT);
        }
      }
      break;

    case 'b':
      return collectKeyword(lexer, 1, 4, "reak", BREAK);

    case 'c':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'a':
            return collectKeyword(lexer, 2, 2, "se", CASE);

          case 'l':
            return collectKeyword(lexer, 2, 3, "ass", CLASS);

          case 'o':
            return collectKeyword(lexer, 2, 6, "ntinue", CONTINUE);
        }
      }

      break;

    case 'd':
      return collectKeyword(lexer, 1, 6, "efault", DEFAULT);


    case 'e':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'l':
            return collectKeyword(lexer, 2, 2, "se", ELSE);

          case 'x':
            return collectKeyword(lexer, 2, 5, "tends", EXTENDS);
        }
      }
      break;

    case 'f':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'a':
            return collectKeyword(lexer, 2, 3, "lse", FALSE);

          case 'i':
            return collectKeyword(lexer, 2, 3, "nal", FINAL);

          case 'o':
            return collectKeyword(lexer, 2, 1, "r", FOR);

          case 'u':
            return collectKeyword(lexer, 2, 6, "nction", FUNCTION);
        }
      }
      break;

    case 'i':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'f':
            return collectKeyword(lexer, 2, 0, "", IF);

          case 'm':
            return collectKeyword(lexer, 2, 4, "port", IMPORT);
        }
      }

    case 'n':
      return collectKeyword(lexer, 1, 3, "ull", SENEGAL_NULL);

    case 'r':
      return collectKeyword(lexer, 1, 5, "eturn", RETURN);

    case 's':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 't':
            return collectKeyword(lexer, 2, 4, "rict", STRICT);

          case 'u':
            return collectKeyword(lexer, 2, 3, "per", SUPER);

          case 'w':
            return collectKeyword(lexer, 2, 4, "itch", SWITCH);
        }
      }
      break;

    case 't':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'h':
            return collectKeyword(lexer, 2, 2, "is", THIS);

          case 'r':
            return collectKeyword(lexer, 2, 2, "ue", TRUE);
        }
      }

      break;

    case 'v':
      return collectKeyword(lexer, 1, 2, "ar", VAR);

    case 'w':
      return collectKeyword(lexer, 1, 4, "hile", WHILE);

  }

  return ID;
}

static Token collectId(Lexer* lexer) {
  char c = peek(lexer);

  while (isalpha(c) || isdigit(c) || c == '_') {
    advance(lexer);
    c = peek(lexer);
  }

  return newToken(lexer, idToken(lexer));
}

static Token collectNumber(Lexer* lexer) {
  while (isdigit(peek(lexer)))
    advance(lexer);

  if (peek(lexer) == '.' && isdigit(peekNext(lexer))) {
    advance(lexer);

    while (isdigit(peek(lexer)))
      advance(lexer);
  }

  return newToken(lexer, NUMBER);
}

// TODO(Calamity210): Walking lexer.start to remove a character is expensive, concatenating to a smaller string is a much better option
static Token collectString(Lexer* lexer) {
  TokenType type = STRING;

  for (;;) {
    char c = peek(lexer);

    if (c == '"')
      break;

    if (c == '\0')
      return errorToken(lexer, "Senegal encountered an unterminated String literal.");

    if (c == '\n')
      lexer->line++;

    if (c == '$' && peekNext(lexer) == '{')
      type = INTERPOLATION;

    if (c == '\\') {

      switch (peekNext(lexer)) {

        case '0':
          lexer->current[0] = '\0';
          advance(lexer);
          break;

        case 'a':
          lexer->current[0] = '\a';
          removeNextCharFromSource(lexer, 1);
          break;

        case 'b':
          lexer->current[0] = '\b';
          removeNextCharFromSource(lexer, 1);
          break;

        case 'f':
          lexer->current[0] = '\f';
          removeNextCharFromSource(lexer, 1);
          break;

        case 'n':
          lexer->current[0] = '\n';
          removeNextCharFromSource(lexer, 1);
          break;

        case 'r':
          lexer->current[0] = '\r';
          removeNextCharFromSource(lexer, 1);
          break;

        case 't':
          lexer->current[0] = '\t';
          removeNextCharFromSource(lexer, 1);
          break;

        case 'v':
          lexer->current[0] = '\v';
          removeNextCharFromSource(lexer, 1);
          break;


        default: {
          if (!isdigit(c)) {
            removeNextCharFromSource(lexer, 0);
          }
        }
      }
    }

    advance(lexer);
  }

  if (isAtEnd(lexer))
    return errorToken(lexer, "Senegal encountered an unterminated String literal.");

  advance(lexer);
  return newToken(lexer, type);
}

Token getNextToken(Lexer *lexer) {
  skipWhitespaceAndComment(lexer);

  lexer->start = lexer->current;

  if (isAtEnd(lexer))
    return newToken(lexer, SENEGAL_EOF);

  char c = advance(lexer);

  if (isdigit(c))
    return collectNumber(lexer);

  if (isalpha(c) || c == '_')
    return collectId(lexer);

  switch (c) {
    case ':':
      return newToken(lexer, COLON);


    case '(':
      return newToken(lexer, LPAREN);

    case ')':
      return newToken(lexer, RPAREN);

    case '{':
      return newToken(lexer, LBRACE);

    case '}':
      return newToken(lexer, RBRACE);

    case '[':
      return newToken(lexer, LBRACKET);

    case ']':
      return newToken(lexer, RBRACKET);

    case ';':
      return newToken(lexer, SEMI);

    case '^':
      return newToken(lexer, CARET);

    case ',':
      return newToken(lexer, COMMA);

    case '.':
      return newToken(lexer, DOT);

    case '-':
      return newToken(lexer, match(lexer, '-') ?
                             MINUS_MINUS : match(lexer, '=') ? MINUS_EQUAL : MINUS);

    case '+':
      return newToken(lexer, match(lexer, '+') ?
                             PLUS_PLUS : match(lexer, '=') ? PLUS_EQUAL : PLUS);

    case '/':
      return newToken(lexer, match(lexer, '=') ? SLASH_EQUAL : SLASH);

    case '*':
      return newToken(lexer, match(lexer, '*') ?
                             STAR_STAR : match(lexer, '=') ? STAR_EQUAL : STAR);

    case '~':
      return newToken(lexer, TILDE);

    case '&':
      return newToken(lexer, match(lexer, '&') ? AMP_AMP : AMP);

    case '!':
      return newToken(lexer, match(lexer, '=') ? BANG_EQUAL : BANG);

    case '=':
      return newToken(lexer, match(lexer, '=') ? EQUAL_EQUAL : match(lexer, '>') ? EQUAL_GREATER : EQUAL);

    case '<':
      return newToken(lexer, match(lexer, '=') ? LESSER_EQUAL : LESSER);

    case '>':
      return newToken(lexer, match(lexer, '=') ? GREATER_EQUAL : GREATER);

    case '|':
      return newToken(lexer, match(lexer, '|') ? PIPE_PIPE : PIPE);

    case '"':
      return collectString(lexer);

    default:
      return errorToken(lexer, "Senegal encountered an unexpected token.");
  }
}