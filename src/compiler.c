//
// Hook Programming Language
// compiler.c
//

#include "compiler.h"
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "error.h"

#define MATCH(s, t) ((s)->token.type == (t))

#define EXPECT(s, t) do \
  { \
    if (!MATCH(s, t)) \
      fatal_error_unexpected_token(s); \
    scanner_next_token(s); \
  } while(0)

#define EXPECT_SEMICOLON(s) do \
  { \
    if (!MATCH(s, TOKEN_SEMICOLON)) \
      fatal_error_unexpected_token(s); \
    scanner_next_token(s); \
  } while(0)

static inline void fatal_error_unexpected_token(scanner_t *scan);
static inline uint16_t parse_word(char *chars);
static void compile_statement(chunk_t *chunk, scanner_t *scan);
static void compile_echo(chunk_t *chunk, scanner_t *scan);
static void compile_expression(chunk_t *chunk, scanner_t *scan);
static void compile_mul_expression(chunk_t *chunk, scanner_t *scan);
static void compile_unary_expression(chunk_t *chunk, scanner_t *scan);
static void compile_prim_expression(chunk_t *chunk, scanner_t *scan);

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

static inline uint16_t parse_word(char *chars)
{
  errno = 0;
  unsigned long result = strtoul(chars, NULL, 10);
  if (errno == ERANGE || result > UINT16_MAX)
    fatal_error("integer number too large");
  return (uint16_t) result;
}

static void compile_statement(chunk_t *chunk, scanner_t *scan)
{
  if (MATCH(scan, TOKEN_ECHO))
  {
    scanner_next_token(scan);
    compile_echo(chunk, scan);
    return;
  }
  fatal_error_unexpected_token(scan);
}

static void compile_echo(chunk_t *chunk, scanner_t *scan)
{
  compile_expression(chunk, scan);
  chunk_emit_opcode(chunk, OP_PRINT);
  EXPECT_SEMICOLON(scan);
}

static void compile_expression(chunk_t *chunk, scanner_t *scan)
{
  compile_mul_expression(chunk, scan);
  for (;;)
  {
    if (MATCH(scan, TOKEN_PLUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(chunk, scan);
      chunk_emit_opcode(chunk, OP_ADD);
      continue;
    }
    if (MATCH(scan, TOKEN_MINUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(chunk, scan);
      chunk_emit_opcode(chunk, OP_SUBTRACT);
      continue;
    }
    break;
  }
}

static void compile_mul_expression(chunk_t *chunk, scanner_t *scan)
{
  compile_unary_expression(chunk, scan);
  for (;;)
  {
    if (MATCH(scan, TOKEN_STAR))
    {
      scanner_next_token(scan);
      compile_unary_expression(chunk, scan);
      chunk_emit_opcode(chunk, OP_MULTIPLY);
      continue;
    }
    if (MATCH(scan, TOKEN_SLASH))
    {
      scanner_next_token(scan);
      compile_unary_expression(chunk, scan);
      chunk_emit_opcode(chunk, OP_DIVIDE);
      continue;
    }
    if (MATCH(scan, TOKEN_PERCENT))
    {
      scanner_next_token(scan);
      compile_unary_expression(chunk, scan);
      chunk_emit_opcode(chunk, OP_MODULO);
      continue;
    }
    break;
  }
}

static void compile_unary_expression(chunk_t *chunk, scanner_t *scan)
{
  if (MATCH(scan, TOKEN_PLUS))
  {
    scanner_next_token(scan);
    compile_unary_expression(chunk, scan);
    return;
  }
  if (MATCH(scan, TOKEN_MINUS))
  {
    scanner_next_token(scan);
    compile_unary_expression(chunk, scan);
    chunk_emit_opcode(chunk, OP_NEGATE);
    return;
  }
  compile_prim_expression(chunk, scan);
}

static void compile_prim_expression(chunk_t *chunk, scanner_t *scan)
{
  if (MATCH(scan, TOKEN_INT))
  {
    uint16_t word = parse_word(scan->token.chars);
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_INT);
    chunk_write_word(chunk, word);
    return;
  }
  if (MATCH(scan, TOKEN_LPAREN))
  {
    scanner_next_token(scan);
    compile_expression(chunk, scan);
    EXPECT(scan, TOKEN_RPAREN);
    return;
  }
  fatal_error_unexpected_token(scan);
}

void compile(chunk_t *chunk, scanner_t *scan)
{
  while (!MATCH(scan, TOKEN_EOF))
    compile_statement(chunk, scan);
  chunk_emit_opcode(chunk, OP_RETURN);
}
