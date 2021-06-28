//
// Hook Programming Language
// scanner.c
//

#include "scanner.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "error.h"

#define CHAR_AT(s, i)   ((s)->pos[(i)])
#define CURRENT_CHAR(s) CHAR_AT(s, 0)

static inline void skip(scanner_t *scan);
static inline void next_char(scanner_t *scan);
static inline void next_chars(scanner_t *scan, int n);
static inline bool match_char(scanner_t *scan, const char c);
static inline bool match_chars(scanner_t *scan, const char *chars);
static inline bool match_number(scanner_t *scan);
static inline bool match_string(scanner_t *scan);
static inline bool match_varname(scanner_t *scan);

static inline void skip(scanner_t *scan)
{
begin:
  while (isspace(CURRENT_CHAR(scan)))
    next_char(scan);
  if (CHAR_AT(scan, 0) == '/'
    && CHAR_AT(scan, 1) == '/')
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
  }
  else
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
  int n = (int) strlen(chars);
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
  if (CURRENT_CHAR(scan) != '\'')
    return false;
  int n = 1;
  for (;;)
  {
    if (CHAR_AT(scan, n) == '\'')
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
  scan->token.length = n  - 2;
  scan->token.start = &scan->pos[1];
  next_chars(scan, n);
  return true;
}

static inline bool match_varname(scanner_t *scan)
{
  if (CURRENT_CHAR(scan) != '$')
    return false;
  int n = 1;
  while (isalnum(CHAR_AT(scan, n)) || CHAR_AT(scan, n) == '_')
    ++n;
  scan->token.type = TOKEN_VARNAME;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

void scanner_init(scanner_t *scan, char *chars)
{
  scan->pos = chars;
  scan->line = 1;
  scan->col = 1;
  scanner_next_token(scan);
}

void scanner_next_token(scanner_t *scan)
{
  skip(scan);
  if (match_char(scan, 0))
  {
    scan->token.type = TOKEN_EOF;
    return;
  }
  if (match_char(scan, ','))
  {
    scan->token.type = TOKEN_COMMA;
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
  if (match_char(scan, '='))
  {
    scan->token.type = TOKEN_EQ;
    return;
  }
  if (match_char(scan, '+'))
  {
    scan->token.type = TOKEN_PLUS;
    return;
  }
  if (match_char(scan, '-'))
  {
    scan->token.type = TOKEN_MINUS;
    return;
  }
  if (match_char(scan, '*'))
  {
    scan->token.type = TOKEN_STAR;
    return;
  }
  if (match_char(scan, '/'))
  {
    scan->token.type = TOKEN_SLASH;
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
  if (match_chars(scan, "echo"))
  {
    scan->token.type = TOKEN_ECHO;
    return;
  }
  if (match_chars(scan, "false"))
  {
    scan->token.type = TOKEN_FALSE;
    return;
  }
  if (match_chars(scan, "null"))
  {
    scan->token.type = TOKEN_NULL;
    return;
  }
  if (match_chars(scan, "true"))
  {
    scan->token.type = TOKEN_TRUE;
    return;
  }
  if (match_varname(scan))
    return;
  fatal_error("unable to recognize token at %d:%d", scan->line, scan->col);
}
