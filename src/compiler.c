//
// Hook Programming Language
// compiler.c
//

#include "compiler.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "builtin.h"
#include "error.h"

#define COMPILER_MAX_VARIABLES (UINT8_MAX + 1)
#define COMPILER_MAX_BREAKS    (UINT8_MAX + 1)

#define MATCH(s, t) ((s)->token.type == (t))

#define EXPECT(s, t) do \
  { \
    if (!MATCH(s, t)) \
      fatal_error_unexpected_token(s); \
    scanner_next_token(s); \
  } while(0)

typedef struct
{
  bool is_local;
  int depth;
  int index;
  int length;
  char *start;
  bool is_mutable;
} variable_t;

typedef struct loop
{
  struct loop *parent;
  int scope_depth;
  uint16_t jump;
  int num_offsets;
  int offsets[COMPILER_MAX_BREAKS];
} loop_t;

typedef struct compiler
{
  struct compiler *parent;
  scanner_t *scan;
  int scope_depth;
  int num_variables;
  int local_index;
  variable_t variables[COMPILER_MAX_VARIABLES];
  loop_t *loop;
  prototype_t *proto;
} compiler_t;

static inline void fatal_error_unexpected_token(scanner_t *scan);
static inline long parse_long(token_t *tk);
static inline double parse_double(token_t *tk);
static inline void push_scope(compiler_t *comp);
static inline void pop_scope(compiler_t *comp);
static inline int discard_variables(compiler_t *comp, int depth);
static inline bool name_equal(token_t *tk, variable_t *var);
static inline void add_local(compiler_t *comp, int index, token_t *tk, bool is_mutable);
static inline int add_nonlocal(compiler_t *comp, token_t *tk);
static inline void add_variable(compiler_t *comp, bool is_local, int index, token_t *tk,
  bool is_mutable);
static inline void define_local(compiler_t *comp, token_t *tk, bool is_mutable);
static inline variable_t resolve_variable(compiler_t *comp, token_t *tk);
static inline variable_t *lookup_variable(compiler_t *comp, token_t *tk);
static inline bool nonlocal_exists(compiler_t *comp, token_t *tk);
static inline int emit_jump(chunk_t *chunk, opcode_t op);
static inline void patch_jump(chunk_t *chunk, int offset);
static inline void start_loop(compiler_t *comp, loop_t *loop);
static inline void end_loop(compiler_t *comp);
static inline void compiler_init(compiler_t *comp, compiler_t *parent, scanner_t *scan,
  string_t *name);
static void compile_statement(compiler_t *comp);
static void compile_block(compiler_t *comp);
static void compile_let_statement(compiler_t *comp);
static void compile_variable_declaration(compiler_t *comp);
static void compile_assignment(compiler_t *comp, token_t tk);
static void compile_call_statement(compiler_t *comp, token_t tk);
static void compile_function_declaration(compiler_t *comp);
static void compile_delete_statement(compiler_t *comp);
static void compile_if_statement(compiler_t *comp);
static void compile_loop_statement(compiler_t *comp);
static void compile_while_statement(compiler_t *comp);
static void compile_do_statement(compiler_t *comp);
static void compile_for_statement(compiler_t *comp);
static void compile_continue_statement(compiler_t *comp);
static void compile_break_statement(compiler_t *comp);
static void compile_return_statement(compiler_t *comp);
static void compile_expression(compiler_t *comp);
static void compile_and_expression(compiler_t *comp);
static void compile_equal_expression(compiler_t *comp);
static void compile_comp_expression(compiler_t *comp);
static void compile_add_expression(compiler_t *comp);
static void compile_mul_expression(compiler_t *comp);
static void compile_unary_expression(compiler_t *comp);
static void compile_prim_expression(compiler_t *comp);
static void compile_array_constructor(compiler_t *comp);
static void compile_anonymous_function(compiler_t *comp);
static void compile_subscript_or_call(compiler_t *comp);
static void compile_variable(compiler_t *comp, token_t *tk);
static bool compile_nonlocal(compiler_t *comp, token_t *tk);

static inline void fatal_error_unexpected_token(scanner_t *scan)
{
  token_t *tk = &scan->token;
  if (tk->type == TOKEN_EOF)
    fatal_error("unexpected end of file at %d:%d", tk->line, tk->col);
  fatal_error("unexpected token '%.*s' at %d:%d", tk->length, tk->start,
    tk->line, tk->col);
}

static inline long parse_long(token_t *tk)
{
  errno = 0;
  long result = strtol(tk->start, NULL, 10);
  if (errno == ERANGE)
    fatal_error("integer number too large at %d:%d", tk->line, tk->col);
  return result;
}

static inline double parse_double(token_t *tk)
{
  errno = 0;
  double result = strtod(tk->start, NULL);
  if (errno == ERANGE)
    fatal_error("floating point number too large at %d:%d", tk->line, tk->col);
  return result;
}

static inline void push_scope(compiler_t *comp)
{
  ++comp->scope_depth;
}

static inline void pop_scope(compiler_t *comp)
{
  comp->num_variables -= discard_variables(comp, comp->scope_depth);
  --comp->scope_depth;
}

static inline int discard_variables(compiler_t *comp, int depth)
{
  variable_t *variables = comp->variables;
  chunk_t *chunk = &comp->proto->chunk;
  int index = comp->num_variables - 1;
  for (; index > -1 && variables[index].depth >= depth; --index)
    if (variables[index].is_local)
      chunk_emit_opcode(chunk, OP_POP);
  return comp->num_variables - index - 1;
}

static inline bool name_equal(token_t *tk, variable_t *var)
{
  return tk->length == var->length
    && !memcmp(tk->start, var->start, tk->length);
}

static inline void add_local(compiler_t *comp, int index, token_t *tk, bool is_mutable)
{
  add_variable(comp, true, index, tk, is_mutable);
}

static inline int add_nonlocal(compiler_t *comp, token_t *tk)
{
  int index = comp->proto->num_nonlocals++;
  add_variable(comp, false, index, tk, false);
  return index;
}

static inline void add_variable(compiler_t *comp, bool is_local, int index, token_t *tk,
  bool is_mutable)
{
  if (comp->num_variables == COMPILER_MAX_VARIABLES)
    fatal_error("cannot declare more than %d variables in one scope at %d:%d",
      COMPILER_MAX_VARIABLES, tk->line, tk->col);
  variable_t *var = &comp->variables[comp->num_variables];
  var->is_local = is_local;
  var->depth = comp->scope_depth;
  var->index = index;
  var->length = tk->length;
  var->start = tk->start;
  var->is_mutable = is_mutable;
  ++comp->num_variables;
}

static inline void define_local(compiler_t *comp, token_t *tk, bool is_mutable)
{
  for (int i = comp->num_variables - 1; i > -1; --i)
  {
    variable_t *var = &comp->variables[i];
    if (var->depth < comp->scope_depth)
      break;
    if (name_equal(tk, var))
      fatal_error("variable '%.*s' is already defined in this scope at %d:%d",
        tk->length, tk->start, tk->line, tk->col);
  }
  add_local(comp, comp->local_index++, tk, is_mutable);
}

static inline variable_t resolve_variable(compiler_t *comp, token_t *tk)
{
  variable_t *var = lookup_variable(comp, tk);
  if (!var)
    if (nonlocal_exists(comp->parent, tk) || lookup_global(tk->length, tk->start) != -1)
      var = &((variable_t) {.is_mutable = false});
  if (!var)
    fatal_error("variable '%.*s' is used but not defined at %d:%d", tk->length,
      tk->start, tk->line, tk->col);
  return *var;
}

static inline variable_t *lookup_variable(compiler_t *comp, token_t *tk)
{
  for (int i = comp->num_variables - 1; i > -1; --i)
  {
    variable_t *var = &comp->variables[i];
    if (name_equal(tk, var))
      return var;
  }
  return NULL;
}

static inline bool nonlocal_exists(compiler_t *comp, token_t *tk)
{
  if (!comp)
    return false;
  return lookup_variable(comp, tk) || nonlocal_exists(comp->parent, tk);
}

static inline int emit_jump(chunk_t *chunk, opcode_t op)
{
  chunk_emit_opcode(chunk, op);
  int offset = chunk->length;
  chunk_emit_word(chunk, 0);
  return offset;
}

static inline void patch_jump(chunk_t *chunk, int offset)
{
  int jump = chunk->length;
  if (jump > UINT16_MAX)
    fatal_error("code too large");
  *((uint16_t *) &chunk->bytes[offset]) = (uint16_t) jump;
}

static inline void start_loop(compiler_t *comp, loop_t *loop)
{
  loop->parent = comp->loop;
  loop->scope_depth = comp->scope_depth;
  loop->jump = (uint16_t) comp->proto->chunk.length;
  loop->num_offsets = 0;
  comp->loop = loop;
}

static inline void end_loop(compiler_t *comp)
{
  chunk_t *chunk = &comp->proto->chunk;
  loop_t *loop = comp->loop;
  for (int i = 0; i < loop->num_offsets; ++i)
    patch_jump(chunk, loop->offsets[i]);
  comp->loop = comp->loop->parent;
}

static inline void compiler_init(compiler_t *comp, compiler_t *parent, scanner_t *scan,
  string_t *name)
{
  comp->parent = parent;
  comp->scan = scan;
  comp->scope_depth = 0;
  comp->num_variables = 0;
  comp->local_index = 1;
  comp->loop = NULL;
  comp->proto = prototype_new(0, name, scan->file);
}

static void compile_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  if (MATCH(scan, TOKEN_LBRACE))
  {
    compile_block(comp);
    return;
  }
  if (MATCH(scan, TOKEN_LET))
  {
    compile_let_statement(comp);
    EXPECT(scan, TOKEN_SEMICOLON);
    return;
  }
  if (MATCH(scan, TOKEN_VAR))
  {
    compile_variable_declaration(comp);
    EXPECT(scan, TOKEN_SEMICOLON);
    return;
  }
  if (MATCH(scan, TOKEN_NAME))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_LPAREN))
    {
      compile_call_statement(comp, tk);
      return;
    }
    compile_assignment(comp, tk);
    EXPECT(scan, TOKEN_SEMICOLON);
    return;
  }
  if (MATCH(scan, TOKEN_FN))
  {
    compile_function_declaration(comp);
    return;
  }
  if (MATCH(scan, TOKEN_DELETE))
  {
    compile_delete_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_IF))
  {
    compile_if_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_LOOP))
  {
    compile_loop_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_WHILE))
  {
    compile_while_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_DO))
  {
    compile_do_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_FOR))
  {
    compile_for_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_CONTINUE))
  {
    compile_continue_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_BREAK))
  {
    compile_break_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_RETURN))
  {
    compile_return_statement(comp);
    return;
  }
  fatal_error_unexpected_token(scan);
}

static void compile_block(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  scanner_next_token(scan);
  push_scope(comp);
  while (!MATCH(scan, TOKEN_RBRACE))
    compile_statement(comp);
  scanner_next_token(scan);
  pop_scope(comp);
}

static void compile_let_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  scanner_next_token(scan);
  if (MATCH(scan, TOKEN_NAME))
  {
    define_local(comp, &scan->token, false);
    scanner_next_token(scan);
    EXPECT(scan, TOKEN_EQ);
    compile_expression(comp);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    define_local(comp, &scan->token, false);
    scanner_next_token(scan);
    uint8_t n = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        fatal_error_unexpected_token(scan);
      define_local(comp, &scan->token, false);
      scanner_next_token(scan);
      ++n;
    }
    EXPECT(scan, TOKEN_RBRACKET);
    EXPECT(scan, TOKEN_EQ);
    int line = scan->line;
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_UNPACK);
    chunk_emit_byte(chunk, n);
    prototype_add_line(proto, line);
    return;
  }
  fatal_error_unexpected_token(scan);
}

static void compile_variable_declaration(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  scanner_next_token(scan);
  if (MATCH(scan, TOKEN_NAME))
  {
    define_local(comp, &scan->token, true);
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      return;
    }
    chunk_emit_opcode(chunk, OP_NULL);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    define_local(comp, &scan->token, true);
    scanner_next_token(scan);
    uint8_t n = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        fatal_error_unexpected_token(scan);
      define_local(comp, &scan->token, true);
      scanner_next_token(scan);
      ++n;
    }
    EXPECT(scan, TOKEN_RBRACKET);
    EXPECT(scan, TOKEN_EQ);
    int line = scan->line;
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_UNPACK);
    chunk_emit_byte(chunk, n);
    prototype_add_line(proto, line);
    return;
  }
  fatal_error_unexpected_token(scan);
}

static void compile_assignment(compiler_t *comp, token_t tk)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  variable_t var = resolve_variable(comp, &tk);
  if (!var.is_mutable)
    fatal_error("cannot assign to immutable variable '%.*s' at %d:%d",
      tk.length, tk.start, tk.line, tk.col);
  if (MATCH(scan, TOKEN_EQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_SET_LOCAL);
    chunk_emit_byte(chunk, var.index);
    return;
  }
  chunk_emit_opcode(chunk, OP_GET_LOCAL);
  chunk_emit_byte(chunk, var.index);
  if (MATCH(scan, TOKEN_PLUSEQ))
  {
    int line = scan->line;
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_ADD);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_SET_LOCAL);
    chunk_emit_byte(chunk, var.index);
    return;
  }
  if (MATCH(scan, TOKEN_MINUSEQ))
  {
    int line = scan->line;
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_SUBTRACT);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_SET_LOCAL);
    chunk_emit_byte(chunk, var.index);
    return;
  }
  if (MATCH(scan, TOKEN_STAREQ))
  {
    int line = scan->line;
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_MULTIPLY);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_SET_LOCAL);
    chunk_emit_byte(chunk, var.index);
    return;
  }
  if (MATCH(scan, TOKEN_SLASHEQ))
  {
    int line = scan->line;
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_DIVIDE);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_SET_LOCAL);
    chunk_emit_byte(chunk, var.index);
    return;
  }
  if (MATCH(scan, TOKEN_PERCENTEQ))
  {
    int line = scan->line;
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_MODULO);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_SET_LOCAL);
    chunk_emit_byte(chunk, var.index);
    return;
  }
  if (MATCH(scan, TOKEN_PLUSPLUS))
  {
    int line = scan->line;
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_INT);
    chunk_emit_word(chunk, 1);
    chunk_emit_opcode(chunk, OP_ADD);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_SET_LOCAL);
    chunk_emit_byte(chunk, var.index);
    return;
  }
  if (MATCH(scan, TOKEN_MINUSMINUS))
  {
    int line = scan->line;
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_INT);
    chunk_emit_word(chunk, 1);
    chunk_emit_opcode(chunk, OP_SUBTRACT);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_SET_LOCAL);
    chunk_emit_byte(chunk, var.index);
    return;
  }
  int n = 0;
  while (MATCH(scan, TOKEN_LBRACKET))
  {
    int line = scan->line;
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_RBRACKET))
    {
      scanner_next_token(scan);
      EXPECT(scan, TOKEN_EQ);
      compile_expression(comp);
      chunk_emit_opcode(chunk, n ? OP_APPEND : OP_INPLACE_APPEND);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
    compile_expression(comp);
    EXPECT(scan, TOKEN_RBRACKET);
    if (MATCH(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, n ? OP_PUT_ELEMENT : OP_INPLACE_PUT_ELEMENT);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
    chunk_emit_opcode(chunk, OP_FETCH_ELEMENT);
    prototype_add_line(proto, line);
    ++n;
    if (MATCH(scan, TOKEN_PLUSEQ))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, OP_ADD);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
    if (MATCH(scan, TOKEN_MINUSEQ))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, OP_SUBTRACT);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
    if (MATCH(scan, TOKEN_STAREQ))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, OP_MULTIPLY);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
    if (MATCH(scan, TOKEN_SLASHEQ))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, OP_DIVIDE);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
    if (MATCH(scan, TOKEN_PERCENTEQ))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, OP_MODULO);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
    if (MATCH(scan, TOKEN_PLUSPLUS))
    {
      int line = scan->line;
      scanner_next_token(scan);
      chunk_emit_opcode(chunk, OP_INT);
      chunk_emit_word(chunk, 1);
      chunk_emit_opcode(chunk, OP_ADD);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
    if (MATCH(scan, TOKEN_MINUSMINUS))
    {
      int line = scan->line;
      scanner_next_token(scan);
      chunk_emit_opcode(chunk, OP_INT);
      chunk_emit_word(chunk, 1);
      chunk_emit_opcode(chunk, OP_SUBTRACT);
      prototype_add_line(proto, line);
      for (int i = 0; i < n; ++i)
        chunk_emit_opcode(chunk, OP_SET_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_LOCAL);
      chunk_emit_byte(chunk, var.index);
      return;
    }
  }
  fatal_error_unexpected_token(scan);
}

static void compile_call_statement(compiler_t *comp, token_t tk)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  int line = scan->line;
  scanner_next_token(scan);
  compile_variable(comp, &tk);
  if (MATCH(scan, TOKEN_RPAREN))
  {
    scanner_next_token(scan);
    EXPECT(scan, TOKEN_SEMICOLON);
    chunk_emit_opcode(chunk, OP_CALL);
    chunk_emit_byte(chunk, 0);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_POP);
    return;
  }
  compile_expression(comp);
  uint8_t nargs = 1;
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    ++nargs;
  }
  EXPECT(scan, TOKEN_RPAREN);
  EXPECT(scan, TOKEN_SEMICOLON);
  chunk_emit_opcode(chunk, OP_CALL);
  chunk_emit_byte(chunk, nargs);
  prototype_add_line(proto, line);
  chunk_emit_opcode(chunk, OP_POP);
}

static void compile_function_declaration(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  scanner_next_token(scan);
  token_t tk;
  if (!MATCH(scan, TOKEN_NAME))
    fatal_error_unexpected_token(scan);
  tk = scan->token;
  scanner_next_token(scan);
  define_local(comp, &tk, false);
  compiler_t child_comp;
  compiler_init(&child_comp, comp, scan, string_from_chars(tk.length, tk.start));
  add_local(&child_comp, 0, &tk, false);
  chunk_t *child_chunk = &child_comp.proto->chunk;
  EXPECT(scan, TOKEN_LPAREN);
  if (MATCH(scan, TOKEN_RPAREN))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_LBRACE))
      fatal_error_unexpected_token(scan);
    compile_block(&child_comp);
    chunk_emit_opcode(child_chunk, OP_NULL);
    chunk_emit_opcode(child_chunk, OP_RETURN);
    int index = proto->num_protos;
    prototype_add_child(proto, child_comp.proto);
    chunk_emit_opcode(chunk, OP_FUNCTION);
    chunk_emit_byte(chunk, index);
    return;
  }
  if (!MATCH(scan, TOKEN_NAME))
    fatal_error_unexpected_token(scan);
  tk = scan->token;
  scanner_next_token(scan);
  define_local(&child_comp, &tk, false);
  int arity = 1;
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    tk = scan->token;
    scanner_next_token(scan);
    define_local(&child_comp, &tk, false);
    ++arity;
  }
  child_comp.proto->arity = arity;
  EXPECT(scan, TOKEN_RPAREN);
  if (!MATCH(scan, TOKEN_LBRACE))
    fatal_error_unexpected_token(scan);
  compile_block(&child_comp);
  chunk_emit_opcode(child_chunk, OP_NULL);
  chunk_emit_opcode(child_chunk, OP_RETURN);
  int index = proto->num_protos;
  prototype_add_child(proto, child_comp.proto);
  chunk_emit_opcode(chunk, OP_FUNCTION);
  chunk_emit_byte(chunk, index);
}

static void compile_delete_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  scanner_next_token(scan);
  if (!MATCH(scan, TOKEN_NAME))
    fatal_error_unexpected_token(scan);
  token_t tk = scan->token;
  scanner_next_token(scan);
  variable_t var = resolve_variable(comp, &tk);
  if (!var.is_mutable)
    fatal_error("cannot delete element from immutable variable '%.*s' at %d:%d",
      tk.length, tk.start, tk.line, tk.col);
  chunk_emit_opcode(chunk, OP_GET_LOCAL);
  chunk_emit_byte(chunk, var.index);
  int line = scan->line;
  EXPECT(scan, TOKEN_LBRACKET);
  compile_expression(comp);
  EXPECT(scan, TOKEN_RBRACKET);
  int next = scan->line;
  int n = 0;
  while (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_FETCH_ELEMENT);
    prototype_add_line(proto, line);
    compile_expression(comp);
    EXPECT(scan, TOKEN_RBRACKET);
    line = next;
    next = scan->line;
    ++n;
  }
  EXPECT(scan, TOKEN_SEMICOLON);
  chunk_emit_opcode(chunk, n ? OP_DELETE : OP_INPLACE_DELETE);
  prototype_add_line(proto, line);
  for (int i = 0; i < n; ++i)
    chunk_emit_opcode(chunk, OP_SET_ELEMENT);
  chunk_emit_opcode(chunk, OP_SET_LOCAL);
  chunk_emit_byte(chunk, var.index);
}

static void compile_if_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  scanner_next_token(scan);
  EXPECT(scan, TOKEN_LPAREN);
  compile_expression(comp);
  EXPECT(scan, TOKEN_RPAREN);
  int offset1 = emit_jump(chunk, OP_JUMP_IF_FALSE);
  chunk_emit_opcode(chunk, OP_POP);
  compile_statement(comp);
  int offset2 = emit_jump(chunk, OP_JUMP);
  patch_jump(chunk, offset1);
  chunk_emit_opcode(chunk, OP_POP);
  if (MATCH(scan, TOKEN_ELSE))
  {
    scanner_next_token(scan);
    compile_statement(comp);
  }
  patch_jump(chunk, offset2);
}

static void compile_loop_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  scanner_next_token(scan);
  if (!MATCH(scan, TOKEN_LBRACE))
    fatal_error_unexpected_token(scan);
  loop_t loop;
  start_loop(comp, &loop);
  compile_block(comp);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, loop.jump);
  end_loop(comp);
}

static void compile_while_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  scanner_next_token(scan);
  EXPECT(scan, TOKEN_LPAREN);
  loop_t loop;
  start_loop(comp, &loop);
  compile_expression(comp);
  EXPECT(scan, TOKEN_RPAREN);
  int offset = emit_jump(chunk, OP_JUMP_IF_FALSE);
  chunk_emit_opcode(chunk, OP_POP);
  compile_statement(comp);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, loop.jump);
  patch_jump(chunk, offset);
  chunk_emit_opcode(chunk, OP_POP);
  end_loop(comp);
}

static void compile_do_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  scanner_next_token(scan);
  loop_t loop;
  start_loop(comp, &loop);
  compile_statement(comp);
  EXPECT(scan, TOKEN_WHILE);
  EXPECT(scan, TOKEN_LPAREN);
  compile_expression(comp);
  EXPECT(scan, TOKEN_RPAREN);
  EXPECT(scan, TOKEN_SEMICOLON);
  int offset = emit_jump(chunk, OP_JUMP_IF_FALSE);
  chunk_emit_opcode(chunk, OP_POP);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, loop.jump);
  patch_jump(chunk, offset);
  chunk_emit_opcode(chunk, OP_POP);
  end_loop(comp);
}

static void compile_for_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  scanner_next_token(scan);
  EXPECT(scan, TOKEN_LPAREN);
  push_scope(comp);
  if (MATCH(scan, TOKEN_SEMICOLON))
    scanner_next_token(scan);
  else
  {
    if (MATCH(scan, TOKEN_LET))
    {
      compile_let_statement(comp);
      EXPECT(scan, TOKEN_SEMICOLON);
    }
    else if (MATCH(scan, TOKEN_VAR))
    {
      compile_variable_declaration(comp);
      EXPECT(scan, TOKEN_SEMICOLON);
    }
    else if (MATCH(scan, TOKEN_NAME))
    {
      token_t tk = scan->token;
      scanner_next_token(scan);
      compile_assignment(comp, tk);
      EXPECT(scan, TOKEN_SEMICOLON);
    }
    else
      fatal_error_unexpected_token(scan);
  }
  uint16_t jump1 = (uint16_t) chunk->length;
  if (MATCH(scan, TOKEN_SEMICOLON))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_TRUE);
  }
  else
  {
    compile_expression(comp);
    EXPECT(scan, TOKEN_SEMICOLON);
  }
  int offset1 = emit_jump(chunk, OP_JUMP_IF_FALSE);
  chunk_emit_opcode(chunk, OP_POP);
  int offset2 = emit_jump(chunk, OP_JUMP);
  uint16_t jump2 = (uint16_t) chunk->length;
  loop_t loop;
  start_loop(comp, &loop);
  if (MATCH(scan, TOKEN_RPAREN))
    scanner_next_token(scan);
  else
  {
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    token_t tk = scan->token;
    scanner_next_token(scan);
    compile_assignment(comp, tk);
    EXPECT(scan, TOKEN_RPAREN);
  }
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, jump1);  
  patch_jump(chunk, offset2);
  compile_statement(comp);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, jump2);
  patch_jump(chunk, offset1);
  chunk_emit_opcode(chunk, OP_POP);
  end_loop(comp);
  pop_scope(comp);
}

static void compile_continue_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  token_t tk = scan->token;
  scanner_next_token(scan);
  if (!comp->loop)
    fatal_error("cannot use 'continue' outside of a loop at %d:%d", tk.line, tk.col);
  EXPECT(scan, TOKEN_SEMICOLON);
  discard_variables(comp, comp->loop->scope_depth + 1);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, comp->loop->jump);
}

static void compile_break_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  token_t tk = scan->token;
  scanner_next_token(scan);
  if (!comp->loop)
    fatal_error("cannot use 'break' outside of a loop at %d:%d", tk.line, tk.col);
  EXPECT(scan, TOKEN_SEMICOLON);
  discard_variables(comp, comp->loop->scope_depth + 1);
  loop_t *loop = comp->loop;
  if (loop->num_offsets == COMPILER_MAX_BREAKS)
    fatal_error("too many breaks at %d:%d", tk.line, tk.col);
  int offset = emit_jump(&comp->proto->chunk, OP_JUMP);
  loop->offsets[loop->num_offsets++] = offset;
}

static void compile_return_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  scanner_next_token(scan);
  if (MATCH(scan, TOKEN_SEMICOLON))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_NULL);
    chunk_emit_opcode(chunk, OP_RETURN);
    return;
  }
  compile_expression(comp);
  EXPECT(scan, TOKEN_SEMICOLON);
  chunk_emit_opcode(chunk, OP_RETURN);
}

static void compile_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  compile_and_expression(comp);
  while (MATCH(scan, TOKEN_PIPEPIPE))
  {
    scanner_next_token(scan);
    int offset = emit_jump(chunk, OP_JUMP_IF_TRUE);
    chunk_emit_opcode(chunk, OP_POP);
    compile_and_expression(comp);
    patch_jump(chunk, offset);
  }
}

static void compile_and_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  compile_equal_expression(comp);
  while (MATCH(scan, TOKEN_AMPAMP))
  {
    scanner_next_token(scan);
    int offset = emit_jump(chunk, OP_JUMP_IF_FALSE);
    chunk_emit_opcode(chunk, OP_POP);
    compile_equal_expression(comp);
    patch_jump(chunk, offset);
  }
}

static void compile_equal_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  compile_comp_expression(comp);
  for (;;)
  {
    if (MATCH(scan, TOKEN_EQEQ))
    {
      scanner_next_token(scan);
      compile_comp_expression(comp);
      chunk_emit_opcode(chunk, OP_EQUAL);
      continue;
    }
    if (MATCH(scan, TOKEN_BANGEQ))
    {
      scanner_next_token(scan);
      compile_comp_expression(comp);
      chunk_emit_opcode(chunk, OP_EQUAL);
      chunk_emit_opcode(chunk, OP_NOT);
      continue;
    }
    break;
  }
}

static void compile_comp_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  compile_add_expression(comp);
  for (;;)
  {
    if (MATCH(scan, TOKEN_GT))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_GREATER);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_GTEQ))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_LESS);
      prototype_add_line(proto, line);
      chunk_emit_opcode(chunk, OP_NOT);
      continue;
    }
    if (MATCH(scan, TOKEN_LT))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_LESS);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_LTEQ))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_GREATER);
      prototype_add_line(proto, line);
      chunk_emit_opcode(chunk, OP_NOT);
      continue;
    }
    break;
  }
}

static void compile_add_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  compile_mul_expression(comp);
  for (;;)
  {
    if (MATCH(scan, TOKEN_PLUS))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_mul_expression(comp);
      chunk_emit_opcode(chunk, OP_ADD);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_MINUS))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_mul_expression(comp);
      chunk_emit_opcode(chunk, OP_SUBTRACT);
      prototype_add_line(proto, line);
      continue;
    }
    break;
  }
}

static void compile_mul_expression(compiler_t *comp)
{
  compile_unary_expression(comp);
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  for (;;)
  {
    if (MATCH(scan, TOKEN_STAR))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_MULTIPLY);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_SLASH))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_DIVIDE);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_PERCENT))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_MODULO);
      prototype_add_line(proto, line);
      continue;
    }
    break;
  }
}

static void compile_unary_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  if (MATCH(scan, TOKEN_MINUS))
  {
    int line = scan->line;
    scanner_next_token(scan);
    compile_unary_expression(comp);
    chunk_emit_opcode(chunk, OP_NEGATE);
    prototype_add_line(proto, line);
    return;
  }
  if (MATCH(scan, TOKEN_BANG))
  {
    scanner_next_token(scan);
    compile_unary_expression(comp);
    chunk_emit_opcode(chunk, OP_NOT);
    return;
  }
  compile_prim_expression(comp);
}

static void compile_prim_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  array_t *consts = comp->proto->consts;
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
    long data = parse_long(&scan->token);
    scanner_next_token(scan);
    if (data <= UINT16_MAX)
    {
      chunk_emit_opcode(chunk, OP_INT);
      chunk_emit_word(chunk, (uint16_t) data);
      return;
    }
    uint8_t index = (uint8_t) consts->length;
    array_inplace_add_element(consts, NUMBER_VALUE(data));
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    return;
  }
  if (MATCH(scan, TOKEN_FLOAT))
  {
    double data = parse_double(&scan->token);
    scanner_next_token(scan);
    uint8_t index = (uint8_t) consts->length;
    array_inplace_add_element(consts, NUMBER_VALUE(data));
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    return;
  }
  if (MATCH(scan, TOKEN_STRING))
  {
    token_t *tk = &scan->token;
    string_t *str = string_from_chars(tk->length, tk->start);
    scanner_next_token(scan);
    uint8_t index = (uint8_t) consts->length;
    array_inplace_add_element(consts, STRING_VALUE(str));
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    compile_array_constructor(comp);
    return;
  }
  if (MATCH(scan, TOKEN_FN))
  {
    compile_anonymous_function(comp);
    return;
  }
  if (MATCH(scan, TOKEN_NAME))
  {
    compile_subscript_or_call(comp);
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

static void compile_array_constructor(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  scanner_next_token(scan);
  if (MATCH(scan, TOKEN_RBRACKET))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_ARRAY);
    chunk_emit_byte(chunk, 0);
    return;
  }
  compile_expression(comp);
  uint8_t length = 1;
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    ++length;
  }
  EXPECT(scan, TOKEN_RBRACKET);
  chunk_emit_opcode(chunk, OP_ARRAY);
  chunk_emit_byte(chunk, length);
  return;
}

static void compile_anonymous_function(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  scanner_next_token(scan);
  compiler_t child_comp;
  compiler_init(&child_comp, comp, scan, string_from_chars(-1, "anonymous"));
  chunk_t *child_chunk = &child_comp.proto->chunk;
  EXPECT(scan, TOKEN_LPAREN);
  if (MATCH(scan, TOKEN_RPAREN))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_LBRACE))
      fatal_error_unexpected_token(scan);
    compile_block(&child_comp);
    chunk_emit_opcode(child_chunk, OP_NULL);
    chunk_emit_opcode(child_chunk, OP_RETURN);
    int index = proto->num_protos;
    prototype_add_child(proto, child_comp.proto);
    chunk_emit_opcode(chunk, OP_FUNCTION);
    chunk_emit_byte(chunk, index);
    return;
  }
  token_t tk;
  if (!MATCH(scan, TOKEN_NAME))
    fatal_error_unexpected_token(scan);
  tk = scan->token;
  scanner_next_token(scan);
  define_local(&child_comp, &tk, false);
  int arity = 1;
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    tk = scan->token;
    scanner_next_token(scan);
    define_local(&child_comp, &tk, false);
    ++arity;
  }
  child_comp.proto->arity = arity;
  EXPECT(scan, TOKEN_RPAREN);
  if (!MATCH(scan, TOKEN_LBRACE))
    fatal_error_unexpected_token(scan);
  compile_block(&child_comp);
  chunk_emit_opcode(child_chunk, OP_NULL);
  chunk_emit_opcode(child_chunk, OP_RETURN);
  int index = proto->num_protos;
  prototype_add_child(proto, child_comp.proto);
  chunk_emit_opcode(chunk, OP_FUNCTION);
  chunk_emit_byte(chunk, index);
}

static void compile_subscript_or_call(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  compile_variable(comp, &scan->token);
  scanner_next_token(scan);
  for (;;)
  {
    if (MATCH(scan, TOKEN_LBRACKET))
    {
      int line = scan->line;
      scanner_next_token(scan);
      compile_expression(comp);
      EXPECT(scan, TOKEN_RBRACKET);
      chunk_emit_opcode(chunk, OP_GET_ELEMENT);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_LPAREN))
    {
      int line = scan->line;
      scanner_next_token(scan);
      if (MATCH(scan, TOKEN_RPAREN))
      {
        scanner_next_token(scan);
        chunk_emit_opcode(chunk, OP_CALL);
        chunk_emit_byte(chunk, 0);
        prototype_add_line(proto, line);
        return;
      }
      compile_expression(comp);
      uint8_t nargs = 1;
      while (MATCH(scan, TOKEN_COMMA))
      {
        scanner_next_token(scan);
        compile_expression(comp);
        ++nargs;
      }
      EXPECT(scan, TOKEN_RPAREN);
      chunk_emit_opcode(chunk, OP_CALL);
      chunk_emit_byte(chunk, nargs);
      prototype_add_line(proto, line);
      continue;
    }
    break;
  }
}

static void compile_variable(compiler_t *comp, token_t *tk)
{
  chunk_t *chunk = &comp->proto->chunk;
  variable_t *var = lookup_variable(comp, tk);
  if (var)
  {
    chunk_emit_opcode(chunk, var->is_local ? OP_GET_LOCAL : OP_NONLOCAL);
    chunk_emit_byte(chunk, var->index);
    return;
  }
  if (compile_nonlocal(comp->parent, tk))
  {
    int index = add_nonlocal(comp, tk);
    chunk_emit_opcode(chunk, OP_NONLOCAL);
    chunk_emit_byte(chunk, index);
    return;
  }
  int index = lookup_global(tk->length, tk->start);
  if (index == -1)
    fatal_error("variable '%.*s' is used but not defined at %d:%d", tk->length,
      tk->start, tk->line, tk->col);
  chunk_emit_opcode(chunk, OP_GLOBAL);
  chunk_emit_byte(chunk, index);
}

static bool compile_nonlocal(compiler_t *comp, token_t *tk)
{
  if (!comp)
    return false;
  chunk_t *chunk = &comp->proto->chunk;
  variable_t *var = lookup_variable(comp, tk);
  if (var)
  {
    opcode_t op = OP_NONLOCAL;
    if (var->is_local)
    {
      if (var->is_mutable)
        fatal_error("cannot capture mutable variable '%.*s' at %d:%d", tk->length,
          tk->start, tk->line, tk->col);
      op = OP_GET_LOCAL;
    }
    chunk_emit_opcode(chunk, op);
    chunk_emit_byte(chunk, var->index);
    return true;
  }
  if (compile_nonlocal(comp->parent, tk))
  {
    int index = add_nonlocal(comp, tk);
    chunk_emit_opcode(chunk, OP_NONLOCAL);
    chunk_emit_byte(chunk, index);
    return true;
  }
  return false;
}

prototype_t *compile(scanner_t *scan)
{
  compiler_t comp;
  compiler_init(&comp, NULL, scan, string_from_chars(-1, "main"));
  while (!MATCH(comp.scan, TOKEN_EOF))
    compile_statement(&comp);
  chunk_t *chunk = &comp.proto->chunk;
  chunk_emit_opcode(chunk, OP_NULL);
  chunk_emit_opcode(chunk, OP_RETURN);
  return comp.proto;
}
