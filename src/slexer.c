#include <string.h>
#include <ctype.h>
#include <wchar.h>

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

static Token newToken(Lexer* lexer, SenegalTokenType type) {
  Token token;
  token.type = type;
  token.start = lexer->start;
  token.length = (int)(lexer->current - lexer->start);
  token.line = lexer->line;

  return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
  Token token;
  token.type = SENEGAL_ERROR;
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

static SenegalTokenType collectKeyword(Lexer* lexer, int start, int length, const char* rest, SenegalTokenType type) {
  if (lexer->current - lexer->start == start + length
      && memcmp(lexer->start + start, rest, length) == 0)
    return type;

  return SENEGAL_ID;
}

static SenegalTokenType idToken(Lexer* lexer) {

  switch (lexer->start[0]) {
    case 'a':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 's':
            return collectKeyword(lexer, 2, 3, "ync", SENEGAL_ASYNC);

          case 'w':
            return collectKeyword(lexer, 2, 3, "ait", SENEGAL_AWAIT);
        }
      }
      break;

    case 'b':
      return collectKeyword(lexer, 1, 4, "reak", SENEGAL_BREAK);

    case 'c':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'a':
            return collectKeyword(lexer, 2, 2, "se", SENEGAL_CASE);

          case 'l':
            return collectKeyword(lexer, 2, 3, "ass", SENEGAL_CLASS);

          case 'o':
            if (lexer->current - lexer->start > 3) {
              switch (lexer->start[3]) {
                case 's':
                  return collectKeyword(lexer, 4, 1, "t", SENEGAL_CONST);

                case 't':
                  return collectKeyword(lexer, 4, 4, "inue", SENEGAL_CONTINUE);
              }
            }

        }
      }

      break;

    case 'd':
      return collectKeyword(lexer, 1, 6, "efault", SENEGAL_DEFAULT);


    case 'e':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'l':
            return collectKeyword(lexer, 2, 2, "se", SENEGAL_ELSE);

          case 'x':
            return collectKeyword(lexer, 2, 5, "tends", SENEGAL_EXTENDS);
        }
      }
      break;

    case 'f':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'a':
            return collectKeyword(lexer, 2, 3, "lse", SENEGAL_FALSE);

          case 'i':
            return collectKeyword(lexer, 2, 3, "nal", SENEGAL_FINAL);

          case 'o':
            return collectKeyword(lexer, 2, 1, "r", SENEGAL_FOR);

          case 'u':
            return collectKeyword(lexer, 2, 6, "nction", SENEGAL_FUNCTION);
        }
      }
      break;

    case 'i':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'f':
            return collectKeyword(lexer, 2, 0, "", SENEGAL_IF);

          case 'm':
            return collectKeyword(lexer, 2, 4, "port", SENEGAL_IMPORT);
        }
      }

    case 'n':
      return collectKeyword(lexer, 1, 3, "ull", SENEGAL_NULL);

    case 'r':
      return collectKeyword(lexer, 1, 5, "eturn", SENEGAL_RETURN);

    case 's':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 't':
            return collectKeyword(lexer, 2, 4, "atic", SENEGAL_STATIC);

          case 'u':
            if (lexer->current - lexer->start > 2) {
              switch (lexer->start[2]) {
                case 'p':
                  return collectKeyword(lexer, 3, 2, "er", SENEGAL_SUPER);

                case 's':
                  return collectKeyword(lexer, 3, 4, "pend", SENEGAL_SUSPEND);
              }
            }

          case 'w':
            return collectKeyword(lexer, 2, 4, "itch", SENEGAL_SWITCH);
        }
      }
      break;

    case 't':
      if (lexer->current - lexer->start > 1) {
        switch (lexer->start[1]) {
          case 'h':
            if (lexer->current - lexer->start > 2) {
              switch (lexer->start[2]) {
                case 'i':
                  return collectKeyword(lexer, 3, 1, "s", SENEGAL_THIS);

                case 'r':
                  return collectKeyword(lexer, 3, 2, "ow", SENEGAL_THROW);
              }
            }

          case 'r':
            return collectKeyword(lexer, 2, 2, "ue", SENEGAL_TRUE);
        }
      }

      break;

    case 'v':
      return collectKeyword(lexer, 1, 2, "ar", SENEGAL_VAR);

    case 'w':
      return collectKeyword(lexer, 1, 4, "hile", SENEGAL_WHILE);

    case 'y':
      return collectKeyword(lexer, 1, 4, "ield", SENEGAL_YIELD);

  }

  return SENEGAL_ID;
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

  if (lexer->current[-1] == '0' && *lexer->current == 'x') {
    advance(lexer);

    lexer->start = lexer->current;

    while (isxdigit((peek(lexer))))
      advance(lexer);

    return newToken(lexer, SENEGAL_HEX);
  }


  while (isdigit((peek(lexer))))
    advance(lexer);

  if (peek(lexer) == '.' && isdigit(peekNext(lexer))) {
    advance(lexer);

    while (isdigit(peek(lexer)))
      advance(lexer);
  }

  return newToken(lexer, SENEGAL_NUMBER);
}

// TODO(Calamity210): Walking lexer.start to remove a character is expensive, concatenating to a smaller string is a much better option
static Token collectString(Lexer* lexer, char quotation) {
  SenegalTokenType type = SENEGAL_STRING;

  for (;;) {
    char c = peek(lexer);

    if (c == quotation)
      break;

    if (c == '\0')
      return errorToken(lexer, "Senegal encountered an unterminated String literal.");

    if (c == '\n')
      lexer->line++;

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

          // "\xhh"
        case 'x': {
          char hexstr[2] = {lexer->current[2], lexer->current[3]};

          lexer->current[0] = strtol(hexstr, NULL, 16);

          // "x"
          removeNextCharFromSource(lexer, 1);

          // First digit
          removeNextCharFromSource(lexer, 1);

          // Second digit
          removeNextCharFromSource(lexer, 1);
          break;
        }

        // "\uhhhh"
        case 'u': {
          char hexstr[4] = {lexer->current[2], lexer->current[3], lexer->current[4], lexer->current[5]};

          lexer->current[0] = (wint_t)strtol(hexstr, NULL, 16);

          // "u"
          removeNextCharFromSource(lexer, 1);

          // First digit
          removeNextCharFromSource(lexer, 1);

          // Second digit
          removeNextCharFromSource(lexer, 1);

          // Third digit
          removeNextCharFromSource(lexer, 1);

          // Fourth digit
          removeNextCharFromSource(lexer, 1);

          break;
        }

          // "\Uhhhhhhhh"
        case 'U': {
          char hexstr[8] = {lexer->current[2], lexer->current[3], lexer->current[4], lexer->current[5],
                            lexer->current[6], lexer->current[7], lexer->current[8], lexer->current[9]};

          lexer->current[0] = (wint_t)strtol(hexstr, NULL, 16);

          // "U"
          removeNextCharFromSource(lexer, 1);

          // First digit
          removeNextCharFromSource(lexer, 1);

          // Second digit
          removeNextCharFromSource(lexer, 1);

          // Third digit
          removeNextCharFromSource(lexer, 1);

          // Fourth digit
          removeNextCharFromSource(lexer, 1);

          // Fifth digit
          removeNextCharFromSource(lexer, 1);

          // Sixth digit
          removeNextCharFromSource(lexer, 1);

          // Seventh digit
          removeNextCharFromSource(lexer, 1);

          // eighth digit
          removeNextCharFromSource(lexer, 1);

          break;
        }

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
      return newToken(lexer, SENEGAL_COLON);


    case '(':
      return newToken(lexer, SENEGAL_LPAREN);

    case ')':
      return newToken(lexer, SENEGAL_RPAREN);

    case '{':
      return newToken(lexer, SENEGAL_LBRACE);

    case '}':
      return newToken(lexer, SENEGAL_RBRACE);

    case '[':
      return newToken(lexer, SENEGAL_LBRACKET);

    case ']':
      return newToken(lexer, SENEGAL_RBRACKET);

    case ';':
      return newToken(lexer, SENEGAL_SEMI);

    case '^':
      return newToken(lexer, SENEGAL_CARET);

    case ',':
      return newToken(lexer, SENEGAL_COMMA);

    case '.':
      return newToken(lexer, SENEGAL_DOT);

    case '-':
      return newToken(lexer, match(lexer, '-') ?
                             SENEGAL_MINUS_MINUS : match(lexer, '=') ? SENEGAL_MINUS_EQUAL : SENEGAL_MINUS);

    case '+':
      return newToken(lexer, match(lexer, '+') ?
                             SENEGAL_PLUS_PLUS : match(lexer, '=') ? SENEGAL_PLUS_EQUAL : SENEGAL_PLUS);

    case '?':
      return newToken(lexer, SENEGAL_QUESTION);

    case '/':
      return newToken(lexer, match(lexer, '=') ? SENEGAL_SLASH_EQUAL : SENEGAL_SLASH);

    case '*':
      return newToken(lexer, match(lexer, '*') ?
                             SENEGAL_STAR_STAR : match(lexer, '=') ? SENEGAL_STAR_EQUAL : SENEGAL_STAR);

    case '~':
      return newToken(lexer, SENEGAL_TILDE);

    case '&':
      return newToken(lexer, match(lexer, '&') ? SENEGAL_AMP_AMP : SENEGAL_AMP);

    case '!':
      return newToken(lexer, match(lexer, '=') ? SENEGAL_BANG_EQUAL : SENEGAL_BANG);

    case '=':
      return newToken(lexer, match(lexer, '=') ?
                             SENEGAL_EQUAL_EQUAL : match(lexer, '>') ? SENEGAL_EQUAL_GREATER : SENEGAL_EQUAL);

    case '<':
      return newToken(lexer, match(lexer, '=') ? SENEGAL_LESSER_EQUAL : SENEGAL_LESSER);

    case '>':
      return newToken(lexer, match(lexer, '=') ? SENEGAL_GREATER_EQUAL : SENEGAL_GREATER);

    case '|':
      return newToken(lexer, match(lexer, '|') ? SENEGAL_PIPE_PIPE : SENEGAL_PIPE);

    case '"':
    case '\'':
      return collectString(lexer, c);

    default:
      return errorToken(lexer, "Senegal encountered an unexpected token.");
  }
}