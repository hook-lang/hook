//
// Hook Programming Language
// compiler.c
//

#include "compiler.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include "scanner.h"
#include "struct.h"
#include "builtin.h"
#include "error.h"

#define MAX_CONSTANTS UINT8_MAX
#define MAX_VARIABLES UINT8_MAX
#define MAX_BREAKS    UINT8_MAX

#define MATCH(s, t) ((s)->token.type == (t))

#define EXPECT(c, t) do \
  { \
    scanner_t *scan = (c)->scan; \
    if (!MATCH(scan, t)) \
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
  int depth;
  uint8_t index;
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
  int offsets[MAX_BREAKS];
} loop_t;

typedef struct compiler
{
  struct compiler *parent;
  scanner_t *scan;
  int scope_depth;
  int num_variables;
  uint8_t local_index;
  variable_t variables[MAX_VARIABLES];
  loop_t *loop;
  function_t *fn;
} compiler_t;

static inline void syntax_error(const char *function, const char *file, int line,
  int col, const char *fmt, ...);
static inline void syntax_error_unexpected(compiler_t *comp);
static inline double parse_double(compiler_t *comp);
static inline bool string_match(token_t *tk, string_t *str);
static inline uint8_t add_number_constant(compiler_t *comp, double data);
static inline uint8_t add_string_constant(compiler_t *comp, token_t *tk);
static inline uint8_t add_constant(compiler_t *comp, value_t val);
static inline void push_scope(compiler_t *comp);
static inline void pop_scope(compiler_t *comp);
static inline int discard_variables(compiler_t *comp, int depth);
static inline bool variable_match(token_t *tk, variable_t *var);
static inline void add_local(compiler_t *comp, token_t *tk, bool is_mutable);
static inline uint8_t add_nonlocal(compiler_t *comp, token_t *tk);
static inline void add_variable(compiler_t *comp, bool is_local, uint8_t index, token_t *tk,
  bool is_mutable);
static inline void define_local(compiler_t *comp, token_t *tk, bool is_mutable);
static inline variable_t resolve_variable(compiler_t *comp, token_t *tk);
static inline variable_t *lookup_variable(compiler_t *comp, token_t *tk);
static inline bool nonlocal_exists(compiler_t *comp, token_t *tk);
static inline int emit_jump(chunk_t *chunk, opcode_t op);
static inline void patch_jump(compiler_t *comp, int offset);
static inline void patch_opcode(chunk_t *chunk, int offset, opcode_t op);
static inline void start_loop(compiler_t *comp, loop_t *loop);
static inline void end_loop(compiler_t *comp);
static inline void compiler_init(compiler_t *comp, compiler_t *parent, scanner_t *scan,
  string_t *name);
static void compile_statement(compiler_t *comp);
static void compile_load_module(compiler_t *comp);
static void compile_constant_declaration(compiler_t *comp);
static void compile_variable_declaration(compiler_t *comp);
static void compile_assign_statement(compiler_t *comp, token_t *tk);
static int compile_assign(compiler_t *comp, int syntax, bool inplace);
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

static inline void syntax_error(const char *function, const char *file, int line,
  int col, const char *fmt, ...)
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
      "number `%.*s` is out of range", tk->length, tk->start);
  return result;
}

static inline bool string_match(token_t *tk, string_t *str)
{
  return tk->length == str->length
    && !memcmp(tk->start, str->chars, tk->length);
}

static inline uint8_t add_number_constant(compiler_t *comp, double data)
{
  array_t *consts = comp->fn->consts;
  value_t *elements = consts->elements;
  for (int i = 0; i < consts->length; ++i)
  {
    value_t elem = elements[i];
    if (elem.type != TYPE_NUMBER)
      continue;
    if (data == elem.as.number)
      return (uint8_t) i;
  }
  return add_constant(comp, NUMBER_VALUE(data));
}

static inline uint8_t add_string_constant(compiler_t *comp, token_t *tk)
{
  array_t *consts = comp->fn->consts;
  value_t *elements = consts->elements;
  for (int i = 0; i < consts->length; ++i)
  {
    value_t elem = elements[i];
    if (elem.type != TYPE_STRING)
      continue;
    if (string_match(tk, AS_STRING(elem)))
      return (uint8_t) i;
  }
  string_t *str = string_from_chars(tk->length, tk->start);
  return add_constant(comp, STRING_VALUE(str));
}

static inline uint8_t add_constant(compiler_t *comp, value_t val)
{
  function_t *fn = comp->fn;
  array_t *consts = fn->consts;
  scanner_t *scan = comp->scan;
  token_t *tk = &scan->token;
  if (consts->length == MAX_CONSTANTS)
    syntax_error(fn->name->chars, scan->file->chars, tk->line, tk->col,
      "a function may only contain %d unique constants", MAX_CONSTANTS);
  uint8_t index = (uint8_t) consts->length;
  array_inplace_add_element(consts, val);
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

static inline int discard_variables(compiler_t *comp, int depth)
{
  variable_t *variables = comp->variables;
  chunk_t *chunk = &comp->fn->chunk;
  int index = comp->num_variables - 1;
  for (; index > -1 && variables[index].depth >= depth; --index)
    if (variables[index].is_local)
      chunk_emit_opcode(chunk, OP_POP);
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
  for (int i = comp->num_variables - 1; i > -1; --i)
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
  for (int i = comp->num_variables - 1; i > -1; --i)
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

static inline int emit_jump(chunk_t *chunk, opcode_t op)
{
  chunk_emit_opcode(chunk, op);
  int offset = chunk->length;
  chunk_emit_word(chunk, 0);
  return offset;
}

static inline void patch_jump(compiler_t *comp, int offset)
{
  chunk_t *chunk = &comp->fn->chunk;
  scanner_t *scan = comp->scan;
  token_t *tk = &scan->token;
  int jump = chunk->length;
  if (jump > UINT16_MAX)
    syntax_error(comp->fn->name->chars, scan->file->chars, tk->line, tk->col,
      "code too large");
  *((uint16_t *) &chunk->bytes[offset]) = (uint16_t) jump;
}

static inline void patch_opcode(chunk_t *chunk, int offset, opcode_t op)
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
  for (int i = 0; i < loop->num_offsets; ++i)
    patch_jump(comp, loop->offsets[i]);
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
  comp->fn = function_new(0, name, scan->file);
}

static void compile_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  if (MATCH(scan, TOKEN_USE))
  {
    compile_load_module(comp);
    return;
  }
  if (MATCH(scan, TOKEN_VAL))
  {
    compile_constant_declaration(comp);
    EXPECT(comp, TOKEN_SEMICOLON);
    return;
  }
  if (MATCH(scan, TOKEN_MUT))
  {
    compile_variable_declaration(comp);
    EXPECT(comp, TOKEN_SEMICOLON);
    return;
  }
  if (MATCH(scan, TOKEN_NAME))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    compile_assign_statement(comp, &tk);
    EXPECT(comp, TOKEN_SEMICOLON);
    return;
  }
  if (MATCH(scan, TOKEN_STRUCT))
  {
    compile_struct_declaration(comp, false);
    return;
  }
  if (MATCH(scan, TOKEN_FN))
  {
    compile_function_declaration(comp, false);
    return;
  }
  if (MATCH(scan, TOKEN_DEL))
  {
    compile_del_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_IF))
  {
    compile_if_statement(comp);
    return;
  }
  if (MATCH(scan, TOKEN_MATCH))
  {
    compile_match_statement(comp);
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
  if (MATCH(scan, TOKEN_LBRACE))
  {
    compile_block(comp);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_load_module(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (MATCH(scan, TOKEN_NAME))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    if (MATCH(scan, TOKEN_AS))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      tk = scan->token;
      scanner_next_token(scan);
    }
    define_local(comp, &tk, false);
    EXPECT(comp, TOKEN_SEMICOLON);
    chunk_emit_opcode(chunk, OP_LOAD_MODULE);
    function_add_line(fn, tk.line);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACE))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    uint8_t index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    uint8_t n = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      token_t tk = scan->token;
      scanner_next_token(scan);
      define_local(comp, &tk, false);
      uint8_t index = add_string_constant(comp, &tk);
      chunk_emit_opcode(chunk, OP_CONSTANT);
      chunk_emit_byte(chunk, index);
      function_add_line(fn, tk.line);
      ++n;
    }
    EXPECT(comp, TOKEN_RBRACE);
    EXPECT(comp, TOKEN_IN);
    int line = scan->token.line;
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    EXPECT(comp, TOKEN_SEMICOLON);
    index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    chunk_emit_opcode(chunk, OP_LOAD_MODULE);
    function_add_line(fn, tk.line);
    chunk_emit_opcode(chunk, OP_DESTRUCT);
    chunk_emit_byte(chunk, n);
    function_add_line(fn, line);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_constant_declaration(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (MATCH(scan, TOKEN_NAME))
  {
    define_local(comp, &scan->token, false);
    scanner_next_token(scan);
    EXPECT(comp, TOKEN_EQ);
    compile_expression(comp);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    define_local(comp, &scan->token, false);
    scanner_next_token(scan);
    uint8_t n = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      define_local(comp, &scan->token, false);
      scanner_next_token(scan);
      ++n;
    }
    EXPECT(comp, TOKEN_RBRACKET);
    EXPECT(comp, TOKEN_EQ);
    int line = scan->token.line;
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_UNPACK);
    chunk_emit_byte(chunk, n);
    function_add_line(fn, line);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACE))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    uint8_t index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    uint8_t n = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      token_t tk = scan->token;
      scanner_next_token(scan);
      define_local(comp, &tk, false);
      uint8_t index = add_string_constant(comp, &tk);
      chunk_emit_opcode(chunk, OP_CONSTANT);
      chunk_emit_byte(chunk, index);
      function_add_line(fn, tk.line);
      ++n;
    }
    EXPECT(comp, TOKEN_RBRACE);
    EXPECT(comp, TOKEN_EQ);
    int line = scan->token.line;
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_DESTRUCT);
    chunk_emit_byte(chunk, n);
    function_add_line(fn, line);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_variable_declaration(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
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
    chunk_emit_opcode(chunk, OP_NIL);
    function_add_line(fn, scan->token.line);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    define_local(comp, &scan->token, true);
    scanner_next_token(scan);
    uint8_t n = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      define_local(comp, &scan->token, true);
      scanner_next_token(scan);
      ++n;
    }
    EXPECT(comp, TOKEN_RBRACKET);
    EXPECT(comp, TOKEN_EQ);
    int line = scan->token.line;
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_UNPACK);
    chunk_emit_byte(chunk, n);
    function_add_line(fn, line);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACE))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, true);
    uint8_t index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    uint8_t n = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      token_t tk = scan->token;
      scanner_next_token(scan);
      define_local(comp, &tk, true);
      uint8_t index = add_string_constant(comp, &tk);
      chunk_emit_opcode(chunk, OP_CONSTANT);
      chunk_emit_byte(chunk, index);
      function_add_line(fn, tk.line);
      ++n;
    }
    EXPECT(comp, TOKEN_RBRACE);
    EXPECT(comp, TOKEN_EQ);
    int line = scan->token.line;
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_DESTRUCT);
    chunk_emit_byte(chunk, n);
    function_add_line(fn, line);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_assign_statement(compiler_t *comp, token_t *tk)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  variable_t var;
  if (MATCH(scan, TOKEN_EQ))
  {
    var = compile_variable(comp, tk, false);
    scanner_next_token(scan);
    compile_expression(comp);
    goto end;
  }
  var = compile_variable(comp, tk, true);
  if (compile_assign(comp, SYN_NONE, true) == SYN_CALL)
  {
    chunk_emit_opcode(chunk, OP_POP);
    return;
  }
end:
  if (!var.is_mutable)
    syntax_error(fn->name->chars, scan->file->chars, tk->line, tk->col,
      "cannot assign to immutable variable `%.*s`", tk->length, tk->start);
  chunk_emit_opcode(chunk, OP_SET_LOCAL);
  chunk_emit_byte(chunk, var.index);
}

static int compile_assign(compiler_t *comp, int syntax, bool inplace)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  int line = scan->token.line;
  if (MATCH(scan, TOKEN_PLUSEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_ADD);
    function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_MINUSEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_SUBTRACT);
    function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_STAREQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_MULTIPLY);
    function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_SLASHEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_DIVIDE);
    function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_PERCENTEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_MODULO);
    function_add_line(fn, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_PLUSPLUS))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_INCR);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_MINUSMINUS))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_DECR);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_RBRACKET))
    {
      scanner_next_token(scan);
      EXPECT(comp, TOKEN_EQ);
      compile_expression(comp);
      chunk_emit_opcode(chunk, inplace ? OP_INPLACE_ADD_ELEMENT : OP_ADD_ELEMENT);
      function_add_line(fn, line);
      return SYN_ASSIGN;
    }
    compile_expression(comp);
    EXPECT(comp, TOKEN_RBRACKET);
    if (MATCH(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, inplace ? OP_INPLACE_PUT_ELEMENT : OP_PUT_ELEMENT);
      function_add_line(fn, line);
      return SYN_ASSIGN;
    }
    int offset = chunk->length;
    chunk_emit_opcode(chunk, OP_GET_ELEMENT);
    function_add_line(fn, line);
    int syn = compile_assign(comp, SYN_SUBSCRIPT, false);
    if (syn == SYN_ASSIGN)
    {
      patch_opcode(chunk, offset, OP_FETCH_ELEMENT);
      chunk_emit_opcode(chunk, OP_SET_ELEMENT);
    }
    return syn;
  }
  if (MATCH(scan, TOKEN_DOT))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    if (MATCH(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, inplace ? OP_INPLACE_PUT_FIELD : OP_PUT_FIELD);
      chunk_emit_byte(chunk, index);
      function_add_line(fn, tk.line);
      return SYN_ASSIGN;
    }
    int offset = chunk->length;
    chunk_emit_opcode(chunk, OP_GET_FIELD);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    int syn = compile_assign(comp, SYN_SUBSCRIPT, false);
    if (syn == SYN_ASSIGN)
    {
      patch_opcode(chunk, offset, OP_FETCH_FIELD);
      chunk_emit_opcode(chunk, OP_SET_FIELD);
    }
    return syn;
  }
  if (MATCH(scan, TOKEN_LPAREN))
  {
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_RPAREN))
    {
      scanner_next_token(scan);
      chunk_emit_opcode(chunk, OP_CALL);
      chunk_emit_byte(chunk, 0);
      function_add_line(fn, line);
      return compile_assign(comp, SYN_CALL, false);
    }
    compile_expression(comp);
    uint8_t num_args = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      ++num_args;
    }
    EXPECT(comp, TOKEN_RPAREN);
    chunk_emit_opcode(chunk, OP_CALL);
    chunk_emit_byte(chunk, num_args);
    function_add_line(fn, line);
    return compile_assign(comp, SYN_CALL, false);
  }
  if (syntax == SYN_NONE || syntax == SYN_SUBSCRIPT)
    syntax_error_unexpected(comp);
  return syntax;
}

static void compile_struct_declaration(compiler_t *comp, bool is_anonymous)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  int line = scan->token.line;
  scanner_next_token(scan);
  token_t tk;
  uint8_t index;
  if (is_anonymous)
  {
    chunk_emit_opcode(chunk, OP_NIL);
    function_add_line(fn, line);
  }
  else
  {
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
  }
  EXPECT(comp, TOKEN_LBRACE);
  if (MATCH(scan, TOKEN_RBRACE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_STRUCT);
    chunk_emit_byte(chunk, 0);
    function_add_line(fn, line);
    return;
  }
  if (!MATCH(scan, TOKEN_NAME))
    syntax_error_unexpected(comp);
  tk = scan->token;
  scanner_next_token(scan);
  index = add_string_constant(comp, &tk);
  chunk_emit_opcode(chunk, OP_CONSTANT);
  chunk_emit_byte(chunk, index);
  function_add_line(fn, tk.line);
  uint8_t length = 1;
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    ++length;
  }
  EXPECT(comp, TOKEN_RBRACE);
  chunk_emit_opcode(chunk, OP_STRUCT);
  chunk_emit_byte(chunk, length);
  function_add_line(fn, line);
}

static void compile_function_declaration(compiler_t *comp, bool is_anonymous)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  int line = scan->token.line;
  scanner_next_token(scan);
  compiler_t child_comp;
  if (!is_anonymous)
  {
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    string_t *name = string_from_chars(tk.length, tk.start);
    compiler_init(&child_comp, comp, scan, name);
    add_variable(&child_comp, true, 0, &tk, false);
  }
  else
    compiler_init(&child_comp, comp, scan, NULL);
  chunk_t *child_chunk = &child_comp.fn->chunk;
  EXPECT(comp, TOKEN_LPAREN);
  if (MATCH(scan, TOKEN_RPAREN))
  {
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_ARROW))
    {
      scanner_next_token(scan);
      compile_expression(&child_comp);
      chunk_emit_opcode(child_chunk, OP_RETURN);
      goto end;
    }
    if (!MATCH(scan, TOKEN_LBRACE))
      syntax_error_unexpected(comp);
    compile_block(&child_comp);
    chunk_emit_opcode(child_chunk, OP_RETURN_NIL);
    function_add_line(fn, scan->token.line);
    goto end;
  }
  bool is_mutable = false;
  if (MATCH(scan, TOKEN_MUT))
  {
    scanner_next_token(scan);
    is_mutable = true;
  }
  if (!MATCH(scan, TOKEN_NAME))
    syntax_error_unexpected(comp);
  define_local(&child_comp, &scan->token, is_mutable);
  scanner_next_token(scan);
  int arity = 1;
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    bool is_mutable = false;
    if (MATCH(scan, TOKEN_MUT))
    {
      scanner_next_token(scan);
      is_mutable = true;
    }
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    define_local(&child_comp, &scan->token, is_mutable);
    scanner_next_token(scan);
    ++arity;
  }
  child_comp.fn->arity = arity;
  EXPECT(comp, TOKEN_RPAREN);
  if (MATCH(scan, TOKEN_ARROW))
  {
    scanner_next_token(scan);
    compile_expression(&child_comp);
    chunk_emit_opcode(child_chunk, OP_RETURN);
    goto end;
  }
  if (!MATCH(scan, TOKEN_LBRACE))
    syntax_error_unexpected(comp);
  compile_block(&child_comp);
  chunk_emit_opcode(child_chunk, OP_RETURN_NIL);
  function_add_line(fn, scan->token.line);
  uint8_t index;
end:
  index = fn->num_functions;
  function_add_child(fn, child_comp.fn);
  chunk_emit_opcode(chunk, OP_CLOSURE);
  chunk_emit_byte(chunk, index);
  function_add_line(fn, line);
}

static void compile_del_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (!MATCH(scan, TOKEN_NAME))
    syntax_error_unexpected(comp);
  token_t tk = scan->token;
  scanner_next_token(scan);
  variable_t var = resolve_variable(comp, &tk);
  if (!var.is_mutable)
    syntax_error(fn->name->chars, scan->file->chars, tk.line, tk.col,
      "cannot delete element from immutable variable `%.*s`", tk.length, tk.start);
  chunk_emit_opcode(chunk, OP_GET_LOCAL);
  chunk_emit_byte(chunk, var.index);
  function_add_line(fn, tk.line);
  compile_delete(comp, true);
  chunk_emit_opcode(chunk, OP_SET_LOCAL);
  chunk_emit_byte(chunk, var.index);
}

static void compile_delete(compiler_t *comp, bool inplace)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    int line = scan->token.line;
    scanner_next_token(scan);
    compile_expression(comp);
    EXPECT(comp, TOKEN_RBRACKET);
    if (MATCH(scan, TOKEN_SEMICOLON))
    {
      scanner_next_token(scan);
      chunk_emit_opcode(chunk, inplace ? OP_INPLACE_DELETE_ELEMENT : OP_DELETE_ELEMENT);
      function_add_line(fn, line);
      return;
    }
    chunk_emit_opcode(chunk, OP_FETCH_ELEMENT);
    function_add_line(fn, line);
    compile_delete(comp, false);
    chunk_emit_opcode(chunk, OP_SET_ELEMENT);
    return;
  }
  if (MATCH(scan, TOKEN_DOT))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_FETCH_FIELD);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    compile_delete(comp, false);
    chunk_emit_opcode(chunk, OP_SET_FIELD);
    return;
  }
  syntax_error_unexpected(comp); 
}

static void compile_if_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  EXPECT(comp, TOKEN_LPAREN);
  compile_expression(comp);
  EXPECT(comp, TOKEN_RPAREN);
  int offset1 = emit_jump(chunk, OP_JUMP_IF_FALSE);
  compile_statement(comp);
  int offset2 = emit_jump(chunk, OP_JUMP);
  patch_jump(comp, offset1);
  if (MATCH(scan, TOKEN_ELSE))
  {
    scanner_next_token(scan);
    compile_statement(comp);
  }
  patch_jump(comp, offset2);
}

static void compile_match_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  EXPECT(comp, TOKEN_LPAREN);
  compile_expression(comp);
  EXPECT(comp, TOKEN_RPAREN);
  EXPECT(comp, TOKEN_LBRACE);
  compile_expression(comp);
  EXPECT(comp, TOKEN_ARROW);
  int offset1 = emit_jump(chunk, OP_MATCH);
  compile_statement(comp);
  int offset2 = emit_jump(chunk, OP_JUMP);
  patch_jump(comp, offset1);
  compile_match_statement_member(comp);
  patch_jump(comp, offset2);
}

static void compile_match_statement_member(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
  if (MATCH(scan, TOKEN_RBRACE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_POP);
    return;
  }
  if (MATCH(scan, TOKEN_UNDERSCORE))
  {
    scanner_next_token(scan);
    EXPECT(comp, TOKEN_ARROW);
    chunk_emit_opcode(chunk, OP_POP);
    compile_statement(comp);
    EXPECT(comp, TOKEN_RBRACE);
    return;
  }
  compile_expression(comp);
  EXPECT(comp, TOKEN_ARROW);
  int offset1 = emit_jump(chunk, OP_MATCH);
  compile_statement(comp);
  int offset2 = emit_jump(chunk, OP_JUMP);
  patch_jump(comp, offset1);
  compile_match_statement_member(comp);
  patch_jump(comp, offset2);
}

static void compile_loop_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  if (!MATCH(scan, TOKEN_LBRACE))
    syntax_error_unexpected(comp);
  loop_t loop;
  start_loop(comp, &loop);
  compile_statement(comp);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, loop.jump);
  end_loop(comp);
}

static void compile_while_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  EXPECT(comp, TOKEN_LPAREN);
  loop_t loop;
  start_loop(comp, &loop);
  compile_expression(comp);
  EXPECT(comp, TOKEN_RPAREN);
  int offset = emit_jump(chunk, OP_JUMP_IF_FALSE);
  compile_statement(comp);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, loop.jump);
  patch_jump(comp, offset);
  end_loop(comp);
}

static void compile_do_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  loop_t loop;
  start_loop(comp, &loop);
  compile_statement(comp);
  EXPECT(comp, TOKEN_WHILE);
  EXPECT(comp, TOKEN_LPAREN);
  compile_expression(comp);
  EXPECT(comp, TOKEN_RPAREN);
  EXPECT(comp, TOKEN_SEMICOLON);
  int offset = emit_jump(chunk, OP_JUMP_IF_FALSE);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, loop.jump);
  patch_jump(comp, offset);
  end_loop(comp);
}

static void compile_for_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  EXPECT(comp, TOKEN_LPAREN);
  push_scope(comp);
  if (MATCH(scan, TOKEN_SEMICOLON))
    scanner_next_token(scan);
  else
  {
    if (MATCH(scan, TOKEN_VAL))
    {
      compile_constant_declaration(comp);
      EXPECT(comp, TOKEN_SEMICOLON);
    }
    else if (MATCH(scan, TOKEN_MUT))
    {
      compile_variable_declaration(comp);
      EXPECT(comp, TOKEN_SEMICOLON);
    }
    else if (MATCH(scan, TOKEN_NAME))
    {
      token_t tk = scan->token;
      scanner_next_token(scan);
      compile_assign_statement(comp, &tk);
      EXPECT(comp, TOKEN_SEMICOLON);
    }
    else
      syntax_error_unexpected(comp);
  }
  uint16_t jump1 = (uint16_t) chunk->length;
  bool missing = MATCH(scan, TOKEN_SEMICOLON);
  int offset1;
  if (missing)
    scanner_next_token(scan);
  else
  {
    compile_expression(comp);
    EXPECT(comp, TOKEN_SEMICOLON);
    offset1 = emit_jump(chunk, OP_JUMP_IF_FALSE);
  }
  int offset2 = emit_jump(chunk, OP_JUMP);
  uint16_t jump2 = (uint16_t) chunk->length;
  loop_t loop;
  start_loop(comp, &loop);
  if (MATCH(scan, TOKEN_RPAREN))
    scanner_next_token(scan);
  else
  {
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    token_t tk = scan->token;
    scanner_next_token(scan);
    compile_assign_statement(comp, &tk);
    EXPECT(comp, TOKEN_RPAREN);
  }
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, jump1);
  patch_jump(comp, offset2);
  compile_statement(comp);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, jump2);
  if (!missing)
    patch_jump(comp, offset1);
  end_loop(comp);
  pop_scope(comp);
}

static void compile_continue_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  token_t tk = scan->token;
  scanner_next_token(scan);
  if (!comp->loop)
    syntax_error(fn->name->chars, scan->file->chars, tk.line, tk.col,
      "cannot use `continue` outside of a loop");
  EXPECT(comp, TOKEN_SEMICOLON);
  discard_variables(comp, comp->loop->scope_depth + 1);
  chunk_emit_opcode(chunk, OP_JUMP);
  chunk_emit_word(chunk, comp->loop->jump);
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
      "cannot use `break` outside of a loop");
  EXPECT(comp, TOKEN_SEMICOLON);
  discard_variables(comp, comp->loop->scope_depth + 1);
  loop_t *loop = comp->loop;
  if (loop->num_offsets == MAX_BREAKS)
    syntax_error(function, file, tk.line, tk.col,
      "cannot use more than %d breaks", MAX_BREAKS);
  int offset = emit_jump(&comp->fn->chunk, OP_JUMP);
  loop->offsets[loop->num_offsets++] = offset;
}

static void compile_return_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (MATCH(scan, TOKEN_SEMICOLON))
  {
    int line = scan->token.line;
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_RETURN_NIL);
    function_add_line(fn, line);
    return;
  }
  compile_expression(comp);
  EXPECT(comp, TOKEN_SEMICOLON);
  chunk_emit_opcode(chunk, OP_RETURN);
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

static void compile_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  compile_and_expression(comp);
  while (MATCH(scan, TOKEN_PIPEPIPE))
  {
    scanner_next_token(scan);
    int offset = emit_jump(&comp->fn->chunk, OP_OR);
    compile_and_expression(comp);
    patch_jump(comp, offset);
  }
}

static void compile_and_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  compile_equal_expression(comp);
  while (MATCH(scan, TOKEN_AMPAMP))
  {
    scanner_next_token(scan);
    int offset = emit_jump(&comp->fn->chunk, OP_AND);
    compile_equal_expression(comp);
    patch_jump(comp, offset);
  }
}

static void compile_equal_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
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
      chunk_emit_opcode(chunk, OP_NOT_EQUAL);
      continue;
    }
    break;
  }
}

static void compile_comp_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  compile_add_expression(comp);
  for (;;)
  {
    int line = scan->token.line;
    if (MATCH(scan, TOKEN_GT))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_GREATER);
      function_add_line(fn, line);
      continue;
    }
    if (MATCH(scan, TOKEN_GTEQ))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_NOT_LESS);
      function_add_line(fn, line);
      continue;
    }
    if (MATCH(scan, TOKEN_LT))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_LESS);
      function_add_line(fn, line);
      continue;
    }
    if (MATCH(scan, TOKEN_LTEQ))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_NOT_GREATER);
      function_add_line(fn, line);
      continue;
    }
    break;
  }
}

static void compile_add_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  compile_mul_expression(comp);
  for (;;)
  {
    int line = scan->token.line;
    if (MATCH(scan, TOKEN_PLUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(comp);
      chunk_emit_opcode(chunk, OP_ADD);
      function_add_line(fn, line);
      continue;
    }
    if (MATCH(scan, TOKEN_MINUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(comp);
      chunk_emit_opcode(chunk, OP_SUBTRACT);
      function_add_line(fn, line);
      continue;
    }
    break;
  }
}

static void compile_mul_expression(compiler_t *comp)
{
  compile_unary_expression(comp);
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  for (;;)
  {
    int line = scan->token.line;
    if (MATCH(scan, TOKEN_STAR))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_MULTIPLY);
      function_add_line(fn, line);
      continue;
    }
    if (MATCH(scan, TOKEN_SLASH))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_DIVIDE);
      function_add_line(fn, line);
      continue;
    }
    if (MATCH(scan, TOKEN_PERCENT))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_MODULO);
      function_add_line(fn, line);
      continue;
    }
    break;
  }
}

static void compile_unary_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  if (MATCH(scan, TOKEN_MINUS))
  {
    int line = scan->token.line;
    scanner_next_token(scan);
    compile_unary_expression(comp);
    chunk_emit_opcode(chunk, OP_NEGATE);
    function_add_line(fn, line);
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
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  int line = scan->token.line;
  if (MATCH(scan, TOKEN_NIL))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_NIL);
    function_add_line(fn, line);
    return;
  }
  if (MATCH(scan, TOKEN_FALSE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_FALSE);
    function_add_line(fn, line);
    return;
  }
  if (MATCH(scan, TOKEN_TRUE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_TRUE);
    function_add_line(fn, line);
    return;
  }
  if (MATCH(scan, TOKEN_INT))
  {
    double data = parse_double(comp);
    scanner_next_token(scan);
    if (data <= UINT16_MAX)
    {
      chunk_emit_opcode(chunk, OP_INT);
      chunk_emit_word(chunk, (uint16_t) data);
      function_add_line(fn, line);
      return;
    }
    uint8_t index = add_number_constant(comp, data);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, line);
    return;
  }
  if (MATCH(scan, TOKEN_FLOAT))
  {
    double data = parse_double(comp);
    scanner_next_token(scan);
    uint8_t index = add_number_constant(comp, data);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, line);
    return;
  }
  if (MATCH(scan, TOKEN_STRING))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, line);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    compile_array_constructor(comp);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACE))
  {
    compile_struct_constructor(comp);
    return;
  }
  if (MATCH(scan, TOKEN_STRUCT))
  {
    compile_struct_declaration(comp, true);
    return;
  }
  if (MATCH(scan, TOKEN_FN))
  {
    compile_function_declaration(comp, true);
    return;
  }
  if (MATCH(scan, TOKEN_IF))
  {
    compile_if_expression(comp);
    return;
  }
  if (MATCH(scan, TOKEN_MATCH))
  {
    compile_match_expression(comp);
    return;
  }
  if (MATCH(scan, TOKEN_NAME))
  {
    compile_subscript(comp);
    return;
  }
  if (MATCH(scan, TOKEN_LPAREN))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    EXPECT(comp, TOKEN_RPAREN);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_array_constructor(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  int line = scan->token.line;
  scanner_next_token(scan);
  uint8_t length = 0;
  if (MATCH(scan, TOKEN_RBRACKET))
  {
    scanner_next_token(scan);
    goto end;
  }
  compile_expression(comp);
  ++length;
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    ++length;
  }
  EXPECT(comp, TOKEN_RBRACKET);
end:
  chunk_emit_opcode(chunk, OP_ARRAY);
  chunk_emit_byte(chunk, length);
  function_add_line(fn, line);
  return;
}

static void compile_struct_constructor(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  int line = scan->token.line;
  scanner_next_token(scan);
  chunk_emit_opcode(chunk, OP_NIL);
  function_add_line(fn, line);
  if (MATCH(scan, TOKEN_RBRACE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_CONSTRUCT);
    chunk_emit_byte(chunk, 0);
    function_add_line(fn, line);
    return;
  }
  if (!MATCH(scan, TOKEN_NAME))
    syntax_error_unexpected(comp);
  token_t tk = scan->token;
  scanner_next_token(scan);
  uint8_t index = add_string_constant(comp, &tk);
  chunk_emit_opcode(chunk, OP_CONSTANT);
  chunk_emit_byte(chunk, index);
  function_add_line(fn, tk.line);
  EXPECT(comp, TOKEN_COLON);
  compile_expression(comp);
  uint8_t length = 1;
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    index = add_string_constant(comp, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk.line);
    EXPECT(comp, TOKEN_COLON);
    compile_expression(comp);
    ++length;
  }
  EXPECT(comp, TOKEN_RBRACE);
  chunk_emit_opcode(chunk, OP_CONSTRUCT);
  chunk_emit_byte(chunk, length);
  function_add_line(fn, line);
}

static void compile_if_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  EXPECT(comp, TOKEN_LPAREN);
  compile_expression(comp);
  EXPECT(comp, TOKEN_RPAREN);
  int offset1 = emit_jump(chunk, OP_JUMP_IF_FALSE);
  compile_expression(comp);
  int offset2 = emit_jump(chunk, OP_JUMP);
  patch_jump(comp, offset1);
  EXPECT(comp, TOKEN_ELSE);
  compile_expression(comp);
  patch_jump(comp, offset2);
}

static void compile_match_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  EXPECT(comp, TOKEN_LPAREN);
  compile_expression(comp);
  EXPECT(comp, TOKEN_RPAREN);
  EXPECT(comp, TOKEN_LBRACE);
  compile_expression(comp);
  EXPECT(comp, TOKEN_ARROW);
  int offset1 = emit_jump(chunk, OP_MATCH);
  compile_expression(comp);
  int offset2 = emit_jump(chunk, OP_JUMP);
  patch_jump(comp, offset1);
  if (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_UNDERSCORE))
    {
      scanner_next_token(scan);
      EXPECT(comp, TOKEN_ARROW);
      chunk_emit_opcode(chunk, OP_POP);
      compile_expression(comp);
      EXPECT(comp, TOKEN_RBRACE);
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
  chunk_t *chunk = &comp->fn->chunk;
  compile_expression(comp);
  EXPECT(comp, TOKEN_ARROW);
  int offset1 = emit_jump(chunk, OP_MATCH);
  compile_expression(comp);
  int offset2 = emit_jump(chunk, OP_JUMP);
  patch_jump(comp, offset1);
  if (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_UNDERSCORE))
    {
      scanner_next_token(scan);
      EXPECT(comp, TOKEN_ARROW);
      chunk_emit_opcode(chunk, OP_POP);
      compile_expression(comp);
      EXPECT(comp, TOKEN_RBRACE);
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
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  compile_variable(comp, &scan->token, true);
  scanner_next_token(scan);
  for (;;)
  {
    int line = scan->token.line;
    if (MATCH(scan, TOKEN_LBRACKET))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      EXPECT(comp, TOKEN_RBRACKET);
      chunk_emit_opcode(chunk, OP_GET_ELEMENT);
      function_add_line(fn, line);
      continue;
    }
    if (MATCH(scan, TOKEN_DOT))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        syntax_error_unexpected(comp);
      token_t tk = scan->token;
      scanner_next_token(scan);
      uint8_t index = add_string_constant(comp, &tk);
      chunk_emit_opcode(chunk, OP_GET_FIELD);
      chunk_emit_byte(chunk, index);
      function_add_line(fn, line);
      continue;
    }
    if (MATCH(scan, TOKEN_LPAREN))
    {
      scanner_next_token(scan);
      if (MATCH(scan, TOKEN_RPAREN))
      {
        scanner_next_token(scan);
        chunk_emit_opcode(chunk, OP_CALL);
        chunk_emit_byte(chunk, 0);
        function_add_line(fn, line);
        return;
      }
      compile_expression(comp);
      uint8_t num_args = 1;
      while (MATCH(scan, TOKEN_COMMA))
      {
        scanner_next_token(scan);
        compile_expression(comp);
        ++num_args;
      }
      EXPECT(comp, TOKEN_RPAREN);
      chunk_emit_opcode(chunk, OP_CALL);
      chunk_emit_byte(chunk, num_args);
      function_add_line(fn, line);
      continue;
    }
    break;
  }
  if (MATCH(scan, TOKEN_LBRACE))
  {
    int line = scan->token.line;
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_RBRACE))
    {
      scanner_next_token(scan);
      chunk_emit_opcode(chunk, OP_INSTANCE);
      chunk_emit_byte(chunk, 0);
      function_add_line(fn, line);
      return;
    }
    compile_expression(comp);
    uint8_t num_args = 1;
    while (MATCH(scan, TOKEN_COMMA))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      ++num_args;
    }
    EXPECT(comp, TOKEN_RBRACE);
    chunk_emit_opcode(chunk, OP_INSTANCE);
    chunk_emit_byte(chunk, num_args);
    function_add_line(fn, line);
  }
}

static variable_t compile_variable(compiler_t *comp, token_t *tk, bool emit)
{
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  variable_t *var = lookup_variable(comp, tk);
  if (var)
  {
    if (!emit)
      return *var;
    chunk_emit_opcode(chunk, var->is_local ? OP_GET_LOCAL : OP_NONLOCAL);
    chunk_emit_byte(chunk, var->index);
    function_add_line(fn, tk->line);
    return *var;
  }
  var = compile_nonlocal(comp->parent, tk);
  if (var)
  {
    uint8_t index = add_nonlocal(comp, tk);
    chunk_emit_opcode(chunk, OP_NONLOCAL);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk->line);
    return *var;
  }
  int index = lookup_global(tk->length, tk->start);
  if (index == -1)
    syntax_error(fn->name->chars, comp->scan->file->chars, tk->line, tk->col,
      "variable `%.*s` is used but not defined", tk->length, tk->start);
  chunk_emit_opcode(chunk, OP_GLOBAL);
  chunk_emit_byte(chunk, (uint8_t) index);
  function_add_line(fn, tk->line);
  return (variable_t) {.is_local = false, .depth = -1, .index = index, .length = tk->length,
    .start = tk->start, .is_mutable = false};
}

static variable_t *compile_nonlocal(compiler_t *comp, token_t *tk)
{
  if (!comp)
    return NULL;
  function_t *fn = comp->fn;
  chunk_t *chunk = &fn->chunk;
  variable_t *var = lookup_variable(comp, tk);
  if (var)
  {
    opcode_t op = OP_NONLOCAL;
    if (var->is_local)
    {
      if (var->is_mutable)
        syntax_error(fn->name->chars, comp->scan->file->chars, tk->line, tk->col,
          "cannot capture mutable variable `%.*s`", tk->length, tk->start);
      op = OP_GET_LOCAL;
    }
    chunk_emit_opcode(chunk, op);
    chunk_emit_byte(chunk, var->index);
    function_add_line(fn, tk->line);
    return var;
  }
  var = compile_nonlocal(comp->parent, tk);
  if (var)
  {
    uint8_t index = add_nonlocal(comp, tk);
    chunk_emit_opcode(chunk, OP_NONLOCAL);
    chunk_emit_byte(chunk, index);
    function_add_line(fn, tk->line);
    return var;
  }
  return NULL;
}

closure_t *compile(string_t *file, string_t *source)
{
  scanner_t scan;
  scanner_init(&scan, file, source);
  compiler_t comp;
  compiler_init(&comp, NULL, &scan, string_from_chars(-1, "main"));
  char args_name[] = "args";
  token_t tk = {.length = sizeof(args_name) - 1, .start = args_name};
  add_local(&comp, &tk, false);
  while (!MATCH(comp.scan, TOKEN_EOF))
    compile_statement(&comp);
  function_t *fn = comp.fn;
  chunk_t *chunk = &fn->chunk;
  chunk_emit_opcode(chunk, OP_RETURN_NIL);
  function_add_line(fn, scan.token.line);
  closure_t *cl = closure_new(fn);
  scanner_free(&scan);
  return cl;
}
