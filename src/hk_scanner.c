//
// The Hook Programming Language
// hk_scanner.c
//

#include "hk_scanner.h"
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define MATCH_MAX_LENGTH (1 << 3)

#define char_at(s, i) ((s)->pos[(i)])
#define current_char(s) char_at(s, 0)

static inline void lexical_error(scanner_t *scan, const char *fmt, ...);
static inline void skip_shebang(scanner_t *scan);
static inline void skip_spaces_comments(scanner_t *scan);
static inline void next_char(scanner_t *scan);
static inline void next_chars(scanner_t *scan, int32_t n);
static inline bool match_char(scanner_t *scan, const char c);
static inline bool match_chars(scanner_t *scan, const char *chars);
static inline bool match_keyword(scanner_t *scan, const char *keyword);
static inline bool match_number(scanner_t *scan);
static inline bool match_string(scanner_t *scan);
static inline bool match_name(scanner_t *scan);

static inline void lexical_error(scanner_t *scan, const char *fmt, ...)
{
  fprintf(stderr, "lexical error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n  in %s:%d,%d\n", scan->file->chars, scan->line, scan->col);
  exit(EXIT_FAILURE);
}

static inline void skip_shebang(scanner_t *scan)
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

static inline void skip_spaces_comments(scanner_t *scan)
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

static inline void next_char(scanner_t *scan)
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

static inline void next_chars(scanner_t *scan, int32_t n)
{
  for (int32_t i = 0; i < n; ++i)
    next_char(scan);
}

static inline bool match_char(scanner_t *scan, const char c)
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

static inline bool match_chars(scanner_t *scan, const char *chars)
{
  int32_t n = (int32_t)strnlen(chars, MATCH_MAX_LENGTH);
  if (strncmp(scan->pos, chars, n))
    return false;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

static inline bool match_keyword(scanner_t *scan, const char *keyword)
{
  int32_t n = (int32_t)strnlen(keyword, MATCH_MAX_LENGTH);
  if (strncmp(scan->pos, keyword, n) || (isalnum(char_at(scan, n))) || (char_at(scan, n) == '_'))
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
  int32_t n = 0;
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
  token_type_t type = TOKEN_INT;
  if (char_at(scan, n) == '.')
  {
    if (!isdigit(char_at(scan, n + 1)))
      goto end;
    n += 2;
    while (isdigit(char_at(scan, n)))
      ++n;
    type = TOKEN_FLOAT;
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
  scan->token.type = type;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

static inline char render_escape_char(scanner_t *scan, int32_t n)
{
  char escaped_chr = char_at(scan, n + 1);
  switch (escaped_chr)
  {
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case '\\':
    return '\\';
  case '\'':
    return '\'';
  case '\"':
    return '\"';
  default:
    lexical_error(scan, "invalid escape sequence");
    return '\0';
  }
}

static inline bool match_string(scanner_t *scan)
{
  char string_delimiter = current_char(scan);
  if (string_delimiter == '\'' || string_delimiter == '\"')
  {
    hk_string_t *literal_string = hk_string_new();
    char literal_char = '\0';
    int32_t n = 1;
    for (;;)
    {
      char scanned_char = char_at(scan, n);
      if (scanned_char == string_delimiter)
      {
        ++n;
        break;
      }
      if (scanned_char == '\\')
      {
        literal_char = render_escape_char(scan, n);
        hk_string_inplace_concat_char(literal_string, literal_char);
        n += 2;
        continue;
      }
      if (scanned_char == '\0')
        lexical_error(scan, "unterminated string");
      hk_string_inplace_concat_char(literal_string, scanned_char);
      ++n;
    }
    scan->token.type = TOKEN_STRING;
    scan->token.line = scan->line;
    scan->token.col = scan->col;
    scan->token.length = n - 2;
    scan->token.start = &scan->pos[1];
    scan->token.value = literal_string;
    next_chars(scan, n);
    return true;
  }
  return false;
}

static inline bool match_name(scanner_t *scan)
{
  if (current_char(scan) != '_' && !isalpha(current_char(scan)))
    return false;
  int32_t n = 1;
  while (char_at(scan, n) == '_' || isalnum(char_at(scan, n)))
    ++n;
  scan->token.type = TOKEN_NAME;
  scan->token.line = scan->line;
  scan->token.col = scan->col;
  scan->token.length = n;
  scan->token.start = scan->pos;
  next_chars(scan, n);
  return true;
}

void scanner_init(scanner_t *scan, hk_string_t *file, hk_string_t *source)
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

void scanner_free(scanner_t *scan)
{
  hk_string_release(scan->file);
  hk_string_release(scan->source);
  hk_string_release(scan->token.value);
}

void scanner_next_token(scanner_t *scan)
{
  skip_spaces_comments(scan);
  if (match_char(scan, '\0'))
  {
    scan->token.type = TOKEN_EOF;
    return;
  }
  if (match_chars(scan, ".."))
  {
    scan->token.type = TOKEN_DOTDOT;
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
  if (match_chars(scan, "|="))
  {
    scan->token.type = TOKEN_PIPEEQ;
    return;
  }
  if (match_chars(scan, "||"))
  {
    scan->token.type = TOKEN_PIPEPIPE;
    return;
  }
  if (match_char(scan, '|'))
  {
    scan->token.type = TOKEN_PIPE;
    return;
  }
  if (match_chars(scan, "^="))
  {
    scan->token.type = TOKEN_CARETEQ;
    return;
  }
  if (match_char(scan, '^'))
  {
    scan->token.type = TOKEN_CARET;
    return;
  }
  if (match_chars(scan, "&="))
  {
    scan->token.type = TOKEN_AMPEQ;
    return;
  }
  if (match_chars(scan, "&&"))
  {
    scan->token.type = TOKEN_AMPAMP;
    return;
  }
  if (match_char(scan, '&'))
  {
    scan->token.type = TOKEN_AMP;
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
  if (match_chars(scan, ">>="))
  {
    scan->token.type = TOKEN_GTGTEQ;
    return;
  }
  if (match_chars(scan, ">>"))
  {
    scan->token.type = TOKEN_GTGT;
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
  if (match_chars(scan, "<<="))
  {
    scan->token.type = TOKEN_LTLTEQ;
    return;
  }
  if (match_chars(scan, "<<"))
  {
    scan->token.type = TOKEN_LTLT;
    return;
  }
  if (match_char(scan, '<'))
  {
    scan->token.type = TOKEN_LT;
    return;
  }
  if (match_chars(scan, "+="))
  {
    scan->token.type = TOKEN_PLUSEQ;
    return;
  }
  if (match_chars(scan, "++"))
  {
    scan->token.type = TOKEN_PLUSPLUS;
    return;
  }
  if (match_char(scan, '+'))
  {
    scan->token.type = TOKEN_PLUS;
    return;
  }
  if (match_chars(scan, "-="))
  {
    scan->token.type = TOKEN_DASHEQ;
    return;
  }
  if (match_chars(scan, "--"))
  {
    scan->token.type = TOKEN_DASHDASH;
    return;
  }
  if (match_char(scan, '-'))
  {
    scan->token.type = TOKEN_DASH;
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
  if (match_chars(scan, "~/="))
  {
    scan->token.type = TOKEN_TILDESLASHEQ;
    return;
  }
  if (match_chars(scan, "~/"))
  {
    scan->token.type = TOKEN_TILDESLASH;
    return;
  }
  if (match_char(scan, '~'))
  {
    scan->token.type = TOKEN_TILDE;
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
  if (match_chars(scan, "_"))
  {
    scan->token.type = TOKEN_UNDERSCORE;
    return;
  }
  if (match_keyword(scan, "as"))
  {
    scan->token.type = TOKEN_AS;
    return;
  }
  if (match_keyword(scan, "break"))
  {
    scan->token.type = TOKEN_BREAK;
    return;
  }
  if (match_keyword(scan, "continue"))
  {
    scan->token.type = TOKEN_CONTINUE;
    return;
  }
  if (match_keyword(scan, "del"))
  {
    scan->token.type = TOKEN_DEL;
    return;
  }
  if (match_keyword(scan, "do"))
  {
    scan->token.type = TOKEN_DO;
    return;
  }
  if (match_keyword(scan, "else"))
  {
    scan->token.type = TOKEN_ELSE;
    return;
  }
  if (match_keyword(scan, "false"))
  {
    scan->token.type = TOKEN_FALSE;
    return;
  }
  if (match_keyword(scan, "fn"))
  {
    scan->token.type = TOKEN_FN;
    return;
  }
  if (match_keyword(scan, "foreach"))
  {
    scan->token.type = TOKEN_FOREACH;
    return;
  }
  if (match_keyword(scan, "for"))
  {
    scan->token.type = TOKEN_FOR;
    return;
  }
  if (match_keyword(scan, "from"))
  {
    scan->token.type = TOKEN_FROM;
    return;
  }
  if (match_keyword(scan, "if!"))
  {
    scan->token.type = TOKEN_IFBANG;
    return;
  }
  if (match_keyword(scan, "if"))
  {
    scan->token.type = TOKEN_IF;
    return;
  }
  if (match_keyword(scan, "import"))
  {
    scan->token.type = TOKEN_IMPORT;
    return;
  }
  if (match_keyword(scan, "in"))
  {
    scan->token.type = TOKEN_IN;
    return;
  }
  if (match_keyword(scan, "let"))
  {
    scan->token.type = TOKEN_LET;
    return;
  }
  if (match_keyword(scan, "loop"))
  {
    scan->token.type = TOKEN_LOOP;
    return;
  }
  if (match_keyword(scan, "match"))
  {
    scan->token.type = TOKEN_MATCH;
    return;
  }
  if (match_keyword(scan, "mut"))
  {
    scan->token.type = TOKEN_MUT;
    return;
  }
  if (match_keyword(scan, "nil"))
  {
    scan->token.type = TOKEN_NIL;
    return;
  }
  if (match_keyword(scan, "return"))
  {
    scan->token.type = TOKEN_RETURN;
    return;
  }
  if (match_keyword(scan, "struct"))
  {
    scan->token.type = TOKEN_STRUCT;
    return;
  }
  if (match_keyword(scan, "true"))
  {
    scan->token.type = TOKEN_TRUE;
    return;
  }
  if (match_keyword(scan, "while!"))
  {
    scan->token.type = TOKEN_WHILEBANG;
    return;
  }
  if (match_keyword(scan, "while"))
  {
    scan->token.type = TOKEN_WHILE;
    return;
  }
  if (match_name(scan))
    return;
  lexical_error(scan, "unexpected character");
}
