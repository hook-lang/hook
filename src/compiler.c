//
// Hook Programming Language
// compiler.c
//

#include "compiler.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "string.h"
#include "error.h"

#define MATCH(s, t) ((s)->token.type == (t))

#define EXPECT(s, t) do \
  { \
    if (!MATCH(s, t)) \
      fatal_error_unexpected_token(s); \
    scanner_next_token(s); \
  } while(0)

static inline void fatal_error_unexpected_token(scanner_t *scan);
static inline long parse_long(char *chars);
static inline double parse_double(char *chars);
static inline bool name_equal(token_t *tk, local_t *local);
static inline int resolve_local(compiler_t *comp, token_t *tk);
static inline int add_local(compiler_t *comp, token_t *tk);
static void compile_statement(compiler_t *comp);
static void compile_assignment(compiler_t *comp);
static void compile_echo(compiler_t *comp);
static void compile_expression(compiler_t *comp);
static void compile_mul_expression(compiler_t *comp);
static void compile_unary_expression(compiler_t *comp);
static void compile_prim_expression(compiler_t *comp);

static inline void fatal_error_unexpected_token(scanner_t *scan)
{
  token_t *tk = &scan->token;
  if (tk->type == TOKEN_EOF)
  {
    const char *fmt = "unexpected end of file at %d:%d";
    fatal_error(fmt, tk->line, tk->col);
  }
  const char *fmt = "unexpected token '%.*s' at %d:%d";
  fatal_error(fmt, tk->length, tk->start, tk->line, tk->col);
}

static inline long parse_long(char *chars)
{
  errno = 0;
  long result = strtol(chars, NULL, 10);
  if (errno == ERANGE)
    fatal_error("integer number too large");
  return result;
}

static inline double parse_double(char *chars)
{
  errno = 0;
  double result = strtod(chars, NULL);
  if (errno == ERANGE)
    fatal_error("floating point number too large");
  return result;
}

static inline bool name_equal(token_t *tk, local_t *local)
{
  if (tk->length != local->length)
    return false;
  return !memcmp(tk->start, local->start, tk->length);
}

static inline int resolve_local(compiler_t *comp, token_t *tk)
{
  for (int i = 0; i < comp->local_count; ++i)
    if (name_equal(tk, &comp->locals[i]))
      return i;
  return -1;
}

static inline int add_local(compiler_t *comp, token_t *tk)
{
  if (comp->local_count == COMPILER_MAX_LOCALS)
    fatal_error("too many local variables");
  int index = comp->local_count;
  local_t *local = &comp->locals[index];
  local->length = tk->length;
  local->start = tk->start;
  ++comp->local_count;
  return index;
}

static void compile_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  if (MATCH(scan, TOKEN_VARNAME))
  {
    compile_assignment(comp);
    return;
  }
  if (MATCH(scan, TOKEN_ECHO))
  {
    compile_echo(comp);
    return;
  }
  fatal_error_unexpected_token(scan);
}

static void compile_assignment(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  token_t *tk = &scan->token;
  int index = resolve_local(comp, tk);
  if (index == -1)
    add_local(comp, tk);
  scanner_next_token(scan);
  EXPECT(scan, TOKEN_EQ);
  compile_expression(comp);
  EXPECT(scan, TOKEN_SEMICOLON);
  if (index == -1)
    return;
  chunk_t *chunk = comp->chunk;
  chunk_emit_opcode(chunk, OP_STORE);
  chunk_write_byte(chunk, index);
}

static void compile_echo(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  scanner_next_token(scan);
  compile_expression(comp);
  EXPECT(scan, TOKEN_SEMICOLON);
  chunk_emit_opcode(comp->chunk, OP_PRINT);
}

static void compile_expression(compiler_t *comp)
{
  compile_mul_expression(comp);
  scanner_t *scan = comp->scan;
  chunk_t *chunk = comp->chunk;
  for (;;)
  {
    if (MATCH(scan, TOKEN_PLUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(comp);
      chunk_emit_opcode(chunk, OP_ADD);
      continue;
    }
    if (MATCH(scan, TOKEN_MINUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(comp);
      chunk_emit_opcode(chunk, OP_SUBTRACT);
      continue;
    }
    break;
  }
}

static void compile_mul_expression(compiler_t *comp)
{
  compile_unary_expression(comp);
  scanner_t *scan = comp->scan;
  chunk_t *chunk = comp->chunk;
  for (;;)
  {
    if (MATCH(scan, TOKEN_STAR))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_MULTIPLY);
      continue;
    }
    if (MATCH(scan, TOKEN_SLASH))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_DIVIDE);
      continue;
    }
    if (MATCH(scan, TOKEN_PERCENT))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_MODULO);
      continue;
    }
    break;
  }
}

static void compile_unary_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  if (MATCH(scan, TOKEN_PLUS))
  {
    scanner_next_token(scan);
    compile_unary_expression(comp);
    return;
  }
  if (MATCH(scan, TOKEN_MINUS))
  {
    scanner_next_token(scan);
    compile_unary_expression(comp);
    chunk_emit_opcode(comp->chunk, OP_NEGATE);
    return;
  }
  compile_prim_expression(comp);
}

static void compile_prim_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = comp->chunk;
  if (MATCH(scan, TOKEN_NULL))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_NULL);
    return;
  }
  if (MATCH(scan, TOKEN_FALSE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_FALSE);
    return;
  }
  if (MATCH(scan, TOKEN_TRUE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_TRUE);
    return;
  }
  array_t *consts = comp->consts;
  if (MATCH(scan, TOKEN_INT))
  {
    long data = parse_long(scan->token.start);
    scanner_next_token(scan);
    if (data <= UINT16_MAX)
    {
      chunk_emit_opcode(chunk, OP_INT);
      chunk_write_word(chunk, (uint16_t) data);
      return;
    }
    int index = consts->length;
    array_add_element(consts, NUMBER_VALUE(data));
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_write_byte(chunk, index);
    return;
  }
  if (MATCH(scan, TOKEN_FLOAT))
  {
    double data = parse_double(scan->token.start);
    scanner_next_token(scan);
    int index = consts->length;
    array_add_element(consts, NUMBER_VALUE(data));
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_write_byte(chunk, index);
    return;
  }
  if (MATCH(scan, TOKEN_STRING))
  {
    token_t *tk = &scan->token;
    string_t *str = string_from_chars(tk->length, tk->start);
    scanner_next_token(scan);
    int index = consts->length;
    array_add_element(consts, STRING_VALUE(str));
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_write_byte(chunk, index);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_RBRACKET))
    {
      scanner_next_token(scan);
      chunk_emit_opcode(chunk, OP_ARRAY);
      chunk_write_byte(chunk, 0);
      return;
    }
    compile_expression(comp);
    int length = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      ++length;
    }
    EXPECT(scan, TOKEN_RBRACKET);
    chunk_emit_opcode(chunk, OP_ARRAY);
    chunk_write_byte(chunk, length);
    return;
  }
  if (MATCH(scan, TOKEN_VARNAME))
  {
    token_t *tk = &scan->token;
    int index = resolve_local(comp, tk);
    if (index == -1)
      fatal_error("undefined local variable %.*s", tk->length, tk->start);
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_LOAD);
    chunk_write_byte(chunk, index);
    return;
  }
  if (MATCH(scan, TOKEN_LPAREN))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    EXPECT(scan, TOKEN_RPAREN);
    return;
  }
  fatal_error_unexpected_token(scan);
}

void compiler_init(compiler_t *comp, chunk_t *chunk, array_t *consts, scanner_t *scan)
{
  comp->scan = scan;
  comp->chunk = chunk;
  comp->consts = consts;
  comp->local_count = 0;
}

void compiler_compile(compiler_t *comp)
{
  while (!MATCH(comp->scan, TOKEN_EOF))
    compile_statement(comp);
  chunk_emit_opcode(comp->chunk, OP_RETURN);
}
