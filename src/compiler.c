//
// Hook Programming Language
// compiler.c
//

#include "compiler.h"
#include <stdlib.h>
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
static void compile_statement(chunk_t *chunk, array_t *consts, scanner_t *scan);
static void compile_echo(chunk_t *chunk, array_t *consts, scanner_t *scan);
static void compile_expression(chunk_t *chunk, array_t *consts, scanner_t *scan);
static void compile_mul_expression(chunk_t *chunk, array_t *consts, scanner_t *scan);
static void compile_unary_expression(chunk_t *chunk, array_t *consts, scanner_t *scan);
static void compile_prim_expression(chunk_t *chunk, array_t *consts, scanner_t *scan);

static inline void fatal_error_unexpected_token(scanner_t *scan)
{
  token_t *tk = &scan->token;
  if (tk->type == TOKEN_EOF)
  {
    const char *fmt = "unexpected end of file at %d:%d";
    fatal_error(fmt, tk->line, tk->col);
  }
  const char *fmt = "unexpected token '%.*s' at %d:%d";
  fatal_error(fmt, tk->length, tk->chars, tk->line, tk->col);
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

static void compile_statement(chunk_t *chunk, array_t *consts, scanner_t *scan)
{
  if (MATCH(scan, TOKEN_ECHO))
  {
    scanner_next_token(scan);
    compile_echo(chunk, consts, scan);
    return;
  }
  fatal_error_unexpected_token(scan);
}

static void compile_echo(chunk_t *chunk, array_t *consts, scanner_t *scan)
{
  compile_expression(chunk, consts, scan);
  chunk_emit_opcode(chunk, OP_PRINT);
  EXPECT(scan, TOKEN_SEMICOLON);
}

static void compile_expression(chunk_t *chunk, array_t *consts, scanner_t *scan)
{
  compile_mul_expression(chunk, consts, scan);
  for (;;)
  {
    if (MATCH(scan, TOKEN_PLUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(chunk, consts, scan);
      chunk_emit_opcode(chunk, OP_ADD);
      continue;
    }
    if (MATCH(scan, TOKEN_MINUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(chunk, consts, scan);
      chunk_emit_opcode(chunk, OP_SUBTRACT);
      continue;
    }
    break;
  }
}

static void compile_mul_expression(chunk_t *chunk, array_t *consts, scanner_t *scan)
{
  compile_unary_expression(chunk, consts, scan);
  for (;;)
  {
    if (MATCH(scan, TOKEN_STAR))
    {
      scanner_next_token(scan);
      compile_unary_expression(chunk, consts, scan);
      chunk_emit_opcode(chunk, OP_MULTIPLY);
      continue;
    }
    if (MATCH(scan, TOKEN_SLASH))
    {
      scanner_next_token(scan);
      compile_unary_expression(chunk, consts, scan);
      chunk_emit_opcode(chunk, OP_DIVIDE);
      continue;
    }
    if (MATCH(scan, TOKEN_PERCENT))
    {
      scanner_next_token(scan);
      compile_unary_expression(chunk, consts, scan);
      chunk_emit_opcode(chunk, OP_MODULO);
      continue;
    }
    break;
  }
}

static void compile_unary_expression(chunk_t *chunk, array_t *consts, scanner_t *scan)
{
  if (MATCH(scan, TOKEN_PLUS))
  {
    scanner_next_token(scan);
    compile_unary_expression(chunk, consts, scan);
    return;
  }
  if (MATCH(scan, TOKEN_MINUS))
  {
    scanner_next_token(scan);
    compile_unary_expression(chunk, consts, scan);
    chunk_emit_opcode(chunk, OP_NEGATE);
    return;
  }
  compile_prim_expression(chunk, consts, scan);
}

static void compile_prim_expression(chunk_t *chunk, array_t *consts, scanner_t *scan)
{
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
  if (MATCH(scan, TOKEN_INT))
  {
    long data = parse_long(scan->token.chars);
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
    double data = parse_double(scan->token.chars);
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
    string_t *str = string_from_chars(tk->length, tk->chars);
    scanner_next_token(scan);
    int index = consts->length;
    array_add_element(consts, STRING_VALUE(str));
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_write_byte(chunk, index);
    return;
  }
  if (MATCH(scan, TOKEN_LPAREN))
  {
    scanner_next_token(scan);
    compile_expression(chunk, consts, scan);
    EXPECT(scan, TOKEN_RPAREN);
    return;
  }
  fatal_error_unexpected_token(scan);
}

void compile(chunk_t *chunk, array_t *consts, scanner_t *scan)
{
  while (!MATCH(scan, TOKEN_EOF))
    compile_statement(chunk, consts, scan);
  chunk_emit_opcode(chunk, OP_RETURN);
}
