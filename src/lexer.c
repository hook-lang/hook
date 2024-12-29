//
// lexer.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "lexer.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MATCH_MAX_LENGTH (1 << 3)

#define char_at(s, i)   ((s)->pos[(i)])
#define current_char(s) char_at(s, 0)

static inline void lexical_error(Lexer *lex, const char *fmt, ...);
static inline void skip_shebang(Lexer *lex);
static inline void skip_spaces_comments(Lexer *lex);
static inline void next_char(Lexer *lex);
static inline void next_chars(Lexer *lex, int n);
static inline bool match_char(Lexer *lex, const char c);
static inline bool match_chars(Lexer *lex, const char *chars);
static inline bool match_keyword(Lexer *lex, const char *keyword);
static inline bool match_number(Lexer *lex);
static inline bool match_string(Lexer *lex);
static inline bool match_name(Lexer *lex);

static inline void lexical_error(Lexer *lex, const char *fmt, ...)
{
  fprintf(stderr, "lexical error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n  in %s:%d,%d\n", lex->file->chars, lex->line, lex->col);
  exit(EXIT_FAILURE);
}

static inline void skip_shebang(Lexer *lex)
{
  if (char_at(lex, 0) != '#' || char_at(lex, 1) != '!')
    return;
  next_chars(lex, 2);
  while (current_char(lex) != '\0')
  {
    if (current_char(lex) == '\n')
    {
      next_char(lex);
      break;
    }
    next_char(lex);
  }
}

static inline void skip_spaces_comments(Lexer *lex)
{
begin:
  while (isspace(current_char(lex)))
    next_char(lex);
  if ((char_at(lex, 0) == '/' && char_at(lex, 1) == '/'))
  {
    next_chars(lex, 2);
    for (;;)
    {
      if (current_char(lex) == '\0')
        return;
      if (current_char(lex) == '\n')
      {
        next_char(lex);
        goto begin;
      }
      next_char(lex);
    }
  }
}

static inline void next_char(Lexer *lex)
{
  if (current_char(lex) == '\n')
  {
    ++lex->line;
    lex->col = 1;
    ++lex->pos;
    return;
  }
  ++lex->col;
  ++lex->pos;
}

static inline void next_chars(Lexer *lex, int n)
{
  for (int i = 0; i < n; ++i)
    next_char(lex);
}

static inline bool match_char(Lexer *lex, const char c)
{
  if (current_char(lex) != c)
    return false;
  lex->token.line = lex->line;
  lex->token.col = lex->col;
  lex->token.length = 1;
  lex->token.start = lex->pos;
  next_char(lex);
  return true;
}

static inline bool match_chars(Lexer *lex, const char *chars)
{
  int n = (int) strnlen(chars, MATCH_MAX_LENGTH);
  if (strncmp(lex->pos, chars, n))
    return false;
  lex->token.line = lex->line;
  lex->token.col = lex->col;
  lex->token.length = n;
  lex->token.start = lex->pos;
  next_chars(lex, n);
  return true;
}

static inline bool match_keyword(Lexer *lex, const char *keyword)
{
  int n = (int) strnlen(keyword, MATCH_MAX_LENGTH);
  if (strncmp(lex->pos, keyword, n)
   || (isalnum(char_at(lex, n)))
   || (char_at(lex, n) == '_'))
    return false;
  lex->token.line = lex->line;
  lex->token.col = lex->col;
  lex->token.length = n;
  lex->token.start = lex->pos;
  next_chars(lex, n);
  return true;
}

static inline bool match_number(Lexer *lex)
{
  int n = 0;
  if (char_at(lex, n) == '0')
    ++n;
  else
  {
    if (char_at(lex, n) < '1' || char_at(lex, n) > '9')
      return false;
    ++n;
    while (isdigit(char_at(lex, n)))
      ++n;
  }
  TokenKind kind = TOKEN_KIND_INT;
  if (char_at(lex, n) == '.')
  {
    if (!isdigit(char_at(lex, n + 1)))
      goto end;
    n += 2;
    while (isdigit(char_at(lex, n)))
      ++n;
    kind = TOKEN_KIND_FLOAT;
  }
  if (char_at(lex, n) == 'e' || char_at(lex, n) == 'E')
  {
    ++n;
    if (char_at(lex, n) == '+' || char_at(lex, n) == '-')
      ++n;
    if (!isdigit(char_at(lex, n)))
      return false;
    ++n;
    while (isdigit(char_at(lex, n)))
      ++n;
  }
  if (isalnum(char_at(lex, n)) || char_at(lex, n) == '_')
    return false;
end:
  lex->token.kind = kind;
  lex->token.line = lex->line;
  lex->token.col = lex->col;
  lex->token.length = n;
  lex->token.start = lex->pos;
  next_chars(lex, n);
  return true;
}

static inline bool match_string(Lexer *lex)
{
  char chr = current_char(lex);
  if (chr == '\'' || chr == '\"')
  {
    int n = 1;
    for (;;)
    {
      if (char_at(lex, n) == chr)
      {
        ++n;
        break;
      }
      if (char_at(lex, n) == '\0')
        lexical_error(lex, "unterminated string");
      ++n;
    }
    lex->token.kind = TOKEN_KIND_STRING;
    lex->token.line = lex->line;
    lex->token.col = lex->col;
    lex->token.length = n - 2;
    lex->token.start = &lex->pos[1];
    next_chars(lex, n);
    return true;
  }
  return false;
}

static inline bool match_name(Lexer *lex)
{
  if (current_char(lex) != '_' && !isalpha(current_char(lex)))
    return false;
  int n = 1;
  while (char_at(lex, n) == '_' || isalnum(char_at(lex, n)))
    ++n;
  lex->token.kind = TOKEN_KIND_NAME;
  lex->token.line = lex->line;
  lex->token.col = lex->col;
  lex->token.length = n;
  lex->token.start = lex->pos;
  next_chars(lex, n);
  return true;
}

void lexer_init(Lexer *lex, HkString *file, HkString *source)
{
  hk_incr_ref(file);
  lex->file = file;
  hk_incr_ref(source);
  lex->source = source;
  lex->pos = source->chars;
  lex->line = 1;
  lex->col = 1;
  skip_shebang(lex);
  lexer_next_token(lex);
}

void lexer_deinit(Lexer *lex)
{
  hk_string_release(lex->file);
  hk_string_release(lex->source);
}

void lexer_next_token(Lexer *lex)
{
  skip_spaces_comments(lex);
  if (match_char(lex, '\0'))
  {
    lex->token.kind = TOKEN_KIND_EOF;
    return;
  }
  if (match_chars(lex, ".."))
  {
    lex->token.kind = TOKEN_KIND_DOTDOT;
    return;
  }
  if (match_char(lex, '.'))
  {
    lex->token.kind = TOKEN_KIND_DOT;
    return;
  }
  if (match_char(lex, ','))
  {
    lex->token.kind = TOKEN_KIND_COMMA;
    return;
  }
  if (match_char(lex, ':'))
  {
    lex->token.kind = TOKEN_KIND_COLON;
    return;
  }
  if (match_char(lex, ';'))
  {
    lex->token.kind = TOKEN_KIND_SEMICOLON;
    return;
  }
  if (match_char(lex, '('))
  {
    lex->token.kind = TOKEN_KIND_LPAREN;
    return;
  }
  if (match_char(lex, ')'))
  {
    lex->token.kind = TOKEN_KIND_RPAREN;
    return;
  }
  if (match_char(lex, '['))
  {
    lex->token.kind = TOKEN_KIND_LBRACKET;
    return;
  }
  if (match_char(lex, ']'))
  {
    lex->token.kind = TOKEN_KIND_RBRACKET;
    return;
  }
  if (match_char(lex, '{'))
  {
    lex->token.kind = TOKEN_KIND_LBRACE;
    return;
  }
  if (match_char(lex, '}'))
  {
    lex->token.kind = TOKEN_KIND_RBRACE;
    return;
  }
  if (match_chars(lex, "|="))
  {
    lex->token.kind = TOKEN_KIND_PIPEEQ;
    return;
  }
  if (match_chars(lex, "||"))
  {
    lex->token.kind = TOKEN_KIND_PIPEPIPE;
    return;
  }
  if (match_char(lex, '|'))
  {
    lex->token.kind = TOKEN_KIND_PIPE;
    return;
  }
  if (match_chars(lex, "^="))
  {
    lex->token.kind = TOKEN_KIND_CARETEQ;
    return;
  }
  if (match_char(lex, '^'))
  {
    lex->token.kind = TOKEN_KIND_CARET;
    return;
  }
  if (match_chars(lex, "&="))
  {
    lex->token.kind = TOKEN_KIND_AMPEQ;
    return;
  }
  if (match_chars(lex, "&&"))
  {
    lex->token.kind = TOKEN_KIND_AMPAMP;
    return;
  }
  if (match_char(lex, '&'))
  {
    lex->token.kind = TOKEN_KIND_AMP;
    return;
  }
  if (match_chars(lex, "=>"))
  {
    lex->token.kind = TOKEN_KIND_ARROW;
    return;
  }
  if (match_chars(lex, "=="))
  {
    lex->token.kind = TOKEN_KIND_EQEQ;
    return;
  }
  if (match_char(lex, '='))
  {
    lex->token.kind = TOKEN_KIND_EQ;
    return;
  }
  if (match_chars(lex, "!="))
  {
    lex->token.kind = TOKEN_KIND_BANGEQ;
    return;
  }
  if (match_char(lex, '!'))
  {
    lex->token.kind = TOKEN_KIND_BANG;
    return;
  }
  if (match_chars(lex, ">="))
  {
    lex->token.kind = TOKEN_KIND_GTEQ;
    return;
  }
  if (match_chars(lex, ">>="))
  {
    lex->token.kind = TOKEN_KIND_GTGTEQ;
    return;
  }
  if (match_chars(lex, ">>"))
  {
    lex->token.kind = TOKEN_KIND_GTGT;
    return;
  }
  if (match_char(lex, '>'))
  {
    lex->token.kind = TOKEN_KIND_GT;
    return;
  }
  if (match_chars(lex, "<="))
  {
    lex->token.kind = TOKEN_KIND_LTEQ;
    return;
  }
  if (match_chars(lex, "<<="))
  {
    lex->token.kind = TOKEN_KIND_LTLTEQ;
    return;
  }
  if (match_chars(lex, "<<"))
  {
    lex->token.kind = TOKEN_KIND_LTLT;
    return;
  }
  if (match_char(lex, '<'))
  {
    lex->token.kind = TOKEN_KIND_LT;
    return;
  }
  if (match_chars(lex, "+="))
  {
    lex->token.kind = TOKEN_KIND_PLUSEQ;
    return;
  }
  if (match_chars(lex, "++"))
  {
    lex->token.kind = TOKEN_KIND_PLUSPLUS;
    return;
  }
  if (match_char(lex, '+'))
  {
    lex->token.kind = TOKEN_KIND_PLUS;
    return;
  }
  if (match_chars(lex, "-="))
  {
    lex->token.kind = TOKEN_KIND_DASHEQ;
    return;
  }
  if (match_chars(lex, "--"))
  {
    lex->token.kind = TOKEN_KIND_DASHDASH;
    return;
  }
  if (match_char(lex, '-'))
  {
    lex->token.kind = TOKEN_KIND_DASH;
    return;
  }
  if (match_chars(lex, "*="))
  {
    lex->token.kind = TOKEN_KIND_STAREQ;
    return;
  }
  if (match_char(lex, '*'))
  {
    lex->token.kind = TOKEN_KIND_STAR;
    return;
  }
  if (match_chars(lex, "/="))
  {
    lex->token.kind = TOKEN_KIND_SLASHEQ;
    return;
  }
  if (match_char(lex, '/'))
  {
    lex->token.kind = TOKEN_KIND_SLASH;
    return;
  }
  if (match_chars(lex, "~/="))
  {
    lex->token.kind = TOKEN_KIND_TILDESLASHEQ;
    return;
  }
  if (match_chars(lex, "~/"))
  {
    lex->token.kind = TOKEN_KIND_TILDESLASH;
    return;
  }
  if (match_char(lex, '~'))
  {
    lex->token.kind = TOKEN_KIND_TILDE;
    return;
  }
  if (match_chars(lex, "%="))
  {
    lex->token.kind = TOKEN_KIND_PERCENTEQ;
    return;
  }
  if (match_char(lex, '%'))
  {
    lex->token.kind = TOKEN_KIND_PERCENT;
    return;
  }
  if (match_number(lex))
    return;
  if (match_string(lex))
    return;
  if (match_keyword(lex, "_"))
  {
    lex->token.kind = TOKEN_KIND_UNDERSCORE_KW;
    return;
  }
  if (match_keyword(lex, "as"))
  {
    lex->token.kind = TOKEN_KIND_AS_KW;
    return;
  }
  if (match_keyword(lex, "break"))
  {
    lex->token.kind = TOKEN_KIND_BREAK_KW;
    return;
  }
  if (match_keyword(lex, "continue"))
  {
    lex->token.kind = TOKEN_KIND_CONTINUE_KW;
    return;
  }
  if (match_keyword(lex, "del"))
  {
    lex->token.kind = TOKEN_KIND_DEL_KW;
    return;
  }
  if (match_keyword(lex, "do"))
  {
    lex->token.kind = TOKEN_KIND_DO_KW;
    return;
  }
  if (match_keyword(lex, "else"))
  {
    lex->token.kind = TOKEN_KIND_ELSE_KW;
    return;
  }
  if (match_keyword(lex, "false"))
  {
    lex->token.kind = TOKEN_KIND_FALSE_KW;
    return;
  }
  if (match_keyword(lex, "fn"))
  {
    lex->token.kind = TOKEN_KIND_FN_KW;
    return;
  }
  if (match_keyword(lex, "foreach"))
  {
    lex->token.kind = TOKEN_KIND_FOREACH_KW;
    return;
  }
  if (match_keyword(lex, "for"))
  {
    lex->token.kind = TOKEN_KIND_FOR_KW;
    return;
  }
  if (match_keyword(lex, "from"))
  {
    lex->token.kind = TOKEN_KIND_FROM_KW;
    return;
  }
  if (match_keyword(lex, "if!"))
  {
    lex->token.kind = TOKEN_KIND_IFBANG_KW;
    return;
  }
  if (match_keyword(lex, "if"))
  {
    lex->token.kind = TOKEN_KIND_IF_KW;
    return;
  }
  if (match_keyword(lex, "import"))
  {
    lex->token.kind = TOKEN_KIND_IMPORT_KW;
    return;
  }
  if (match_keyword(lex, "in"))
  {
    lex->token.kind = TOKEN_KIND_IN_KW;
    return;
  }
  if (match_keyword(lex, "let"))
  {
    lex->token.kind = TOKEN_KIND_LET_KW;
    return;
  }
  if (match_keyword(lex, "loop"))
  {
    lex->token.kind = TOKEN_KIND_LOOP_KW;
    return;
  }
  if (match_keyword(lex, "match"))
  {
    lex->token.kind = TOKEN_KIND_MATCH_KW;
    return;
  }
  if (match_keyword(lex, "nil"))
  {
    lex->token.kind = TOKEN_KIND_NIL_KW;
    return;
  }
  if (match_keyword(lex, "return"))
  {
    lex->token.kind = TOKEN_KIND_RETURN_KW;
    return;
  }
  if (match_keyword(lex, "struct"))
  {
    lex->token.kind = TOKEN_KIND_STRUCT_KW;
    return;
  }
  if (match_keyword(lex, "true"))
  {
    lex->token.kind = TOKEN_KIND_TRUE_KW;
    return;
  }
  if (match_keyword(lex, "var"))
  {
    lex->token.kind = TOKEN_KIND_VAR_KW;
    return;
  }

  if (match_keyword(lex, "while!"))
  {
    lex->token.kind = TOKEN_KIND_WHILEBANG_KW;
    return;
  }
  if (match_keyword(lex, "while"))
  {
    lex->token.kind = TOKEN_KIND_WHILE_KW;
    return;
  }
  if (match_name(lex))
    return;
  lexical_error(lex, "unexpected character");
}
