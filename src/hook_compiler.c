//
// Hook Programming Language
// hook_compiler.c
//

#include "hook_compiler.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include "hook_scanner.h"
#include "hook_struct.h"
#include "hook_builtin.h"

#define MAX_CONSTANTS UINT8_MAX
#define MAX_VARIABLES UINT8_MAX
#define MAX_BREAKS    UINT8_MAX

#define match(s, t) ((s)->token.type == (t))

#define consume(c, t) do \
  { \
    scanner_t *scan = (c)->scan; \
    if (!match(scan, t)) \
      syntax_error_unexpected(c); \
    scanner_next_token(scan); \
  } while(0)

#define SYN_NONE      0x00
#define SYN_ASSIGN    0x01
#define SYN_CALL      0x02
#define SYN_SUBSCRIPT 0x03

typedef struct
{
  bool is_local;
  int32_t depth;
  uint8_t index;
  int32_t length;
  char *start;
  bool is_mutable;
} variable_t;

typedef struct loop
{
  struct loop *parent;
  int32_t scope_depth;
  uint16_t jump;
  int32_t num_offsets;
  int32_t offsets[MAX_BREAKS];
} loop_t;

typedef struct compiler
{
  struct compiler *parent;
  scanner_t *scan;
  int32_t scope_depth;
  int32_t num_variables;
  uint8_t local_index;
  variable_t variables[MAX_VARIABLES];
  loop_t *loop;
  hk_function_t *fn;
} compiler_t;

static inline void syntax_error(const char *function, const char *file, int32_t line,
  int32_t col, const char *fmt, ...);
static inline void syntax_error_unexpected(compiler_t *comp);
static inline double parse_double(compiler_t *comp);
static inline bool string_match(token_t *tk, hk_string_t *str);
static inline uint8_t add_float_constant(compiler_t *comp, double data);
static inline uint8_t add_string_constant(compiler_t *comp, token_t *tk);
static inline uint8_t add_constant(compiler_t *comp, hk_value_t val);
static inline void push_scope(compiler_t *comp);
static inline void pop_scope(compiler_t *comp);
static inline int32_t discard_variables(compiler_t *comp, int32_t depth);
static inline bool variable_match(token_t *tk, variable_t *var);
static inline void add_local(compiler_t *comp, token_t *tk, bool is_mutable);
static inline uint8_t add_nonlocal(compiler_t *comp, token_t *tk);
static inline void add_variable(compiler_t *comp, bool is_local, uint8_t index, token_t *tk,
  bool is_mutable);
static inline void define_local(compiler_t *comp, token_t *tk, bool is_mutable);
static inline variable_t resolve_variable(compiler_t *comp, token_t *tk);
static inline variable_t *lookup_variable(compiler_t *comp, token_t *tk);
static inline bool nonlocal_exists(compiler_t *comp, token_t *tk);
static inline int32_t emit_jump(hk_chunk_t *chunk, int32_t op);
static inline void patch_jump(compiler_t *comp, int32_t offset);
static inline void patch_opcode(hk_chunk_t *chunk, int32_t offset, int32_t op);
static inline void start_loop(compiler_t *comp, loop_t *loop);
static inline void end_loop(compiler_t *comp);
static inline void compiler_init(compiler_t *comp, compiler_t *parent, scanner_t *scan,
  hk_string_t *name);
static void compile_statement(compiler_t *comp);
static void compile_load_module(compiler_t *comp);
static void compile_constant_declaration(compiler_t *comp);
static void compile_variable_declaration(compiler_t *comp);
static void compile_assign_statement(compiler_t *comp, token_t *tk);
static int32_t compile_assign(compiler_t *comp, int32_t syntax, bool inplace);
static void compile_struct_declaration(compiler_t *comp, bool is_anonymous);
static void compile_function_declaration(compiler_t *comp, bool is_anonymous);
static void compile_del_statement(compiler_t *comp);
static void compile_delete(compiler_t *comp, bool inplace);
static void compile_if_statement(compiler_t *comp);
static void compile_match_statement(compiler_t *comp);
static void compile_match_statement_member(compiler_t *comp);
static void compile_loop_statement(compiler_t *comp);
static void compile_while_statement(compiler_t *comp);
static void compile_do_statement(compiler_t *comp);
static void compile_for_statement(compiler_t *comp);
static void compile_continue_statement(compiler_t *comp);
static void compile_break_statement(compiler_t *comp);
static void compile_return_statement(compiler_t *comp);
static void compile_block(compiler_t *comp);
static void compile_expression(compiler_t *comp);
static void compile_and_expression(compiler_t *comp);
static void compile_equal_expression(compiler_t *comp);
static void compile_comp_expression(compiler_t *comp);
static void compile_add_expression(compiler_t *comp);
static void compile_range_expression(compiler_t *comp);
static void compile_mul_expression(compiler_t *comp);
static void compile_unary_expression(compiler_t *comp);
static void compile_prim_expression(compiler_t *comp);
static void compile_array_constructor(compiler_t *comp);
static void compile_struct_constructor(compiler_t *comp);
static void compile_if_expression(compiler_t *comp);
static void compile_match_expression(compiler_t *comp);
static void compile_match_expression_member(compiler_t *comp);
static void compile_subscript(compiler_t *comp);
static variable_t compile_variable(compiler_t *comp, token_t *tk, bool emit);
static variable_t *compile_nonlocal(compiler_t *comp, token_t *tk);

static inline void syntax_error(const char *function, const char *file, int32_t line,
  int32_t col, const char *fmt, ...)
{
  fprintf(stderr, "syntax error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n  at %s() in %s:%d,%d\n", function, file, line, col);
  exit(EXIT_FAILURE);
}

static inline void syntax_error_unexpected(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  token_t *tk = &scan->token;
  char *function = comp->fn->name->chars;
  char *file = scan->file->chars;
  if (tk->type == TOKEN_EOF)
    syntax_error(function, file, tk->line, tk->col, "unexpected end of file");
  syntax_error(function, file, tk->line, tk->col, "unexpected token `%.*s`",
    tk->length, tk->start);
}

static inline double parse_double(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  token_t *tk = &scan->token;
  errno = 0;
  double result = strtod(tk->start, NULL);
  if (errno == ERANGE)
    syntax_error(comp->fn->name->chars, scan->file->chars, tk->line, tk->col,
      "floating point number `%.*s` out of range", tk->length, tk->start);
  return result;
}

static inline bool string_match(token_t *tk, hk_string_t *str)
{
  return tk->length == str->length
    && !memcmp(tk->start, str->chars, tk->length);
}

static inline uint8_t add_float_constant(compiler_t *comp, double data)
{
  hk_array_t *consts = comp->fn->consts;
  hk_value_t *elements = consts->elements;
  for (int32_t i = 0; i < consts->length; ++i)
  {
    hk_value_t elem = elements[i];
    if (!hk_is_float(elem))
      continue;
    if (data == elem.as_float)
      return (uint8_t) i;
  }
  return add_constant(comp, hk_float_value(data));
}

static inline uint8_t add_string_constant(compiler_t *comp, token_t *tk)
{
  hk_array_t *consts = comp->fn->consts;
  hk_value_t *elements = consts->elements;
  for (int32_t i = 0; i < consts->length; ++i)
  {
    hk_value_t elem = elements[i];
    if (!hk_is_string(elem))
      continue;
    if (string_match(tk, hk_as_string(elem)))
      return (uint8_t) i;
  }
  hk_string_t *str = hk_string_from_chars(tk->length, tk->start);
  return add_constant(comp, hk_string_value(str));
}

static inline uint8_t add_constant(compiler_t *comp, hk_value_t val)
{
  hk_function_t *fn = comp->fn;
  hk_array_t *consts = fn->consts;
  scanner_t *scan = comp->scan;
  token_t *tk = &scan->token;
  if (consts->length == MAX_CONSTANTS)
    syntax_error(fn->name->chars, scan->file->chars, tk->line, tk->col,
      "a function may only contain %d unique constants", MAX_CONSTANTS);
  uint8_t index = (uint8_t) consts->length;
  hk_array_inplace_add_element(consts, val);
  return index;
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

static inline int32_t discard_variables(compiler_t *comp, int32_t depth)
{
  variable_t *variables = comp->variables;
  hk_chunk_t *chunk = &comp->fn->chunk;
  int32_t index = comp->num_variables - 1;
  for (; index > -1 && variables[index].depth >= depth; --index)
    if (variables[index].is_local)
      hk_chunk_emit_opcode(chunk, HK_OP_POP);
  return comp->num_variables - index - 1;
}

static inline bool variable_match(token_t *tk, variable_t *var)
{
  return tk->length == var->length
    && !memcmp(tk->start, var->start, tk->length);
}

static inline void add_local(compiler_t *comp, token_t *tk, bool is_mutable)
{
  uint8_t index = comp->local_index++;
  add_variable(comp, true, index, tk, is_mutable);
}

static inline uint8_t add_nonlocal(compiler_t *comp, token_t *tk)
{
  uint8_t index = comp->fn->num_nonlocals++;
  add_variable(comp, false, index, tk, false);
  return index;
}

static inline void add_variable(compiler_t *comp, bool is_local, uint8_t index, token_t *tk,
  bool is_mutable)
{
  if (comp->num_variables == MAX_VARIABLES)
    syntax_error(comp->fn->name->chars, comp->scan->file->chars, tk->line, tk->col,
      "a function may only contain %d unique variables", MAX_VARIABLES);
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
  for (int32_t i = comp->num_variables - 1; i > -1; --i)
  {
    variable_t *var = &comp->variables[i];
    if (var->depth < comp->scope_depth)
      break;
    if (variable_match(tk, var))
      syntax_error(comp->fn->name->chars, comp->scan->file->chars, tk->line, tk->col,
        "variable `%.*s` is already defined in this scope", tk->length, tk->start);
  }
  add_local(comp, tk, is_mutable);
}

static inline variable_t resolve_variable(compiler_t *comp, token_t *tk)
{
  variable_t *var = lookup_variable(comp, tk);
  if (var)
    return *var;
  if (!nonlocal_exists(comp->parent, tk) && lookup_global(tk->length, tk->start) == -1)
    syntax_error(comp->fn->name->chars, comp->scan->file->chars, tk->line, tk->col,
      "variable `%.*s` is used but not defined", tk->length, tk->start);
  return (variable_t) {.is_mutable = false};
}

static inline variable_t *lookup_variable(compiler_t *comp, token_t *tk)
{
  for (int32_t i = comp->num_variables - 1; i > -1; --i)
  {
    variable_t *var = &comp->variables[i];
    if (variable_match(tk, var))
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

static inline int32_t emit_jump(hk_chunk_t *chunk, int32_t op)
{
  hk_chunk_emit_opcode(chunk, op);
  int32_t offset = chunk->length;
  hk_chunk_emit_word(chunk, 0);
  return offset;
}

static inline void patch_jump(compiler_t *comp, int32_t offset)
{
  hk_chunk_t *chunk = &comp->fn->chunk;
  scanner_t *scan = comp->scan;
  token_t *tk = &scan->token;
  int32_t jump = chunk->length;
  if (jump > UINT16_MAX)
    syntax_error(comp->fn->name->chars, scan->file->chars, tk->line, tk->col,
      "code too large");
  *((uint16_t *) &chunk->bytes[offset]) = (uint16_t) jump;
}

static inline void patch_opcode(hk_chunk_t *chunk, int32_t offset, int32_t op)
{
  chunk->bytes[offset] = (uint8_t) op;
}

static inline void start_loop(compiler_t *comp, loop_t *loop)
{
  loop->parent = comp->loop;
  loop->scope_depth = comp->scope_depth;
  loop->jump = (uint16_t) comp->fn->chunk.length;
  loop->num_offsets = 0;
  comp->loop = loop;
}

static inline void end_loop(compiler_t *comp)
{
  loop_t *loop = comp->loop;
  for (int32_t i = 0; i < loop->num_offsets; ++i)
    patch_jump(comp, loop->offsets[i]);
  comp->loop = comp->loop->parent;
}

static inline void compiler_init(compiler_t *comp, compiler_t *parent, scanner_t *scan,
  hk_string_t *name)
{
  comp->parent = parent;
  comp->scan = scan;
  comp->scope_depth = 0;
  comp->num_variables = 0;
  comp->local_index = 1;
  comp->loop = NULL;
  comp->fn = hk_function_new(0, name, scan->file);
}

static void compile_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  if (match(scan, TOKEN_USE))
  {
    compile_load_module(comp);
    return;
  }
  if (match(scan, TOKEN_VAL))
  {
    compile_constant_declaration(comp);
    consume(comp, TOKEN_SEMICOLON);
    return;
  }
  if (match(scan, TOKEN_MUT))
  {
    compile_variable_declaration(comp);
    consume(comp, TOKEN_SEMICOLON);
    return;
  }
  if (match(scan, TOKEN_NAME))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    compile_assign_statement(comp, &tk);
    consume(comp, TOKEN_SEMICOLON);
    return;
  }
  if (match(scan, TOKEN_STRUCT))
  {
    compile_struct_declaration(comp, false);
    return;
  }
  if (match(scan, TOKEN_FN))
  {
    compile_function_declaration(comp, false);
    return;
  }
  if (match(scan, TOKEN_DEL))
  {
    compile_del_statement(comp);
    return;
  }
  if (match(scan, TOKEN_IF))
  {
    compile_if_statement(comp);
    return;
  }
  if (match(scan, TOKEN_MATCH))
  {
    compile_match_statement(comp);
    return;
  }
  if (match(scan, TOKEN_LOOP))
  {
    compile_loop_statement(comp);
    return;
  }
  if (match(scan, TOKEN_WHILE))
  {
    compile_while_statement(comp);
    return;
  }
  if (match(scan, TOKEN_DO))
  {
    compile_do_statement(comp);
    return;
  }
  if (match(scan, TOKEN_FOR))
  {
    compile_for_statement(comp);
    return;
  }
  if (match(scan, TOKEN_CONTINUE))
  {
    compile_continue_statement(comp);
    return;
  }
  if (match(scan, TOKEN_BREAK))
  {
    compile_break_statement(comp);
    return;
  }
  if (match(scan, TOKEN_RETURN))
  {
    compile_return_statement(comp);
    return;
  }
  if (match(scan, TOKEN_LBRACE))
  {
    compile_block(comp);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_load_module(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (match(scan, TOKEN_NAME))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    if (match(scan, TOKEN_AS))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      tk = scan->token;
      scanner_next_token(scan);
    }
    define_local(comp, &tk, false);
    consume(comp, TOKEN_SEMICOLON);
    hk_chunk_emit_opcode(chunk, HK_OP_LOAD_MODULE);
    hk_function_add_line(fn, tk.line);
    return;
  }
  if (match(scan, TOKEN_LBRACE))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    uint8_t n = 1;
    while (match(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      token_t tk = scan->token;
      scanner_next_token(scan);
      define_local(comp, &tk, false);
      uint8_t index = add_string_constant(comp, &tk);
      hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
      hk_chunk_emit_byte(chunk, index);
      hk_function_add_line(fn, tk.line);
      ++n;
    }
    consume(comp, TOKEN_RBRACE);
    consume(comp, TOKEN_IN);
    int32_t line = scan->token.line;
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    consume(comp, TOKEN_SEMICOLON);
    index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    hk_chunk_emit_opcode(chunk, HK_OP_LOAD_MODULE);
    hk_function_add_line(fn, tk.line);
    hk_chunk_emit_opcode(chunk, HK_OP_DESTRUCT);
    hk_chunk_emit_byte(chunk, n);
    hk_function_add_line(fn, line);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_constant_declaration(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (match(scan, TOKEN_NAME))
  {
    define_local(comp, &scan->token, false);
    scanner_next_token(scan);
    consume(comp, TOKEN_EQ);
    compile_expression(comp);
    return;
  }
  if (match(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    define_local(comp, &scan->token, false);
    scanner_next_token(scan);
    uint8_t n = 1;
    while (match(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      define_local(comp, &scan->token, false);
      scanner_next_token(scan);
      ++n;
    }
    consume(comp, TOKEN_RBRACKET);
    consume(comp, TOKEN_EQ);
    int32_t line = scan->token.line;
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_UNPACK);
    hk_chunk_emit_byte(chunk, n);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_LBRACE))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    uint8_t n = 1;
    while (match(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      token_t tk = scan->token;
      scanner_next_token(scan);
      define_local(comp, &tk, false);
      uint8_t index = add_string_constant(comp, &tk);
      hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
      hk_chunk_emit_byte(chunk, index);
      hk_function_add_line(fn, tk.line);
      ++n;
    }
    consume(comp, TOKEN_RBRACE);
    consume(comp, TOKEN_EQ);
    int32_t line = scan->token.line;
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_DESTRUCT);
    hk_chunk_emit_byte(chunk, n);
    hk_function_add_line(fn, line);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_variable_declaration(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (match(scan, TOKEN_NAME))
  {
    define_local(comp, &scan->token, true);
    scanner_next_token(scan);
    if (match(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      return;  
    }
    hk_chunk_emit_opcode(chunk, HK_OP_NIL);
    hk_function_add_line(fn, scan->token.line);
    return;
  }
  if (match(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    define_local(comp, &scan->token, true);
    scanner_next_token(scan);
    uint8_t n = 1;
    while (match(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      define_local(comp, &scan->token, true);
      scanner_next_token(scan);
      ++n;
    }
    consume(comp, TOKEN_RBRACKET);
    consume(comp, TOKEN_EQ);
    int32_t line = scan->token.line;
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_UNPACK);
    hk_chunk_emit_byte(chunk, n);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_LBRACE))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, true);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    uint8_t n = 1;
    while (match(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      token_t tk = scan->token;
      scanner_next_token(scan);
      define_local(comp, &tk, true);
      uint8_t index = add_string_constant(comp, &tk);
      hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
      hk_chunk_emit_byte(chunk, index);
      hk_function_add_line(fn, tk.line);
      ++n;
    }
    consume(comp, TOKEN_RBRACE);
    consume(comp, TOKEN_EQ);
    int32_t line = scan->token.line;
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_DESTRUCT);
    hk_chunk_emit_byte(chunk, n);
    hk_function_add_line(fn, line);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_assign_statement(compiler_t *comp, token_t *tk)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  variable_t var;
  if (match(scan, TOKEN_EQ))
  {
    var = compile_variable(comp, tk, false);
    scanner_next_token(scan);
    compile_expression(comp);
    goto end;
  }
  var = compile_variable(comp, tk, true);
  if (compile_assign(comp, SYN_NONE, true) == SYN_CALL)
  {
    hk_chunk_emit_opcode(chunk, HK_OP_POP);
    return;
  }
end:
  if (!var.is_mutable)
    syntax_error(fn->name->chars, scan->file->chars, tk->line, tk->col,
      "cannot assign to immutable variable `%.*s`", tk->length, tk->start);
  hk_chunk_emit_opcode(chunk, HK_OP_SET_LOCAL);
  hk_chunk_emit_byte(chunk, var.index);
}

static int32_t compile_assign(compiler_t *comp, int32_t syntax, bool inplace)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  int32_t line = scan->token.line;
  if (match(scan, TOKEN_PLUSEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_ADD);
    hk_function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (match(scan, TOKEN_MINUSEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_SUBTRACT);
    hk_function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (match(scan, TOKEN_STAREQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_MULTIPLY);
    hk_function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (match(scan, TOKEN_SLASHEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_DIVIDE);
    hk_function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (match(scan, TOKEN_PERCENTEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_MODULO);
    hk_function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (match(scan, TOKEN_PLUSPLUS))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_INCR);
    return SYN_ASSIGN;
  }
  if (match(scan, TOKEN_MINUSMINUS))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_DECR);
    return SYN_ASSIGN;
  }
  if (match(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_RBRACKET))
    {
      scanner_next_token(scan);
      consume(comp, TOKEN_EQ);
      compile_expression(comp);
      hk_chunk_emit_opcode(chunk, inplace ? HK_OP_INPLACE_ADD_ELEMENT : HK_OP_ADD_ELEMENT);
      hk_function_add_line(fn, line);
      return SYN_ASSIGN;
    }
    compile_expression(comp);
    consume(comp, TOKEN_RBRACKET);
    if (match(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      hk_chunk_emit_opcode(chunk, inplace ? HK_OP_INPLACE_PUT_ELEMENT : HK_OP_PUT_ELEMENT);
      hk_function_add_line(fn, line);
      return SYN_ASSIGN;
    }
    int32_t offset = chunk->length;
    hk_chunk_emit_opcode(chunk, HK_OP_GET_ELEMENT);
    hk_function_add_line(fn, line);
    int32_t syn = compile_assign(comp, SYN_SUBSCRIPT, false);
    if (syn == SYN_ASSIGN)
    {
      patch_opcode(chunk, offset, HK_OP_FETCH_ELEMENT);
      hk_chunk_emit_opcode(chunk, HK_OP_SET_ELEMENT);
    }
    return syn;
  }
  if (match(scan, TOKEN_DOT))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    if (match(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      hk_chunk_emit_opcode(chunk, inplace ? HK_OP_INPLACE_PUT_FIELD : HK_OP_PUT_FIELD);
      hk_chunk_emit_byte(chunk, index);
      hk_function_add_line(fn, tk.line);
      return SYN_ASSIGN;
    }
    int32_t offset = chunk->length;
    hk_chunk_emit_opcode(chunk, HK_OP_GET_FIELD);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    int32_t syn = compile_assign(comp, SYN_SUBSCRIPT, false);
    if (syn == SYN_ASSIGN)
    {
      patch_opcode(chunk, offset, HK_OP_FETCH_FIELD);
      hk_chunk_emit_opcode(chunk, HK_OP_SET_FIELD);
    }
    return syn;
  }
  if (match(scan, TOKEN_LPAREN))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_RPAREN))
    {
      scanner_next_token(scan);
      hk_chunk_emit_opcode(chunk, HK_OP_CALL);
      hk_chunk_emit_byte(chunk, 0);
      hk_function_add_line(fn, line);
      return compile_assign(comp, SYN_CALL, false);
    }
    compile_expression(comp);
    uint8_t num_args = 1;
    while (match(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      ++num_args;
    }
    consume(comp, TOKEN_RPAREN);
    hk_chunk_emit_opcode(chunk, HK_OP_CALL);
    hk_chunk_emit_byte(chunk, num_args);
    hk_function_add_line(fn, line);
    return compile_assign(comp, SYN_CALL, false);
  }
  if (syntax == SYN_NONE || syntax == SYN_SUBSCRIPT)
    syntax_error_unexpected(comp);
  return syntax;
}

static void compile_struct_declaration(compiler_t *comp, bool is_anonymous)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  int32_t line = scan->token.line;
  scanner_next_token(scan);
  token_t tk;
  uint8_t index;
  if (is_anonymous)
  {
    hk_chunk_emit_opcode(chunk, HK_OP_NIL);
    hk_function_add_line(fn, line);
  }
  else
  {
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
  }
  consume(comp, TOKEN_LBRACE);
  if (match(scan, TOKEN_RBRACE))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_STRUCT);
    hk_chunk_emit_byte(chunk, 0);
    hk_function_add_line(fn, line);
    return;
  }
  if (!match(scan, TOKEN_NAME))
    syntax_error_unexpected(comp);
  tk = scan->token;
  scanner_next_token(scan);
  index = add_string_constant(comp, &tk);
  hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
  hk_chunk_emit_byte(chunk, index);
  hk_function_add_line(fn, tk.line);
  uint8_t length = 1;
  while (match(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    ++length;
  }
  consume(comp, TOKEN_RBRACE);
  hk_chunk_emit_opcode(chunk, HK_OP_STRUCT);
  hk_chunk_emit_byte(chunk, length);
  hk_function_add_line(fn, line);
}

static void compile_function_declaration(compiler_t *comp, bool is_anonymous)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  int32_t line = scan->token.line;
  scanner_next_token(scan);
  compiler_t child_comp;
  if (!is_anonymous)
  {
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    hk_string_t *name = hk_string_from_chars(tk.length, tk.start);
    compiler_init(&child_comp, comp, scan, name);
    add_variable(&child_comp, true, 0, &tk, false);
  }
  else
    compiler_init(&child_comp, comp, scan, NULL);
  hk_chunk_t *child_chunk = &child_comp.fn->chunk;
  consume(comp, TOKEN_LPAREN);
  if (match(scan, TOKEN_RPAREN))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_ARROW))
    {
      scanner_next_token(scan);
      compile_expression(&child_comp);
      hk_chunk_emit_opcode(child_chunk, HK_OP_RETURN);
      goto end;
    }
    if (!match(scan, TOKEN_LBRACE))
      syntax_error_unexpected(comp);
    compile_block(&child_comp);
    hk_chunk_emit_opcode(child_chunk, HK_OP_RETURN_NIL);
    hk_function_add_line(fn, scan->token.line);
    goto end;
  }
  bool is_mutable = false;
  if (match(scan, TOKEN_MUT))
  {
    scanner_next_token(scan);
    is_mutable = true;
  }
  if (!match(scan, TOKEN_NAME))
    syntax_error_unexpected(comp);
  define_local(&child_comp, &scan->token, is_mutable);
  scanner_next_token(scan);
  int32_t arity = 1;
  while (match(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    bool is_mutable = false;
    if (match(scan, TOKEN_MUT))
    {
      scanner_next_token(scan);
      is_mutable = true;
    }
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    define_local(&child_comp, &scan->token, is_mutable);
    scanner_next_token(scan);
    ++arity;
  }
  child_comp.fn->arity = arity;
  consume(comp, TOKEN_RPAREN);
  if (match(scan, TOKEN_ARROW))
  {
    scanner_next_token(scan);
    compile_expression(&child_comp);
    hk_chunk_emit_opcode(child_chunk, HK_OP_RETURN);
    goto end;
  }
  if (!match(scan, TOKEN_LBRACE))
    syntax_error_unexpected(comp);
  compile_block(&child_comp);
  hk_chunk_emit_opcode(child_chunk, HK_OP_RETURN_NIL);
  hk_function_add_line(fn, scan->token.line);
  uint8_t index;
end:
  index = fn->num_functions;
  hk_function_add_child(fn, child_comp.fn);
  hk_chunk_emit_opcode(chunk, HK_OP_CLOSURE);
  hk_chunk_emit_byte(chunk, index);
  hk_function_add_line(fn, line);
}

static void compile_del_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (!match(scan, TOKEN_NAME))
    syntax_error_unexpected(comp);
  token_t tk = scan->token;
  scanner_next_token(scan);
  variable_t var = resolve_variable(comp, &tk);
  if (!var.is_mutable)
    syntax_error(fn->name->chars, scan->file->chars, tk.line, tk.col,
      "cannot delete element from immutable variable `%.*s`", tk.length, tk.start);
  hk_chunk_emit_opcode(chunk, HK_OP_GET_LOCAL);
  hk_chunk_emit_byte(chunk, var.index);
  hk_function_add_line(fn, tk.line);
  compile_delete(comp, true);
  hk_chunk_emit_opcode(chunk, HK_OP_SET_LOCAL);
  hk_chunk_emit_byte(chunk, var.index);
}

static void compile_delete(compiler_t *comp, bool inplace)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  if (match(scan, TOKEN_LBRACKET))
  {
    int32_t line = scan->token.line;
    scanner_next_token(scan);
    compile_expression(comp);
    consume(comp, TOKEN_RBRACKET);
    if (match(scan, TOKEN_SEMICOLON))
    {
      scanner_next_token(scan);
      hk_chunk_emit_opcode(chunk, inplace ? HK_OP_INPLACE_DELETE_ELEMENT : HK_OP_DELETE_ELEMENT);
      hk_function_add_line(fn, line);
      return;
    }
    hk_chunk_emit_opcode(chunk, HK_OP_FETCH_ELEMENT);
    hk_function_add_line(fn, line);
    compile_delete(comp, false);
    hk_chunk_emit_opcode(chunk, HK_OP_SET_ELEMENT);
    return;
  }
  if (match(scan, TOKEN_DOT))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_FETCH_FIELD);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    compile_delete(comp, false);
    hk_chunk_emit_opcode(chunk, HK_OP_SET_FIELD);
    return;
  }
  syntax_error_unexpected(comp); 
}

static void compile_if_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_LPAREN);
  compile_expression(comp);
  consume(comp, TOKEN_RPAREN);
  int32_t offset1 = emit_jump(chunk, HK_OP_JUMP_IF_FALSE);
  compile_statement(comp);
  int32_t offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  if (match(scan, TOKEN_ELSE))
  {
    scanner_next_token(scan);
    compile_statement(comp);
  }
  patch_jump(comp, offset2);
}

static void compile_match_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_LPAREN);
  compile_expression(comp);
  consume(comp, TOKEN_RPAREN);
  consume(comp, TOKEN_LBRACE);
  compile_expression(comp);
  consume(comp, TOKEN_ARROW);
  int32_t offset1 = emit_jump(chunk, HK_OP_MATCH);
  compile_statement(comp);
  int32_t offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  compile_match_statement_member(comp);
  patch_jump(comp, offset2);
}

static void compile_match_statement_member(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  if (match(scan, TOKEN_RBRACE))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_POP);
    return;
  }
  if (match(scan, TOKEN_UNDERSCORE))
  {
    scanner_next_token(scan);
    consume(comp, TOKEN_ARROW);
    hk_chunk_emit_opcode(chunk, HK_OP_POP);
    compile_statement(comp);
    consume(comp, TOKEN_RBRACE);
    return;
  }
  compile_expression(comp);
  consume(comp, TOKEN_ARROW);
  int32_t offset1 = emit_jump(chunk, HK_OP_MATCH);
  compile_statement(comp);
  int32_t offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  compile_match_statement_member(comp);
  patch_jump(comp, offset2);
}

static void compile_loop_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  if (!match(scan, TOKEN_LBRACE))
    syntax_error_unexpected(comp);
  loop_t loop;
  start_loop(comp, &loop);
  compile_statement(comp);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, loop.jump);
  end_loop(comp);
}

static void compile_while_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_LPAREN);
  loop_t loop;
  start_loop(comp, &loop);
  compile_expression(comp);
  consume(comp, TOKEN_RPAREN);
  int32_t offset = emit_jump(chunk, HK_OP_JUMP_IF_FALSE);
  compile_statement(comp);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, loop.jump);
  patch_jump(comp, offset);
  end_loop(comp);
}

static void compile_do_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  loop_t loop;
  start_loop(comp, &loop);
  compile_statement(comp);
  consume(comp, TOKEN_WHILE);
  consume(comp, TOKEN_LPAREN);
  compile_expression(comp);
  consume(comp, TOKEN_RPAREN);
  consume(comp, TOKEN_SEMICOLON);
  int32_t offset = emit_jump(chunk, HK_OP_JUMP_IF_FALSE);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, loop.jump);
  patch_jump(comp, offset);
  end_loop(comp);
}

static void compile_for_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_LPAREN);
  push_scope(comp);
  if (match(scan, TOKEN_SEMICOLON))
    scanner_next_token(scan);
  else
  {
    if (match(scan, TOKEN_VAL))
    {
      compile_constant_declaration(comp);
      consume(comp, TOKEN_SEMICOLON);
    }
    else if (match(scan, TOKEN_MUT))
    {
      compile_variable_declaration(comp);
      consume(comp, TOKEN_SEMICOLON);
    }
    else if (match(scan, TOKEN_NAME))
    {
      token_t tk = scan->token;
      scanner_next_token(scan);
      compile_assign_statement(comp, &tk);
      consume(comp, TOKEN_SEMICOLON);
    }
    else
      syntax_error_unexpected(comp);
  }
  uint16_t jump1 = (uint16_t) chunk->length;
  bool missing = match(scan, TOKEN_SEMICOLON);
  int32_t offset1;
  if (missing)
    scanner_next_token(scan);
  else
  {
    compile_expression(comp);
    consume(comp, TOKEN_SEMICOLON);
    offset1 = emit_jump(chunk, HK_OP_JUMP_IF_FALSE);
  }
  int32_t offset2 = emit_jump(chunk, HK_OP_JUMP);
  uint16_t jump2 = (uint16_t) chunk->length;
  loop_t loop;
  start_loop(comp, &loop);
  if (match(scan, TOKEN_RPAREN))
    scanner_next_token(scan);
  else
  {
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    compile_assign_statement(comp, &tk);
    consume(comp, TOKEN_RPAREN);
  }
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, jump1);
  patch_jump(comp, offset2);
  compile_statement(comp);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, jump2);
  if (!missing)
    patch_jump(comp, offset1);
  end_loop(comp);
  pop_scope(comp);
}

static void compile_continue_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  token_t tk = scan->token;
  scanner_next_token(scan);
  if (!comp->loop)
    syntax_error(fn->name->chars, scan->file->chars, tk.line, tk.col,
      "cannot use continue outside of a loop");
  consume(comp, TOKEN_SEMICOLON);
  discard_variables(comp, comp->loop->scope_depth + 1);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, comp->loop->jump);
}

static void compile_break_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  char *function = comp->fn->name->chars;
  char *file = scan->file->chars;
  token_t tk = scan->token;
  scanner_next_token(scan);
  if (!comp->loop)
    syntax_error(function, file, tk.line, tk.col,
      "cannot use break outside of a loop");
  consume(comp, TOKEN_SEMICOLON);
  discard_variables(comp, comp->loop->scope_depth + 1);
  loop_t *loop = comp->loop;
  if (loop->num_offsets == MAX_BREAKS)
    syntax_error(function, file, tk.line, tk.col,
      "cannot use more than %d breaks", MAX_BREAKS);
  int32_t offset = emit_jump(&comp->fn->chunk, HK_OP_JUMP);
  loop->offsets[loop->num_offsets++] = offset;
}

static void compile_return_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (match(scan, TOKEN_SEMICOLON))
  {
    int32_t line = scan->token.line;
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_RETURN_NIL);
    hk_function_add_line(fn, line);
    return;
  }
  compile_expression(comp);
  consume(comp, TOKEN_SEMICOLON);
  hk_chunk_emit_opcode(chunk, HK_OP_RETURN);
}

static void compile_block(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  scanner_next_token(scan);
  push_scope(comp);
  while (!match(scan, TOKEN_RBRACE))
    compile_statement(comp);
  scanner_next_token(scan);
  pop_scope(comp);
}

static void compile_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  compile_and_expression(comp);
  while (match(scan, TOKEN_PIPEPIPE))
  {
    scanner_next_token(scan);
    int32_t offset = emit_jump(&comp->fn->chunk, HK_OP_OR);
    compile_and_expression(comp);
    patch_jump(comp, offset);
  }
}

static void compile_and_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  compile_equal_expression(comp);
  while (match(scan, TOKEN_AMPAMP))
  {
    scanner_next_token(scan);
    int32_t offset = emit_jump(&comp->fn->chunk, HK_OP_AND);
    compile_equal_expression(comp);
    patch_jump(comp, offset);
  }
}

static void compile_equal_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  compile_comp_expression(comp);
  for (;;)
  {
    if (match(scan, TOKEN_EQEQ))
    {
      scanner_next_token(scan);
      compile_comp_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_EQUAL);
      continue;
    }
    if (match(scan, TOKEN_BANGEQ))
    {
      scanner_next_token(scan);
      compile_comp_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_NOT_EQUAL);
      continue;
    }
    break;
  }
}

static void compile_comp_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  compile_add_expression(comp);
  for (;;)
  {
    int32_t line = scan->token.line;
    if (match(scan, TOKEN_GT))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_GREATER);
      hk_function_add_line(fn, line);
      continue;
    }
    if (match(scan, TOKEN_GTEQ))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_NOT_LESS);
      hk_function_add_line(fn, line);
      continue;
    }
    if (match(scan, TOKEN_LT))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_LESS);
      hk_function_add_line(fn, line);
      continue;
    }
    if (match(scan, TOKEN_LTEQ))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_NOT_GREATER);
      hk_function_add_line(fn, line);
      continue;
    }
    break;
  }
}

static void compile_add_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  compile_range_expression(comp);
  for (;;)
  {
    int32_t line = scan->token.line;
    if (match(scan, TOKEN_PLUS))
    {
      scanner_next_token(scan);
      compile_range_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_ADD);
      hk_function_add_line(fn, line);
      continue;
    }
    if (match(scan, TOKEN_MINUS))
    {
      scanner_next_token(scan);
      compile_range_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_SUBTRACT);
      hk_function_add_line(fn, line);
      continue;
    }
    break;
  }
}

static void compile_range_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  compile_mul_expression(comp);
  int32_t line = scan->token.line;
  if (match(scan, TOKEN_DOTDOT))
  {
    scanner_next_token(scan);
    compile_mul_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_RANGE);
    hk_function_add_line(fn, line);
  }
}

static void compile_mul_expression(compiler_t *comp)
{
  compile_unary_expression(comp);
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  for (;;)
  {
    int32_t line = scan->token.line;
    if (match(scan, TOKEN_STAR))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_MULTIPLY);
      hk_function_add_line(fn, line);
      continue;
    }
    if (match(scan, TOKEN_SLASH))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_DIVIDE);
      hk_function_add_line(fn, line);
      continue;
    }
    if (match(scan, TOKEN_PERCENT))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_MODULO);
      hk_function_add_line(fn, line);
      continue;
    }
    break;
  }
}

static void compile_unary_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  if (match(scan, TOKEN_MINUS))
  {
    int32_t line = scan->token.line;
    scanner_next_token(scan);
    compile_unary_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_NEGATE);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_BANG))
  {
    scanner_next_token(scan);
    compile_unary_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_NOT);
    return;
  }
  compile_prim_expression(comp);
}

static void compile_prim_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  int32_t line = scan->token.line;
  if (match(scan, TOKEN_NIL))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_NIL);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_FALSE))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_FALSE);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_TRUE))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_TRUE);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_INT))
  {
    double data = parse_double(comp);
    scanner_next_token(scan);
    if (data <= UINT16_MAX)
    {
      hk_chunk_emit_opcode(chunk, HK_OP_INT);
      hk_chunk_emit_word(chunk, (uint16_t) data);
      hk_function_add_line(fn, line);
      return;
    }
    uint8_t index = add_float_constant(comp, data);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_FLOAT))
  {
    double data = parse_double(comp);
    scanner_next_token(scan);
    uint8_t index = add_float_constant(comp, data);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_STRING))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, line);
    return;
  }
  if (match(scan, TOKEN_LBRACKET))
  {
    compile_array_constructor(comp);
    return;
  }
  if (match(scan, TOKEN_LBRACE))
  {
    compile_struct_constructor(comp);
    return;
  }
  if (match(scan, TOKEN_STRUCT))
  {
    compile_struct_declaration(comp, true);
    return;
  }
  if (match(scan, TOKEN_FN))
  {
    compile_function_declaration(comp, true);
    return;
  }
  if (match(scan, TOKEN_IF))
  {
    compile_if_expression(comp);
    return;
  }
  if (match(scan, TOKEN_MATCH))
  {
    compile_match_expression(comp);
    return;
  }
  if (match(scan, TOKEN_NAME))
  {
    compile_subscript(comp);
    return;
  }
  if (match(scan, TOKEN_LPAREN))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    consume(comp, TOKEN_RPAREN);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_array_constructor(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  int32_t line = scan->token.line;
  scanner_next_token(scan);
  uint8_t length = 0;
  if (match(scan, TOKEN_RBRACKET))
  {
    scanner_next_token(scan);
    goto end;
  }
  compile_expression(comp);
  ++length;
  while (match(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    ++length;
  }
  consume(comp, TOKEN_RBRACKET);
end:
  hk_chunk_emit_opcode(chunk, HK_OP_ARRAY);
  hk_chunk_emit_byte(chunk, length);
  hk_function_add_line(fn, line);
  return;
}

static void compile_struct_constructor(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  int32_t line = scan->token.line;
  scanner_next_token(scan);
  hk_chunk_emit_opcode(chunk, HK_OP_NIL);
  hk_function_add_line(fn, line);
  if (match(scan, TOKEN_RBRACE))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTRUCT);
    hk_chunk_emit_byte(chunk, 0);
    hk_function_add_line(fn, line);
    return;
  }
  if (!match(scan, TOKEN_NAME))
    syntax_error_unexpected(comp);
  token_t tk = scan->token;
  scanner_next_token(scan);
  uint8_t index = add_string_constant(comp, &tk);
  hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
  hk_chunk_emit_byte(chunk, index);
  hk_function_add_line(fn, tk.line);
  consume(comp, TOKEN_COLON);
  compile_expression(comp);
  uint8_t length = 1;
  while (match(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk.line);
    consume(comp, TOKEN_COLON);
    compile_expression(comp);
    ++length;
  }
  consume(comp, TOKEN_RBRACE);
  hk_chunk_emit_opcode(chunk, HK_OP_CONSTRUCT);
  hk_chunk_emit_byte(chunk, length);
  hk_function_add_line(fn, line);
}

static void compile_if_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_LPAREN);
  compile_expression(comp);
  consume(comp, TOKEN_RPAREN);
  int32_t offset1 = emit_jump(chunk, HK_OP_JUMP_IF_FALSE);
  compile_expression(comp);
  int32_t offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  consume(comp, TOKEN_ELSE);
  compile_expression(comp);
  patch_jump(comp, offset2);
}

static void compile_match_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_LPAREN);
  compile_expression(comp);
  consume(comp, TOKEN_RPAREN);
  consume(comp, TOKEN_LBRACE);
  compile_expression(comp);
  consume(comp, TOKEN_ARROW);
  int32_t offset1 = emit_jump(chunk, HK_OP_MATCH);
  compile_expression(comp);
  int32_t offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  if (match(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_UNDERSCORE))
    {
      scanner_next_token(scan);
      consume(comp, TOKEN_ARROW);
      hk_chunk_emit_opcode(chunk, HK_OP_POP);
      compile_expression(comp);
      consume(comp, TOKEN_RBRACE);
      patch_jump(comp, offset2);
      return;
    }
    compile_match_expression_member(comp);
    patch_jump(comp, offset2);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_match_expression_member(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_chunk_t *chunk = &comp->fn->chunk;
  compile_expression(comp);
  consume(comp, TOKEN_ARROW);
  int32_t offset1 = emit_jump(chunk, HK_OP_MATCH);
  compile_expression(comp);
  int32_t offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  if (match(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_UNDERSCORE))
    {
      scanner_next_token(scan);
      consume(comp, TOKEN_ARROW);
      hk_chunk_emit_opcode(chunk, HK_OP_POP);
      compile_expression(comp);
      consume(comp, TOKEN_RBRACE);
      patch_jump(comp, offset2);
      return;
    }
    compile_match_expression_member(comp);
    patch_jump(comp, offset2);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_subscript(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  compile_variable(comp, &scan->token, true);
  scanner_next_token(scan);
  for (;;)
  {
    int32_t line = scan->token.line;
    if (match(scan, TOKEN_LBRACKET))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      consume(comp, TOKEN_RBRACKET);
      hk_chunk_emit_opcode(chunk, HK_OP_GET_ELEMENT);
      hk_function_add_line(fn, line);
      continue;
    }
    if (match(scan, TOKEN_DOT))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      token_t tk = scan->token;
      scanner_next_token(scan);
      uint8_t index = add_string_constant(comp, &tk);
      hk_chunk_emit_opcode(chunk, HK_OP_GET_FIELD);
      hk_chunk_emit_byte(chunk, index);
      hk_function_add_line(fn, line);
      continue;
    }
    if (match(scan, TOKEN_LPAREN))
    {
      scanner_next_token(scan);
      if (match(scan, TOKEN_RPAREN))
      {
        scanner_next_token(scan);
        hk_chunk_emit_opcode(chunk, HK_OP_CALL);
        hk_chunk_emit_byte(chunk, 0);
        hk_function_add_line(fn, line);
        return;
      }
      compile_expression(comp);
      uint8_t num_args = 1;
      while (match(scan, TOKEN_COMMA))
      {
        scanner_next_token(scan);
        compile_expression(comp);
        ++num_args;
      }
      consume(comp, TOKEN_RPAREN);
      hk_chunk_emit_opcode(chunk, HK_OP_CALL);
      hk_chunk_emit_byte(chunk, num_args);
      hk_function_add_line(fn, line);
      continue;
    }
    break;
  }
  if (match(scan, TOKEN_LBRACE))
  {
    int32_t line = scan->token.line;
    scanner_next_token(scan);
    if (match(scan, TOKEN_RBRACE))
    {
      scanner_next_token(scan);
      hk_chunk_emit_opcode(chunk, HK_OP_INSTANCE);
      hk_chunk_emit_byte(chunk, 0);
      hk_function_add_line(fn, line);
      return;
    }
    compile_expression(comp);
    uint8_t num_args = 1;
    while (match(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      ++num_args;
    }
    consume(comp, TOKEN_RBRACE);
    hk_chunk_emit_opcode(chunk, HK_OP_INSTANCE);
    hk_chunk_emit_byte(chunk, num_args);
    hk_function_add_line(fn, line);
  }
}

static variable_t compile_variable(compiler_t *comp, token_t *tk, bool emit)
{
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  variable_t *var = lookup_variable(comp, tk);
  if (var)
  {
    if (!emit)
      return *var;
    hk_chunk_emit_opcode(chunk, var->is_local ? HK_OP_GET_LOCAL : HK_OP_NONLOCAL);
    hk_chunk_emit_byte(chunk, var->index);
    hk_function_add_line(fn, tk->line);
    return *var;
  }
  var = compile_nonlocal(comp->parent, tk);
  if (var)
  {
    uint8_t index = add_nonlocal(comp, tk);
    hk_chunk_emit_opcode(chunk, HK_OP_NONLOCAL);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk->line);
    return *var;
  }
  int32_t index = lookup_global(tk->length, tk->start);
  if (index == -1)
    syntax_error(fn->name->chars, comp->scan->file->chars, tk->line, tk->col,
      "variable `%.*s` is used but not defined", tk->length, tk->start);
  hk_chunk_emit_opcode(chunk, HK_OP_GLOBAL);
  hk_chunk_emit_byte(chunk, (uint8_t) index);
  hk_function_add_line(fn, tk->line);
  return (variable_t) {.is_local = false, .depth = -1, .index = index, .length = tk->length,
    .start = tk->start, .is_mutable = false};
}

static variable_t *compile_nonlocal(compiler_t *comp, token_t *tk)
{
  if (!comp)
    return NULL;
  hk_function_t *fn = comp->fn;
  hk_chunk_t *chunk = &fn->chunk;
  variable_t *var = lookup_variable(comp, tk);
  if (var)
  {
    int32_t op = HK_OP_NONLOCAL;
    if (var->is_local)
    {
      if (var->is_mutable)
        syntax_error(fn->name->chars, comp->scan->file->chars, tk->line, tk->col,
          "cannot capture mutable variable `%.*s`", tk->length, tk->start);
      op = HK_OP_GET_LOCAL;
    }
    hk_chunk_emit_opcode(chunk, op);
    hk_chunk_emit_byte(chunk, var->index);
    hk_function_add_line(fn, tk->line);
    return var;
  }
  var = compile_nonlocal(comp->parent, tk);
  if (var)
  {
    uint8_t index = add_nonlocal(comp, tk);
    hk_chunk_emit_opcode(chunk, HK_OP_NONLOCAL);
    hk_chunk_emit_byte(chunk, index);
    hk_function_add_line(fn, tk->line);
    return var;
  }
  return NULL;
}

hk_closure_t *hk_compile(hk_string_t *file, hk_string_t *source)
{
  scanner_t scan;
  scanner_init(&scan, file, source);
  compiler_t comp;
  compiler_init(&comp, NULL, &scan, hk_string_from_chars(-1, "main"));
  char args_name[] = "args";
  token_t tk = {.length = sizeof(args_name) - 1, .start = args_name};
  add_local(&comp, &tk, false);
  while (!match(comp.scan, TOKEN_EOF))
    compile_statement(&comp);
  hk_function_t *fn = comp.fn;
  hk_chunk_t *chunk = &fn->chunk;
  hk_chunk_emit_opcode(chunk, HK_OP_RETURN_NIL);
  hk_function_add_line(fn, scan.token.line);
  hk_closure_t *cl = hk_closure_new(fn);
  scanner_free(&scan);
  return cl;
}
