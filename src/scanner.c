//
// The Hook Programming Language
// scanner.c
//

#include "scanner.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MATCH_MAX_LENGTH (1 << 3)

#define char_at(s, i)   ((s)->pos[(i)])
#define current_char(s) char_at(s, 0)

static inline void lexical_error(Scanner *scan, const char *fmt, ...);
static inline void skip_shebang(Scanner *scan);
static inline void skip_spaces_comments(Scanner *scan);
static inline void next_char(Scanner *scan);
static inline void next_chars(Scanner *scan, int n);
static inline bool match_char(Scanner *scan, const char c);
static inline bool match_chars(Scanner *scan, const char *chars);
static inline bool match_keyword(Scanner *scan, const char *keyword);
static inline bool match_number(Scanner *scan);
static inline bool match_string(Scanner *scan);
static inline bool match_name(Scanner *scan);

static inline void lexical_error(Scanner *scan, const char *fmt, ...)
{
  fprintf(stderr, "lexical error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n  in %s:%d,%d\n", scan->file->chars, scan->line, scan->col);
  exit(EXIT_FAILURE);
}

static inline void skip_shebang(Scanner *scan)
{
  if (char_at(scan, 0) != '#' || char_at(scan, 1) != '!')
    return;
  next_chars(scan, 2);
  while (current_char(scan) != '\0')
  {
    if (current_char(scan) == '\n')
    {
      next_char(scan);
      break;
    }
    next_char(scan);
  }
}

static inline void skip_spaces_comments(Scanner *scan)
{
begin:
  while (isspace(current_char(scan)))
    next_char(scan);
  if ((char_at(scan, 0) == '/' && char_at(scan, 1) == '/'))
  {
    next_chars(scan, 2);
    for (;;)
    {
      if (current_char(scan) == '\0')
        return;
      if (current_char(scan) == '\n')
      {
        next_char(scan);
        goto begin;
      }
      next_char(scan);
    }
  }
}

static inline void next_char(Scanner *scan)
{
  if (current_char(scan) == '\n')
  {
    ++scan->line;
    scan->col = 1;
    ++scan->pos;
    return;
  }
  ++scan->col;
  ++scan->pos;
}

static inline void next_chars(Scanner *scan, int n)
{
  for (int i = 0; i < n; ++i)
    next_char(scan);
}

static inline bool match_char(Scanner *scan, const char c)
{
  if (current_char(scan) != c)
    return false;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = 1;
  scan->token.start = scan->pos;
  next_char(scan);
  return true;
}

static inline bool match_chars(Scanner *scan, const char *chars)
{
  int n = (int) strnlen(chars, MATCH_MAX_LENGTH);
  if (strncmp(scan->pos, chars, n))
    return false;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

static inline bool match_keyword(Scanner *scan, const char *keyword)
{
  int n = (int) strnlen(keyword, MATCH_MAX_LENGTH);
  if (strncmp(scan->pos, keyword, n)
   || (isalnum(char_at(scan, n)))
   || (char_at(scan, n) == '_'))
    return false;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

static inline bool match_number(Scanner *scan)
{
  int n = 0;
  if (char_at(scan, n) == '0')
    ++n;
  else
  {
    if (char_at(scan, n) < '1' || char_at(scan, n) > '9')
      return false;
    ++n;
    while (isdigit(char_at(scan, n)))
      ++n;
  }
  TokenKind kind = TOKEN_KIND_INT;
  if (char_at(scan, n) == '.')
  {
    if (!isdigit(char_at(scan, n + 1)))
      goto end;
    n += 2;
    while (isdigit(char_at(scan, n)))
      ++n;
    kind = TOKEN_KIND_FLOAT;
  }
  if (char_at(scan, n) == 'e' || char_at(scan, n) == 'E')
  {
    ++n;
    if (char_at(scan, n) == '+' || char_at(scan, n) == '-')
      ++n;
    if (!isdigit(char_at(scan, n)))
      return false;
    ++n;
    while (isdigit(char_at(scan, n)))
      ++n;
  }
  if (isalnum(char_at(scan, n)) || char_at(scan, n) == '_')
    return false;
end:
  scan->token.kind = kind;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

static inline bool match_string(Scanner *scan)
{
  char chr = current_char(scan);
  if (chr == '\'' || chr == '\"')
  {
    int n = 1;
    for (;;)
    {
      if (char_at(scan, n) == chr)
      {
        ++n;
        break;
      }
      if (char_at(scan, n) == '\0')
        lexical_error(scan, "unterminated string");
      ++n;
    }
    scan->token.kind = TOKEN_KIND_STRING;
    scan->token.line = scan->line;
    scan->token.col = scan->col;
    scan->token.length = n - 2;
    scan->token.start = &scan->pos[1];
    next_chars(scan, n);
    return true;
  }
  return false;
}

static inline bool match_name(Scanner *scan)
{
  if (current_char(scan) != '_' && !isalpha(current_char(scan)))
    return false;
  int n = 1;
  while (char_at(scan, n) == '_' || isalnum(char_at(scan, n)))
    ++n;
  scan->token.kind = TOKEN_KIND_NAME;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

void scanner_init(Scanner *scan, HkString *file, HkString *source)
{
  hk_incr_ref(file);
  scan->file = file;
  hk_incr_ref(source);
  scan->source = source;
  scan->pos = source->chars;
  scan->line = 1;
  scan->col = 1;
  skip_shebang(scan);
  scanner_next_token(scan);
}

void scanner_deinit(Scanner *scan)
{
  hk_string_release(scan->file);
  hk_string_release(scan->source);
}

void scanner_next_token(Scanner *scan)
{
  skip_spaces_comments(scan);
  if (match_char(scan, '\0'))
  {
    scan->token.kind = TOKEN_KIND_EOF;
    return;
  }
  if (match_chars(scan, ".."))
  {
    scan->token.kind = TOKEN_KIND_DOTDOT;
    return;
  }
  if (match_char(scan, '.'))
  {
    scan->token.kind = TOKEN_KIND_DOT;
    return;
  }
  if (match_char(scan, ','))
  {
    scan->token.kind = TOKEN_KIND_COMMA;
    return;
  }
  if (match_char(scan, ':'))
  {
    scan->token.kind = TOKEN_KIND_COLON;
    return;
  }
  if (match_char(scan, ';'))
  {
    scan->token.kind = TOKEN_KIND_SEMICOLON;
    return;
  }
  if (match_char(scan, '('))
  {
    scan->token.kind = TOKEN_KIND_LPAREN;
    return;
  }
  if (match_char(scan, ')'))
  {
    scan->token.kind = TOKEN_KIND_RPAREN;
    return;
  }
  if (match_char(scan, '['))
  {
    scan->token.kind = TOKEN_KIND_LBRACKET;
    return;
  }
  if (match_char(scan, ']'))
  {
    scan->token.kind = TOKEN_KIND_RBRACKET;
    return;
  }
  if (match_char(scan, '{'))
  {
    scan->token.kind = TOKEN_KIND_LBRACE;
    return;
  }
  if (match_char(scan, '}'))
  {
    scan->token.kind = TOKEN_KIND_RBRACE;
    return;
  }
  if (match_chars(scan, "|="))
  {
    scan->token.kind = TOKEN_KIND_PIPEEQ;
    return;
  }
  if (match_chars(scan, "||"))
  {
    scan->token.kind = TOKEN_KIND_PIPEPIPE;
    return;
  }
  if (match_char(scan, '|'))
  {
    scan->token.kind = TOKEN_KIND_PIPE;
    return;
  }
  if (match_chars(scan, "^="))
  {
    scan->token.kind = TOKEN_KIND_CARETEQ;
    return;
  }
  if (match_char(scan, '^'))
  {
    scan->token.kind = TOKEN_KIND_CARET;
    return;
  }
  if (match_chars(scan, "&="))
  {
    scan->token.kind = TOKEN_KIND_AMPEQ;
    return;
  }
  if (match_chars(scan, "&&"))
  {
    scan->token.kind = TOKEN_KIND_AMPAMP;
    return;
  }
  if (match_char(scan, '&'))
  {
    scan->token.kind = TOKEN_KIND_AMP;
    return;
  }
  if (match_chars(scan, "=>"))
  {
    scan->token.kind = TOKEN_KIND_ARROW;
    return;
  }
  if (match_chars(scan, "=="))
  {
    scan->token.kind = TOKEN_KIND_EQEQ;
    return;
  }
  if (match_char(scan, '='))
  {
    scan->token.kind = TOKEN_KIND_EQ;
    return;
  }
  if (match_chars(scan, "!="))
  {
    scan->token.kind = TOKEN_KIND_BANGEQ;
    return;
  }
  if (match_char(scan, '!'))
  {
    scan->token.kind = TOKEN_KIND_BANG;
    return;
  }
  if (match_chars(scan, ">="))
  {
    scan->token.kind = TOKEN_KIND_GTEQ;
    return;
  }
  if (match_chars(scan, ">>="))
  {
    scan->token.kind = TOKEN_KIND_GTGTEQ;
    return;
  }
  if (match_chars(scan, ">>"))
  {
    scan->token.kind = TOKEN_KIND_GTGT;
    return;
  }
  if (match_char(scan, '>'))
  {
    scan->token.kind = TOKEN_KIND_GT;
    return;
  }
  if (match_chars(scan, "<="))
  {
    scan->token.kind = TOKEN_KIND_LTEQ;
    return;
  }
  if (match_chars(scan, "<<="))
  {
    scan->token.kind = TOKEN_KIND_LTLTEQ;
    return;
  }
  if (match_chars(scan, "<<"))
  {
    scan->token.kind = TOKEN_KIND_LTLT;
    return;
  }
  if (match_char(scan, '<'))
  {
    scan->token.kind = TOKEN_KIND_LT;
    return;
  }
  if (match_chars(scan, "+="))
  {
    scan->token.kind = TOKEN_KIND_PLUSEQ;
    return;
  }
  if (match_chars(scan, "++"))
  {
    scan->token.kind = TOKEN_KIND_PLUSPLUS;
    return;
  }
  if (match_char(scan, '+'))
  {
    scan->token.kind = TOKEN_KIND_PLUS;
    return;
  }
  if (match_chars(scan, "-="))
  {
    scan->token.kind = TOKEN_KIND_DASHEQ;
    return;
  }
  if (match_chars(scan, "--"))
  {
    scan->token.kind = TOKEN_KIND_DASHDASH;
    return;
  }
  if (match_char(scan, '-'))
  {
    scan->token.kind = TOKEN_KIND_DASH;
    return;
  }
  if (match_chars(scan, "*="))
  {
    scan->token.kind = TOKEN_KIND_STAREQ;
    return;
  }
  if (match_char(scan, '*'))
  {
    scan->token.kind = TOKEN_KIND_STAR;
    return;
  }
  if (match_chars(scan, "/="))
  {
    scan->token.kind = TOKEN_KIND_SLASHEQ;
    return;
  }
  if (match_char(scan, '/'))
  {
    scan->token.kind = TOKEN_KIND_SLASH;
    return;
  }
  if (match_chars(scan, "~/="))
  {
    scan->token.kind = TOKEN_KIND_TILDESLASHEQ;
    return;
  }
  if (match_chars(scan, "~/"))
  {
    scan->token.kind = TOKEN_KIND_TILDESLASH;
    return;
  }
  if (match_char(scan, '~'))
  {
    scan->token.kind = TOKEN_KIND_TILDE;
    return;
  }
  if (match_chars(scan, "%="))
  {
    scan->token.kind = TOKEN_KIND_PERCENTEQ;
    return;
  }
  if (match_char(scan, '%'))
  {
    scan->token.kind = TOKEN_KIND_PERCENT;
    return;
  }
  if (match_number(scan))
    return;
  if (match_string(scan))
    return;
  if (match_keyword(scan, "_"))
  {
    scan->token.kind = TOKEN_KIND_UNDERSCORE_KW;
    return;
  }
  if (match_keyword(scan, "as"))
  {
    scan->token.kind = TOKEN_KIND_AS_KW;
    return;
  }
  if (match_keyword(scan, "break"))
  {
    scan->token.kind = TOKEN_KIND_BREAK_KW;
    return;
  }
  if (match_keyword(scan, "continue"))
  {
    scan->token.kind = TOKEN_KIND_CONTINUE_KW;
    return;
  }
  if (match_keyword(scan, "del"))
  {
    scan->token.kind = TOKEN_KIND_DEL_KW;
    return;
  }
  if (match_keyword(scan, "do"))
  {
    scan->token.kind = TOKEN_KIND_DO_KW;
    return;
  }
  if (match_keyword(scan, "else"))
  {
    scan->token.kind = TOKEN_KIND_ELSE_KW;
    return;
  }
  if (match_keyword(scan, "false"))
  {
    scan->token.kind = TOKEN_KIND_FALSE_KW;
    return;
  }
  if (match_keyword(scan, "fn"))
  {
    scan->token.kind = TOKEN_KIND_FN_KW;
    return;
  }
  if (match_keyword(scan, "foreach"))
  {
    scan->token.kind = TOKEN_KIND_FOREACH_KW;
    return;
  }
  if (match_keyword(scan, "for"))
  {
    scan->token.kind = TOKEN_KIND_FOR_KW;
    return;
  }
  if (match_keyword(scan, "from"))
  {
    scan->token.kind = TOKEN_KIND_FROM_KW;
    return;
  }
  if (match_keyword(scan, "if!"))
  {
    scan->token.kind = TOKEN_KIND_IFBANG_KW;
    return;
  }
  if (match_keyword(scan, "if"))
  {
    scan->token.kind = TOKEN_KIND_IF_KW;
    return;
  }
  if (match_keyword(scan, "import"))
  {
    scan->token.kind = TOKEN_KIND_IMPORT_KW;
    return;
  }
  if (match_keyword(scan, "in"))
  {
    scan->token.kind = TOKEN_KIND_IN_KW;
    return;
  }
  if (match_keyword(scan, "let"))
  {
    scan->token.kind = TOKEN_KIND_LET_KW;
    return;
  }
  if (match_keyword(scan, "loop"))
  {
    scan->token.kind = TOKEN_KIND_LOOP_KW;
    return;
  }
  if (match_keyword(scan, "match"))
  {
    scan->token.kind = TOKEN_KIND_MATCH_KW;
    return;
  }
  if (match_keyword(scan, "mut"))
  {
    scan->token.kind = TOKEN_KIND_MUT_KW;
    return;
  }
  if (match_keyword(scan, "nil"))
  {
    scan->token.kind = TOKEN_KIND_NIL_KW;
    return;
  }
  if (match_keyword(scan, "return"))
  {
    scan->token.kind = TOKEN_KIND_RETURN_KW;
    return;
  }
  if (match_keyword(scan, "struct"))
  {
    scan->token.kind = TOKEN_KIND_STRUCT_KW;
    return;
  }
  if (match_keyword(scan, "true"))
  {
    scan->token.kind = TOKEN_KIND_TRUE_KW;
    return;
  }
  if (match_keyword(scan, "while!"))
  {
    scan->token.kind = TOKEN_KIND_WHILEBANG_KW;
    return;
  }
  if (match_keyword(scan, "while"))
  {
    scan->token.kind = TOKEN_KIND_WHILE_KW;
    return;
  }
  if (match_name(scan))
    return;
  lexical_error(scan, "unexpected character");
}
