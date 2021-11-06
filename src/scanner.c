//
// Hook Programming Language
// scanner.c
//

#include "scanner.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "error.h"

#define MATCH_MAX_LENGTH 8

#define CHAR_AT(s, i)   ((s)->pos[(i)])
#define CURRENT_CHAR(s) CHAR_AT(s, 0)

static inline void skip(scanner_t *scan);
static inline void next_char(scanner_t *scan);
static inline void next_chars(scanner_t *scan, int n);
static inline bool match_char(scanner_t *scan, const char c);
static inline bool match_chars(scanner_t *scan, const char *chars);
static inline bool match_number(scanner_t *scan);
static inline bool match_string(scanner_t *scan);
static inline bool match_name(scanner_t *scan);

static inline void skip(scanner_t *scan)
{
begin:
  while (isspace(CURRENT_CHAR(scan)))
    next_char(scan);
  if ((CHAR_AT(scan, 0) == '/' && CHAR_AT(scan, 1) == '/')
   || (CHAR_AT(scan, 0) == '#' && CHAR_AT(scan, 1) == '!'))
  {
    next_chars(scan, 2);
    for (;;)
    {
      if (CURRENT_CHAR(scan) == '\0')
        return;
      if (CURRENT_CHAR(scan) == '\n')
      {
        next_char(scan);
        goto begin;
      }
      next_char(scan);
    }
  }
}

static inline void next_char(scanner_t *scan)
{
  if (CURRENT_CHAR(scan) == '\n')
  {
    ++scan->line;
    scan->col = 1;
    ++scan->pos;
    return;
  }
  ++scan->col;
  ++scan->pos;
}

static inline void next_chars(scanner_t *scan, int n)
{
  for (int i = 0; i < n; ++i)
    next_char(scan);
}

static inline bool match_char(scanner_t *scan, const char c)
{
  if (CURRENT_CHAR(scan) != c)
    return false;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = 1;
  scan->token.start = scan->pos;
  next_char(scan);
  return true;
}

static inline bool match_chars(scanner_t *scan, const char *chars)
{
  int n = (int) strnlen(chars, MATCH_MAX_LENGTH);
  if (strncmp(scan->pos, chars, n)
   || (isalnum(CHAR_AT(scan, n)))
   || (CHAR_AT(scan, n) == '_'))
    return false;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

static inline bool match_number(scanner_t *scan)
{
  if (!isdigit(CURRENT_CHAR(scan)))
    return false;
  int n = 1;
  while (isdigit(CHAR_AT(scan, n)))
    ++n;
  token_type_t type = TOKEN_INT;
  if (CHAR_AT(scan, n) == '.')
  {
    ++n;
    while (isdigit(CHAR_AT(scan, n)))
      ++n;
    type = TOKEN_FLOAT;
  }
  if (isalnum(CHAR_AT(scan, n)) || CHAR_AT(scan, n) == '_')
    return false;
  scan->token.type = type;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

static inline bool match_string(scanner_t *scan)
{
  char chr = CURRENT_CHAR(scan);
  if (chr == '\'' || chr == '\"')
  {
    int n = 1;
    for (;;)
    {
      if (CHAR_AT(scan, n) == chr)
      {
        ++n;
        break;
      }
      if (CHAR_AT(scan, n) == '\0')
        fatal_error("unclosed string at %d:%d", scan->line, scan->col);
      ++n;
    }
    scan->token.type = TOKEN_STRING;
    scan->token.line = scan->line;
    scan->token.col = scan->col;
    scan->token.length = n - 2;
    scan->token.start = &scan->pos[1];
    next_chars(scan, n);
    return true;
  }
  return false;
}

static inline bool match_name(scanner_t *scan)
{
  if (CURRENT_CHAR(scan) != '_' && !isalpha(CURRENT_CHAR(scan)))
    return false;
  int n = 1;
  while (CHAR_AT(scan, n) == '_' || isalnum(CHAR_AT(scan, n)))
    ++n;
  scan->token.type = TOKEN_NAME;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

void scanner_init(scanner_t *scan, string_t *file, string_t *source)
{
  INCR_REF(file);
  scan->file = file;
  INCR_REF(source);
  scan->source = source;
  scan->pos = source->chars;
  scan->line = 1;
  scan->col = 1;
  scanner_next_token(scan);
}

void scanner_free(scanner_t *scan)
{
  string_t *file = scan->file;
  DECR_REF(file);
  if (IS_UNREACHABLE(file))
    string_free(file);
  string_t *source = scan->source;
  DECR_REF(source);
  if (IS_UNREACHABLE(source))
    string_free(source);
}

void scanner_next_token(scanner_t *scan)
{
  skip(scan);
  if (match_char(scan, 0))
  {
    scan->token.type = TOKEN_EOF;
    return;
  }
  if (match_char(scan, '.'))
  {
    scan->token.type = TOKEN_DOT;
    return;
  }
  if (match_char(scan, ','))
  {
    scan->token.type = TOKEN_COMMA;
    return;
  }
  if (match_char(scan, ':'))
  {
    scan->token.type = TOKEN_COLON;
    return;
  }
  if (match_char(scan, ';'))
  {
    scan->token.type = TOKEN_SEMICOLON;
    return;
  }
  if (match_char(scan, '('))
  {
    scan->token.type = TOKEN_LPAREN;
    return;
  }
  if (match_char(scan, ')'))
  {
    scan->token.type = TOKEN_RPAREN;
    return;
  }
  if (match_char(scan, '['))
  {
    scan->token.type = TOKEN_LBRACKET;
    return;
  }
  if (match_char(scan, ']'))
  {
    scan->token.type = TOKEN_RBRACKET;
    return;
  }
  if (match_char(scan, '{'))
  {
    scan->token.type = TOKEN_LBRACE;
    return;
  }
  if (match_char(scan, '}'))
  {
    scan->token.type = TOKEN_RBRACE;
    return;
  }
  if (match_chars(scan, "||"))
  {
    scan->token.type = TOKEN_PIPEPIPE;
    return;
  }
  if (match_chars(scan, "&&"))
  {
    scan->token.type = TOKEN_AMPAMP;
    return;
  }
  if (match_chars(scan, "=>"))
  {
    scan->token.type = TOKEN_ARROW;
    return;
  }
  if (match_chars(scan, "=="))
  {
    scan->token.type = TOKEN_EQEQ;
    return;
  }
  if (match_char(scan, '='))
  {
    scan->token.type = TOKEN_EQ;
    return;
  }
  if (match_chars(scan, "!="))
  {
    scan->token.type = TOKEN_BANGEQ;
    return;
  }
  if (match_char(scan, '!'))
  {
    scan->token.type = TOKEN_BANG;
    return;
  }
  if (match_chars(scan, ">="))
  {
    scan->token.type = TOKEN_GTEQ;
    return;
  }
  if (match_char(scan, '>'))
  {
    scan->token.type = TOKEN_GT;
    return;
  }
  if (match_chars(scan, "<="))
  {
    scan->token.type = TOKEN_LTEQ;
    return;
  }
  if (match_char(scan, '<'))
  {
    scan->token.type = TOKEN_LT;
    return;
  }
  if (match_chars(scan, "++"))
  {
    scan->token.type = TOKEN_PLUSPLUS;
    return;
  }
  if (match_chars(scan, "+="))
  {
    scan->token.type = TOKEN_PLUSEQ;
    return;
  }
  if (match_char(scan, '+'))
  {
    scan->token.type = TOKEN_PLUS;
    return;
  }
  if (match_chars(scan, "--"))
  {
    scan->token.type = TOKEN_MINUSMINUS;
    return;
  }
  if (match_chars(scan, "-="))
  {
    scan->token.type = TOKEN_MINUSEQ;
    return;
  }
  if (match_char(scan, '-'))
  {
    scan->token.type = TOKEN_MINUS;
    return;
  }
  if (match_chars(scan, "*="))
  {
    scan->token.type = TOKEN_STAREQ;
    return;
  }
  if (match_char(scan, '*'))
  {
    scan->token.type = TOKEN_STAR;
    return;
  }
  if (match_chars(scan, "/="))
  {
    scan->token.type = TOKEN_SLASHEQ;
    return;
  }
  if (match_char(scan, '/'))
  {
    scan->token.type = TOKEN_SLASH;
    return;
  }
  if (match_chars(scan, "%="))
  {
    scan->token.type = TOKEN_PERCENTEQ;
    return;
  }
  if (match_char(scan, '%'))
  {
    scan->token.type = TOKEN_PERCENT;
    return;
  }
  if (match_number(scan))
    return;
  if (match_string(scan))
    return;
  if (match_chars(scan, "break"))
  {
    scan->token.type = TOKEN_BREAK;
    return;
  }
  if (match_chars(scan, "const"))
  {
    scan->token.type = TOKEN_CONST;
    return;
  }
  if (match_chars(scan, "continue"))
  {
    scan->token.type = TOKEN_CONTINUE;
    return;
  }
  if (match_chars(scan, "delete"))
  {
    scan->token.type = TOKEN_DELETE;
    return;
  }
  if (match_chars(scan, "do"))
  {
    scan->token.type = TOKEN_DO;
    return;
  }
  if (match_chars(scan, "else"))
  {
    scan->token.type = TOKEN_ELSE;
    return;
  }
  if (match_chars(scan, "false"))
  {
    scan->token.type = TOKEN_FALSE;
    return;
  }
  if (match_chars(scan, "fn"))
  {
    scan->token.type = TOKEN_FN;
    return;
  }
  if (match_chars(scan, "for"))
  {
    scan->token.type = TOKEN_FOR;
    return;
  }
  if (match_chars(scan, "if"))
  {
    scan->token.type = TOKEN_IF;
    return;
  }
  if (match_chars(scan, "let"))
  {
    scan->token.type = TOKEN_LET;
    return;
  }
  if (match_chars(scan, "loop"))
  {
    scan->token.type = TOKEN_LOOP;
    return;
  }
  if (match_chars(scan, "null"))
  {
    scan->token.type = TOKEN_NULL;
    return;
  }
  if (match_chars(scan, "return"))
  {
    scan->token.type = TOKEN_RETURN;
    return;
  }
  if (match_chars(scan, "struct"))
  {
    scan->token.type = TOKEN_STRUCT;
    return;
  }
  if (match_chars(scan, "true"))
  {
    scan->token.type = TOKEN_TRUE;
    return;
  }
  if (match_chars(scan, "while"))
  {
    scan->token.type = TOKEN_WHILE;
    return;
  }
  if (match_name(scan))
    return;
  fatal_error("unable to recognize token at %d:%d", scan->line, scan->col);
}
