//
// Hook Programming Language
// compiler.c
//

#include "compiler.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "struct.h"
#include "builtin.h"
#include "error.h"

#define MAX_CONSTANTS (UINT8_MAX + 1)
#define MAX_VARIABLES (UINT8_MAX + 1)
#define MAX_BREAKS    (UINT8_MAX + 1)

#define MATCH(s, t) ((s)->token.type == (t))

#define EXPECT(s, t) do \
  { \
    if (!MATCH(s, t)) \
      fatal_error_unexpected_token(s); \
    scanner_next_token(s); \
  } while(0)

#define SYN_NONE      0
#define SYN_ASSIGN    1
#define SYN_CALL      2
#define SYN_SUBSCRIPT 3

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
  prototype_t *proto;
} compiler_t;

static inline void fatal_error_unexpected_token(scanner_t *scan);
static inline double parse_double(token_t *tk);
static inline bool string_match(token_t *tk, string_t *str);
static inline uint8_t add_number_constant(prototype_t *proto, double data);
static inline uint8_t add_string_constant(prototype_t *proto, token_t *tk);
static inline uint8_t add_constant(prototype_t *proto, value_t val);
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
static inline void patch_jump(chunk_t *chunk, int offset);
static inline void start_loop(compiler_t *comp, loop_t *loop);
static inline void end_loop(compiler_t *comp);
static inline void compiler_init(compiler_t *comp, compiler_t *parent, scanner_t *scan,
  string_t *name);
static void compile_statement(compiler_t *comp);
static void compile_block(compiler_t *comp);
static void compile_variable_declaration(compiler_t *comp);
static void compile_assign_statement(compiler_t *comp, token_t *tk);
static int compile_assign(compiler_t *comp, int syntax, bool inplace);
static void compile_struct_declaration(compiler_t *comp, bool is_anonymous);
static void compile_function_declaration(compiler_t *comp, bool is_anonymous);
static void compile_delete_statement(compiler_t *comp);
static void compile_delete(compiler_t *comp, bool inplace);
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
static void compile_array_initializer(compiler_t *comp);
static void compile_struct_initializer(compiler_t *comp);
static void compile_if_expression(compiler_t *comp);
static void compile_subscript(compiler_t *comp);
static variable_t compile_variable(compiler_t *comp, token_t *tk, bool emit);
static variable_t *compile_nonlocal(compiler_t *comp, token_t *tk);

static inline void fatal_error_unexpected_token(scanner_t *scan)
{
  token_t *tk = &scan->token;
  if (tk->type == TOKEN_EOF)
    fatal_error("unexpected end of file at %d:%d", tk->line, tk->col);
  fatal_error("unexpected token '%.*s' at %d:%d", tk->length, tk->start,
    tk->line, tk->col);
}

static inline double parse_double(token_t *tk)
{
  errno = 0;
  double result = strtod(tk->start, NULL);
  if (errno == ERANGE)
    fatal_error("floating point number too large at %d:%d", tk->line, tk->col);
  return result;
}

static inline bool string_match(token_t *tk, string_t *str)
{
  return tk->length == str->length
    && !memcmp(tk->start, str->chars, tk->length);
}

static inline uint8_t add_number_constant(prototype_t *proto, double data)
{
  array_t *consts = proto->consts;
  value_t *elements = consts->elements;
  for (int i = 0; i < consts->length; ++i)
  {
    value_t elem = elements[i];
    if (elem.type != TYPE_NUMBER)
      continue;
    if (data == elem.as.number)
      return (uint8_t) i;
  }
  return add_constant(proto, NUMBER_VALUE(data));
}

static inline uint8_t add_string_constant(prototype_t *proto, token_t *tk)
{
  array_t *consts = proto->consts;
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
  return add_constant(proto, STRING_VALUE(str));
}

static inline uint8_t add_constant(prototype_t *proto, value_t val)
{
  array_t *consts = proto->consts;
  if (consts->length == MAX_CONSTANTS)
    fatal_error("a function may only contain %d unique constants", MAX_CONSTANTS);
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
  chunk_t *chunk = &comp->proto->chunk;
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
  uint8_t index = comp->proto->num_nonlocals++;
  add_variable(comp, false, index, tk, false);
  return index;
}

static inline void add_variable(compiler_t *comp, bool is_local, uint8_t index, token_t *tk,
  bool is_mutable)
{
  if (comp->num_variables == MAX_VARIABLES)
    fatal_error("cannot declare more than %d variables in one scope at %d:%d",
      MAX_VARIABLES, tk->line, tk->col);
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
      fatal_error("variable '%.*s' is already defined in this scope at %d:%d",
        tk->length, tk->start, tk->line, tk->col);
  }
  add_local(comp, tk, is_mutable);
}

static inline variable_t resolve_variable(compiler_t *comp, token_t *tk)
{
  variable_t *var = lookup_variable(comp, tk);
  if (var)
    return *var;
  if (!nonlocal_exists(comp->parent, tk) && lookup_global(tk->length, tk->start) == -1)
  {
    fatal_error("variable '%.*s' is used but not defined at %d:%d", tk->length,
      tk->start, tk->line, tk->col);
  }
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
    compile_variable_declaration(comp);
    EXPECT(scan, TOKEN_SEMICOLON);
    return;
  }
  if (MATCH(scan, TOKEN_NAME))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    compile_assign_statement(comp, &tk);
    EXPECT(scan, TOKEN_SEMICOLON);
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

static void compile_variable_declaration(compiler_t *comp)
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
  if (MATCH(scan, TOKEN_MUT))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    define_local(comp, &scan->token, true);
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      return;  
    }
    chunk_emit_opcode(chunk, OP_NULL);
    prototype_add_line(proto, scan->line);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    bool is_mutable = false;
    if (MATCH(scan, TOKEN_MUT))
    {
      scanner_next_token(scan);
      is_mutable = true;
    }
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    define_local(comp, &scan->token, is_mutable);
    scanner_next_token(scan);
    uint8_t n = 1;
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
        fatal_error_unexpected_token(scan);
      define_local(comp, &scan->token, is_mutable);
      scanner_next_token(scan);
      ++n;
    }
    EXPECT(scan, TOKEN_RBRACKET);
    EXPECT(scan, TOKEN_EQ);
    int line = scan->token.line;
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_UNPACK);
    chunk_emit_byte(chunk, n);
    prototype_add_line(proto, line);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACE))
  {
    scanner_next_token(scan);
    bool is_mutable = false;
    if (MATCH(scan, TOKEN_MUT))
    {
      scanner_next_token(scan);
      is_mutable = true;
    }
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, is_mutable);
    uint8_t index = add_string_constant(proto, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    prototype_add_line(proto, tk.line);
    uint8_t n = 1;
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
        fatal_error_unexpected_token(scan);
      token_t tk = scan->token;
      scanner_next_token(scan);
      define_local(comp, &tk, is_mutable);
      uint8_t index = add_string_constant(proto, &tk);
      chunk_emit_opcode(chunk, OP_CONSTANT);
      chunk_emit_byte(chunk, index);
      prototype_add_line(proto, tk.line);
      ++n;
    }
    EXPECT(scan, TOKEN_RBRACE);
    EXPECT(scan, TOKEN_EQ);
    int line = scan->token.line;
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_DESTRUCT);
    chunk_emit_byte(chunk, n);
    prototype_add_line(proto, line);
    return;
  }
  fatal_error_unexpected_token(scan);
}

static void compile_assign_statement(compiler_t *comp, token_t *tk)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
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
    fatal_error("cannot assign to immutable variable '%.*s' at %d:%d",
      tk->length, tk->start, tk->line, tk->col);
  chunk_emit_opcode(chunk, OP_SET_LOCAL);
  chunk_emit_byte(chunk, var.index);
}

static int compile_assign(compiler_t *comp, int syntax, bool inplace)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  int line = scan->token.line;
  if (MATCH(scan, TOKEN_PLUSEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_ADD);
    prototype_add_line(proto, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_MINUSEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_SUBTRACT);
    prototype_add_line(proto, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_STAREQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_MULTIPLY);
    prototype_add_line(proto, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_SLASHEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_DIVIDE);
    prototype_add_line(proto, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_PERCENTEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    chunk_emit_opcode(chunk, OP_MODULO);
    prototype_add_line(proto, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_PLUSPLUS))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_INT);
    chunk_emit_word(chunk, 1);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_ADD);
    prototype_add_line(proto, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_MINUSMINUS))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_INT);
    chunk_emit_word(chunk, 1);
    prototype_add_line(proto, line);
    chunk_emit_opcode(chunk, OP_SUBTRACT);
    prototype_add_line(proto, line);
    return SYN_ASSIGN;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    scanner_next_token(scan);
    if (MATCH(scan, TOKEN_RBRACKET))
    {
      scanner_next_token(scan);
      EXPECT(scan, TOKEN_EQ);
      compile_expression(comp);
      chunk_emit_opcode(chunk, inplace ? OP_INPLACE_ADD_ELEMENT : OP_ADD_ELEMENT);
      prototype_add_line(proto, line);
      return SYN_ASSIGN;
    }
    compile_expression(comp);
    EXPECT(scan, TOKEN_RBRACKET);
    if (MATCH(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, inplace ? OP_INPLACE_PUT_ELEMENT : OP_PUT_ELEMENT);
      prototype_add_line(proto, line);
      return SYN_ASSIGN;
    }
    int offset = chunk->length;
    chunk_emit_opcode(chunk, OP_GET_ELEMENT);
    prototype_add_line(proto, line);
    int syn = compile_assign(comp, SYN_SUBSCRIPT, false);
    if (syn == SYN_ASSIGN) {
      chunk->bytes[offset] = (uint8_t) OP_FETCH_ELEMENT;
      chunk_emit_opcode(chunk, OP_SET_ELEMENT);
    }
    return syn;
  }
  if (MATCH(scan, TOKEN_DOT))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(proto, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    prototype_add_line(proto, tk.line);
    if (MATCH(scan, TOKEN_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      chunk_emit_opcode(chunk, inplace ? OP_INPLACE_PUT_FIELD : OP_PUT_FIELD);
      prototype_add_line(proto, tk.line);
      return SYN_ASSIGN;
    }
    int offset = chunk->length;
    chunk_emit_opcode(chunk, OP_GET_FIELD);
    prototype_add_line(proto, tk.line);
    int syn = compile_assign(comp, SYN_SUBSCRIPT, false);
    if (syn == SYN_ASSIGN) {
      chunk->bytes[offset] = (uint8_t) OP_FETCH_FIELD;
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
      prototype_add_line(proto, line);
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
    EXPECT(scan, TOKEN_RPAREN);
    chunk_emit_opcode(chunk, OP_CALL);
    chunk_emit_byte(chunk, num_args);
    prototype_add_line(proto, line);
    return compile_assign(comp, SYN_CALL, false);
  }
  if (syntax == SYN_NONE || syntax == SYN_SUBSCRIPT)
    fatal_error_unexpected_token(scan);
  return syntax;
}

static void compile_struct_declaration(compiler_t *comp, bool is_anonymous)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  int line = scan->token.line;
  scanner_next_token(scan);
  token_t tk;
  string_t *name = NULL;
  if (!is_anonymous)
  {
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    name = string_from_chars(tk.length, tk.start);
  }
  struct_t *ztruct = struct_new(name);
  EXPECT(scan, TOKEN_LBRACE);
  if (MATCH(scan, TOKEN_RBRACE))
  {
    scanner_next_token(scan);
    goto end;
  }
  if (!MATCH(scan, TOKEN_NAME))
    fatal_error_unexpected_token(scan);
  tk = scan->token;
  scanner_next_token(scan);
  if (!struct_put_if_absent(ztruct, tk.length, tk.start))
    fatal_error("field `%.*s` is already declared at %d:%d", tk.length,
      tk.start, tk.line, tk.col);
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    tk = scan->token;
    scanner_next_token(scan);
    if (!struct_put_if_absent(ztruct, tk.length, tk.start))
      fatal_error("field `%.*s` is already declared at %d:%d", tk.length,
        tk.start, tk.line, tk.col);
  }
  EXPECT(scan, TOKEN_RBRACE);
  uint8_t index;
end:
  index = add_constant(proto, STRUCT_VALUE(ztruct));
  chunk_emit_opcode(chunk, OP_CONSTANT);
  chunk_emit_byte(chunk, index);
  prototype_add_line(proto, line);
}

static void compile_function_declaration(compiler_t *comp, bool is_anonymous)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  int line = scan->token.line;
  scanner_next_token(scan);
  compiler_t child_comp;
  if (!is_anonymous)
  {
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    token_t tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    string_t *name = string_from_chars(tk.length, tk.start);
    compiler_init(&child_comp, comp, scan, name);
    add_variable(&child_comp, true, 0, &tk, false);
  }
  else
    compiler_init(&child_comp, comp, scan, NULL);
  chunk_t *child_chunk = &child_comp.proto->chunk;
  EXPECT(scan, TOKEN_LPAREN);
  if (MATCH(scan, TOKEN_RPAREN))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_LBRACE))
      fatal_error_unexpected_token(scan);
    compile_block(&child_comp);
    chunk_emit_opcode(child_chunk, OP_NULL);
    prototype_add_line(proto, scan->line);
    goto end;
  }
  bool is_mutable = false;
  if (MATCH(scan, TOKEN_MUT))
  {
    scanner_next_token(scan);
    is_mutable = true;
  }
  if (!MATCH(scan, TOKEN_NAME))
    fatal_error_unexpected_token(scan);
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
      fatal_error_unexpected_token(scan);
    define_local(&child_comp, &scan->token, is_mutable);
    scanner_next_token(scan);
    ++arity;
  }
  child_comp.proto->arity = arity;
  EXPECT(scan, TOKEN_RPAREN);
  if (!MATCH(scan, TOKEN_LBRACE))
    fatal_error_unexpected_token(scan);
  compile_block(&child_comp);
  chunk_emit_opcode(child_chunk, OP_NULL);
  prototype_add_line(proto, scan->line);
end:
  chunk_emit_opcode(child_chunk, OP_RETURN);
  uint8_t index = proto->num_protos;
  prototype_add_child(proto, child_comp.proto);
  chunk_emit_opcode(chunk, OP_FUNCTION);
  chunk_emit_byte(chunk, index);
  prototype_add_line(proto, line);
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
  prototype_add_line(proto, tk.line);
  compile_delete(comp, true);
  chunk_emit_opcode(chunk, OP_SET_LOCAL);
  chunk_emit_byte(chunk, var.index);
}

static void compile_delete(compiler_t *comp, bool inplace)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    int line = scan->token.line;
    scanner_next_token(scan);
    compile_expression(comp);
    EXPECT(scan, TOKEN_RBRACKET);
    if (MATCH(scan, TOKEN_SEMICOLON))
    {
      scanner_next_token(scan);
      chunk_emit_opcode(chunk, inplace ? OP_INPLACE_DELETE_ELEMENT : OP_DELETE_ELEMENT);
      prototype_add_line(proto, line);
      return;
    }
    chunk_emit_opcode(chunk, OP_FETCH_ELEMENT);
    prototype_add_line(proto, line);
    compile_delete(comp, false);
    chunk_emit_opcode(chunk, OP_SET_ELEMENT);
    return;
  }
  if (MATCH(scan, TOKEN_DOT))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(proto, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    prototype_add_line(proto, tk.line);
    chunk_emit_opcode(chunk, OP_FETCH_FIELD);
    prototype_add_line(proto, tk.line);
    compile_delete(comp, false);
    chunk_emit_opcode(chunk, OP_SET_FIELD);
    return;
  }
  fatal_error_unexpected_token(scan); 
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
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  scanner_next_token(scan);
  EXPECT(scan, TOKEN_LPAREN);
  push_scope(comp);
  if (MATCH(scan, TOKEN_SEMICOLON))
    scanner_next_token(scan);
  else
  {
    if (MATCH(scan, TOKEN_LET))
    {
      compile_variable_declaration(comp);
      EXPECT(scan, TOKEN_SEMICOLON);
    }
    else if (MATCH(scan, TOKEN_NAME))
    {
      token_t tk = scan->token;
      scanner_next_token(scan);
      compile_assign_statement(comp, &tk);
      EXPECT(scan, TOKEN_SEMICOLON);
    }
    else
      fatal_error_unexpected_token(scan);
  }
  uint16_t jump1 = (uint16_t) chunk->length;
  if (MATCH(scan, TOKEN_SEMICOLON))
  {
    int line = scan->token.line;
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_TRUE);
    prototype_add_line(proto, line);
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
    compile_assign_statement(comp, &tk);
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
  if (loop->num_offsets == MAX_BREAKS)
    fatal_error("too many breaks at %d:%d", tk.line, tk.col);
  int offset = emit_jump(&comp->proto->chunk, OP_JUMP);
  loop->offsets[loop->num_offsets++] = offset;
}

static void compile_return_statement(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  scanner_next_token(scan);
  if (MATCH(scan, TOKEN_SEMICOLON))
  {
    int line = scan->token.line;
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_NULL);
    prototype_add_line(proto, line);
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
    int line = scan->token.line;
    if (MATCH(scan, TOKEN_GT))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_GREATER);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_GTEQ))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_LESS);
      prototype_add_line(proto, line);
      chunk_emit_opcode(chunk, OP_NOT);
      continue;
    }
    if (MATCH(scan, TOKEN_LT))
    {
      scanner_next_token(scan);
      compile_add_expression(comp);
      chunk_emit_opcode(chunk, OP_LESS);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_LTEQ))
    {
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
    int line = scan->token.line;
    if (MATCH(scan, TOKEN_PLUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(comp);
      chunk_emit_opcode(chunk, OP_ADD);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_MINUS))
    {
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
    int line = scan->token.line;
    if (MATCH(scan, TOKEN_STAR))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_MULTIPLY);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_SLASH))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      chunk_emit_opcode(chunk, OP_DIVIDE);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_PERCENT))
    {
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
    int line = scan->token.line;
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
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  int line = scan->token.line;
  if (MATCH(scan, TOKEN_NULL))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_NULL);
    prototype_add_line(proto, line);
    return;
  }
  if (MATCH(scan, TOKEN_FALSE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_FALSE);
    prototype_add_line(proto, line);
    return;
  }
  if (MATCH(scan, TOKEN_TRUE))
  {
    scanner_next_token(scan);
    chunk_emit_opcode(chunk, OP_TRUE);
    prototype_add_line(proto, line);
    return;
  }
  if (MATCH(scan, TOKEN_INT))
  {
    double data = parse_double(&scan->token);
    scanner_next_token(scan);
    if (data <= UINT16_MAX)
    {
      chunk_emit_opcode(chunk, OP_INT);
      chunk_emit_word(chunk, (uint16_t) data);
      prototype_add_line(proto, line);
      return;
    }
    uint8_t index = add_number_constant(proto, data);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    prototype_add_line(proto, line);
    return;
  }
  if (MATCH(scan, TOKEN_FLOAT))
  {
    double data = parse_double(&scan->token);
    scanner_next_token(scan);
    uint8_t index = add_number_constant(proto, data);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    prototype_add_line(proto, line);
    return;
  }
  if (MATCH(scan, TOKEN_STRING))
  {
    token_t tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(proto, &tk);
    chunk_emit_opcode(chunk, OP_CONSTANT);
    chunk_emit_byte(chunk, index);
    prototype_add_line(proto, line);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACKET))
  {
    compile_array_initializer(comp);
    return;
  }
  if (MATCH(scan, TOKEN_LBRACE))
  {
    compile_struct_initializer(comp);
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
  if (MATCH(scan, TOKEN_NAME))
  {
    compile_subscript(comp);
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

static void compile_array_initializer(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
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
  EXPECT(scan, TOKEN_RBRACKET);
end:
  chunk_emit_opcode(chunk, OP_ARRAY);
  chunk_emit_byte(chunk, length);
  prototype_add_line(proto, line);
  return;
}

static void compile_struct_initializer(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  int line = scan->token.line;
  scanner_next_token(scan);
  struct_t *ztruct = struct_new(NULL);
  if (MATCH(scan, TOKEN_RBRACE))
  {
    scanner_next_token(scan);
    goto end;
  }
  if (!MATCH(scan, TOKEN_NAME))
    fatal_error_unexpected_token(scan);
  token_t tk = scan->token;
  scanner_next_token(scan);
  if (!struct_put_if_absent(ztruct, tk.length, tk.start))
    fatal_error("field `%.*s` is already declared at %d:%d", tk.length,
      tk.start, tk.line, tk.col);
  EXPECT(scan, TOKEN_COLON);
  compile_expression(comp);
  while (MATCH(scan, TOKEN_COMMA))
  {
    scanner_next_token(scan);
    if (!MATCH(scan, TOKEN_NAME))
      fatal_error_unexpected_token(scan);
    tk = scan->token;
    scanner_next_token(scan);
    if (!struct_put_if_absent(ztruct, tk.length, tk.start))
      fatal_error("field `%.*s` is already declared at %d:%d", tk.length,
        tk.start, tk.line, tk.col);
    EXPECT(scan, TOKEN_COLON);
    compile_expression(comp);
  }
  EXPECT(scan, TOKEN_RBRACE);
  uint8_t index;
end:
  index = add_constant(proto, STRUCT_VALUE(ztruct));
  chunk_emit_opcode(chunk, OP_CONSTANT);
  chunk_emit_byte(chunk, index);
  prototype_add_line(proto, line);
  chunk_emit_opcode(chunk, OP_INSTANCE);
  prototype_add_line(proto, line);
}

static void compile_if_expression(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  chunk_t *chunk = &comp->proto->chunk;
  scanner_next_token(scan);
  EXPECT(scan, TOKEN_LPAREN);
  compile_expression(comp);
  EXPECT(scan, TOKEN_RPAREN);
  int offset1 = emit_jump(chunk, OP_JUMP_IF_FALSE);
  chunk_emit_opcode(chunk, OP_POP);
  compile_expression(comp);
  int offset2 = emit_jump(chunk, OP_JUMP);
  patch_jump(chunk, offset1);
  chunk_emit_opcode(chunk, OP_POP);
  EXPECT(scan, TOKEN_ELSE);
  compile_expression(comp);
  patch_jump(chunk, offset2);
}

static void compile_subscript(compiler_t *comp)
{
  scanner_t *scan = comp->scan;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  compile_variable(comp, &scan->token, true);
  scanner_next_token(scan);
  for (;;)
  {
    int line = scan->token.line;
    if (MATCH(scan, TOKEN_LBRACKET))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      EXPECT(scan, TOKEN_RBRACKET);
      chunk_emit_opcode(chunk, OP_GET_ELEMENT);
      prototype_add_line(proto, line);
      continue;
    }
    if (MATCH(scan, TOKEN_DOT))
    {
      scanner_next_token(scan);
      if (!MATCH(scan, TOKEN_NAME))
        fatal_error_unexpected_token(scan);
      token_t tk = scan->token;
      scanner_next_token(scan);
      uint8_t index = add_string_constant(proto, &tk);
      chunk_emit_opcode(chunk, OP_CONSTANT);
      chunk_emit_byte(chunk, index);
      prototype_add_line(proto, tk.line);
      chunk_emit_opcode(chunk, OP_GET_FIELD);
      prototype_add_line(proto, line);
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
        prototype_add_line(proto, line);
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
      EXPECT(scan, TOKEN_RPAREN);
      chunk_emit_opcode(chunk, OP_CALL);
      chunk_emit_byte(chunk, num_args);
      prototype_add_line(proto, line);
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
      chunk_emit_opcode(chunk, OP_INITILIZE);
      chunk_emit_byte(chunk, 0);
      prototype_add_line(proto, line);
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
    EXPECT(scan, TOKEN_RBRACE);
    chunk_emit_opcode(chunk, OP_INITILIZE);
    chunk_emit_byte(chunk, num_args);
    prototype_add_line(proto, line);
  }
}

static variable_t compile_variable(compiler_t *comp, token_t *tk, bool emit)
{
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
  variable_t *var = lookup_variable(comp, tk);
  if (var)
  {
    if (!emit)
      return *var;
    chunk_emit_opcode(chunk, var->is_local ? OP_GET_LOCAL : OP_NONLOCAL);
    chunk_emit_byte(chunk, var->index);
    prototype_add_line(proto, tk->line);
    return *var;
  }
  var = compile_nonlocal(comp->parent, tk);
  if (var)
  {
    uint8_t index = add_nonlocal(comp, tk);
    chunk_emit_opcode(chunk, OP_NONLOCAL);
    chunk_emit_byte(chunk, index);
    prototype_add_line(proto, tk->line);
    return *var;
  }
  int index = lookup_global(tk->length, tk->start);
  if (index == -1)
    fatal_error("variable '%.*s' is used but not defined at %d:%d", tk->length,
      tk->start, tk->line, tk->col);
  chunk_emit_opcode(chunk, OP_GLOBAL);
  chunk_emit_byte(chunk, (uint8_t) index);
  prototype_add_line(proto, tk->line);
  return (variable_t) {.is_local = false, .depth = -1, .index = index, .length = tk->length,
    .start = tk->start, .is_mutable = false};
}

static variable_t *compile_nonlocal(compiler_t *comp, token_t *tk)
{
  if (!comp)
    return false;
  prototype_t *proto = comp->proto;
  chunk_t *chunk = &proto->chunk;
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
    prototype_add_line(proto, tk->line);
    return var;
  }
  var = compile_nonlocal(comp->parent, tk);
  if (var)
  {
    uint8_t index = add_nonlocal(comp, tk);
    chunk_emit_opcode(chunk, OP_NONLOCAL);
    chunk_emit_byte(chunk, index);
    prototype_add_line(proto, tk->line);
    return var;
  }
  return NULL;
}

prototype_t *compile(scanner_t *scan)
{
  compiler_t comp;
  compiler_init(&comp, NULL, scan, string_from_chars(-1, "main"));
  char args_name[] = "args";
  token_t tk = {.length = sizeof(args_name) - 1, .start = args_name};
  add_local(&comp, &tk, false);
  while (!MATCH(comp.scan, TOKEN_EOF))
    compile_statement(&comp);
  prototype_t *proto = comp.proto;
  chunk_t *chunk = &proto->chunk;
  chunk_emit_opcode(chunk, OP_NULL);
  prototype_add_line(proto, scan->line);
  chunk_emit_opcode(chunk, OP_RETURN);
  return comp.proto;
}
