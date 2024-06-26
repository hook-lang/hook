//
// The Hook Programming Language
// compiler.c
//

#include <hook/compiler.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <hook/struct.h>
#include <hook/utils.h>
#include "builtin.h"
#include "scanner.h"

#define MAX_CONSTANTS UINT8_MAX
#define MAX_VARIABLES UINT8_MAX
#define MAX_BREAKS    UINT8_MAX

#define analyze(c) ((c)->flags & HK_COMPILER_FLAG_ANALYZE)

typedef enum
{
  PRODUCTION_NONE,
  PRODUCTION_ASSIGN,
  PRODUCTION_CALL,
  PRODUCTION_SUBSCRIPT
} Production;

#define match(s, k) ((s)->token.kind == (k))

#define consume(c, k) do \
  { \
    if (!match((c)->scan, k)) \
      syntax_error_unexpected(c); \
    scanner_next_token((c)->scan); \
  } while(0)

#define add_placeholder(c) do \
  { \
    ++(c)->nextIndex; \
  } while (0)

typedef struct
{
  bool    isLocal;
  int     depth;
  uint8_t index;
  int     length;
  char    *start;
  bool    isMutable;
} Variable;

typedef struct Loop
{
  struct Loop *parent;
  int         scopeDepth;
  uint16_t    jump;
  int         numOffsets;
  int         offsets[MAX_BREAKS];
} Loop;

typedef struct Compiler
{
  struct Compiler *parent;
  int             flags;
  Scanner         *scan;
  int             scopeDepth;
  int             numVariables;
  uint8_t         nextIndex;
  Variable        variables[MAX_VARIABLES];
  Loop            *loop;
  HkFunction      *fn;
} Compiler;

static inline void syntax_error(HkString *fnName, const char *file, int line,
  int col, const char *fmt, ...);
static inline void compilation_error(HkString *fnName, const char *file, int line,
  int col, const char *fmt, ...);
static inline void print_semantic_error(HkString *fnName, const char *file, int line,
  int col, const char *fmt, ...);
static inline void print_error_message(HkString *fnName, const char *file, int line,
  int col, const char *fmt, va_list args);
static inline void syntax_error_unexpected(Compiler *comp);
static inline double parse_double(Compiler *comp);
static inline bool string_match(Token *tk, HkString *str);
static inline uint8_t add_number_constant(Compiler *comp, double data);
static inline uint8_t add_string_constant(Compiler *comp, Token *tk);
static inline uint8_t add_constant(Compiler *comp, HkValue val);
static inline void push_scope(Compiler *comp);
static inline void pop_scope(Compiler *comp);
static inline int discard_variables(Compiler *comp, int depth);
static inline bool variable_match(Token *tk, Variable *var);
static inline void add_local(Compiler *comp, Token *tk, bool isMutable);
static inline uint8_t add_nonlocal(Compiler *comp, Token *tk);
static inline void add_variable(Compiler *comp, bool isLocal, uint8_t index, Token *tk,
  bool isMutable);
static inline void define_local(Compiler *comp, Token *tk, bool isMutable);
static inline Variable resolve_variable(Compiler *comp, Token *tk);
static inline Variable *lookup_variable(Compiler *comp, Token *tk);
static inline bool nonlocal_exists(Compiler *comp, Token *tk);
static inline int emit_jump(HkChunk *chunk, HkOpCode op);
static inline void patch_jump(Compiler *comp, int offset);
static inline void patch_opcode(HkChunk *chunk, int offset, HkOpCode op);
static inline void start_loop(Compiler *comp, Loop *loop);
static inline void end_loop(Compiler *comp);
static inline void compiler_init(Compiler *comp, Compiler *parent, int flags,
  Scanner *scan, HkString *fnName);
static void compile_statement(Compiler *comp);
static void compile_import_statement(Compiler *comp);
static void compile_constant_declaration(Compiler *comp);
static void compile_variable_declaration(Compiler *comp);
static void compile_assign_statement(Compiler *comp, Token *tk);
static int compile_assign(Compiler *comp, Production prod, bool inplace);
static void compile_struct_declaration(Compiler *comp, bool isAnonymous);
static void compile_function_declaration(Compiler *comp);
static void compile_anonymous_function(Compiler *comp);
static void compile_anonymous_function_without_params(Compiler *comp);
static void compile_del_statement(Compiler *comp);
static void compile_delete(Compiler *comp, bool inplace);
static void compile_if_statement(Compiler *comp, bool not);
static void compile_match_statement(Compiler *comp);
static void compile_match_statement_member(Compiler *comp);
static void compile_loop_statement(Compiler *comp);
static void compile_while_statement(Compiler *comp, bool not);
static void compile_do_statement(Compiler *comp);
static void compile_for_statement(Compiler *comp);
static void compile_foreach_statement(Compiler *comp);
static void compile_continue_statement(Compiler *comp);
static void compile_break_statement(Compiler *comp);
static void compile_return_statement(Compiler *comp);
static void compile_block(Compiler *comp);
static void compile_expression(Compiler *comp);
static void compile_and_expression(Compiler *comp);
static void compile_equal_expression(Compiler *comp);
static void compile_comp_expression(Compiler *comp);
static void compile_bitwise_or_expression(Compiler *comp);
static void compile_bitwise_xor_expression(Compiler *comp);
static void compile_bitwise_and_expression(Compiler *comp);
static void compile_left_shift_expression(Compiler *comp);
static void compile_right_shift_expression(Compiler *comp);
static void compile_range_expression(Compiler *comp);
static void compile_add_expression(Compiler *comp);
static void compile_mul_expression(Compiler *comp);
static void compile_unary_expression(Compiler *comp);
static void compile_prim_expression(Compiler *comp);
static void compile_array_constructor(Compiler *comp);
static void compile_struct_constructor(Compiler *comp);
static void compile_if_expression(Compiler *comp, bool not);
static void compile_match_expression(Compiler *comp);
static void compile_match_expression_member(Compiler *comp);
static void compile_subscript(Compiler *comp);
static Variable compile_variable(Compiler *comp, Token *tk, bool emit);
static Variable *compile_nonlocal(Compiler *comp, Token *tk);

static inline void syntax_error(HkString *fnName, const char *file, int line,
  int col, const char *fmt, ...)
{
  fprintf(stderr, "syntax error: ");
  va_list args;
  va_start(args, fmt);
  print_error_message(fnName, file, line, col, fmt, args);
  exit(EXIT_FAILURE);
}

static inline void compilation_error(HkString *fnName, const char *file, int line,
  int col, const char *fmt, ...)
{
  fprintf(stderr, "compilation error: ");
  va_list args;
  va_start(args, fmt);
  print_error_message(fnName, file, line, col, fmt, args);
  exit(EXIT_FAILURE);
}

static inline void print_semantic_error(HkString *fnName, const char *file, int line,
  int col, const char *fmt, ...)
{
  fprintf(stderr, "semantic error: ");
  va_list args;
  va_start(args, fmt);
  print_error_message(fnName, file, line, col, fmt, args);
}

static inline void print_error_message(HkString *fnName, const char *file, int line,
  int col, const char *fmt, va_list args)
{
  vfprintf(stderr, fmt, args);
  va_end(args);
  char *chars = fnName ? fnName->chars : "<anonymous>";
  fprintf(stderr, "\n  at %s() in %s:%d,%d\n", chars, file, line, col);
}

static inline void syntax_error_unexpected(Compiler *comp)
{
  Scanner *scan = comp->scan;
  Token *tk = &scan->token;
  HkString *fnName = comp->fn->name;
  char *file = scan->file->chars;
  if (tk->kind == TOKEN_KIND_EOF)
    syntax_error(fnName, file, tk->line, tk->col, "unexpected end of file");
  syntax_error(fnName, file, tk->line, tk->col, "unexpected token `%.*s`",
    tk->length, tk->start);
}

static inline double parse_double(Compiler *comp)
{
  Scanner *scan = comp->scan;
  Token *tk = &scan->token;
  double result;
  if (!hk_double_from_chars(&result, tk->start, false))
    compilation_error(comp->fn->name, scan->file->chars, tk->line, tk->col,
      "floating point number `%.*s` out of range", tk->length, tk->start);
  return result;
}

static inline bool string_match(Token *tk, HkString *str)
{
  return tk->length == str->length
    && !memcmp(tk->start, str->chars, tk->length);
}

static inline uint8_t add_number_constant(Compiler *comp, double data)
{
  HkArray *consts = comp->fn->chunk.consts;
  HkValue *elements = consts->elements;
  for (int i = 0; i < consts->length; ++i)
  {
    HkValue elem = elements[i];
    if (!hk_is_number(elem))
      continue;
    if (data == hk_as_number(elem))
      return (uint8_t) i;
  }
  return add_constant(comp, hk_number_value(data));
}

static inline uint8_t add_string_constant(Compiler *comp, Token *tk)
{
  HkArray *consts = comp->fn->chunk.consts;
  HkValue *elements = consts->elements;
  for (int i = 0; i < consts->length; ++i)
  {
    HkValue elem = elements[i];
    if (!hk_is_string(elem))
      continue;
    if (string_match(tk, hk_as_string(elem)))
      return (uint8_t) i;
  }
  HkString *str = hk_string_from_chars(tk->length, tk->start);
  return add_constant(comp, hk_string_value(str));
}

static inline uint8_t add_constant(Compiler *comp, HkValue val)
{
  HkFunction *fn = comp->fn;
  HkArray *consts = fn->chunk.consts;
  Scanner *scan = comp->scan;
  Token *tk = &scan->token;
  if (consts->length == MAX_CONSTANTS)
    compilation_error(fn->name, scan->file->chars, tk->line, tk->col,
      "a function may only contain %d unique constants", MAX_CONSTANTS);
  uint8_t index = (uint8_t) consts->length;
  hk_array_inplace_add_element(consts, val);
  return index;
}

static inline void push_scope(Compiler *comp)
{
  ++comp->scopeDepth;
}

static inline void pop_scope(Compiler *comp)
{
  comp->numVariables -= discard_variables(comp, comp->scopeDepth);
  --comp->scopeDepth;
  int index = comp->numVariables - 1;
  comp->nextIndex = comp->variables[index].index + 1;
}

static inline int discard_variables(Compiler *comp, int depth)
{
  Variable *variables = comp->variables;
  HkChunk *chunk = &comp->fn->chunk;
  int index = comp->numVariables - 1;
  for (; index > -1 && variables[index].depth >= depth; --index)
    if (variables[index].isLocal)
      hk_chunk_emit_opcode(chunk, HK_OP_POP);
  return comp->numVariables - index - 1;
}

static inline bool variable_match(Token *tk, Variable *var)
{
  return tk->length == var->length
    && !memcmp(tk->start, var->start, tk->length);
}

static inline void add_local(Compiler *comp, Token *tk, bool isMutable)
{
  uint8_t index = comp->nextIndex++;
  add_variable(comp, true, index, tk, isMutable);
}

static inline uint8_t add_nonlocal(Compiler *comp, Token *tk)
{
  uint8_t index = comp->fn->numNonlocals++;
  add_variable(comp, false, index, tk, false);
  return index;
}

static inline void add_variable(Compiler *comp, bool isLocal, uint8_t index, Token *tk,
  bool isMutable)
{
  if (comp->numVariables == MAX_VARIABLES)
    compilation_error(comp->fn->name, comp->scan->file->chars, tk->line, tk->col,
      "a function may only contain %d unique variables", MAX_VARIABLES);
  Variable *var = &comp->variables[comp->numVariables];
  var->isLocal = isLocal;
  var->depth = comp->scopeDepth;
  var->index = index;
  var->length = tk->length;
  var->start = tk->start;
  var->isMutable = isMutable;
  ++comp->numVariables;
}

static inline void define_local(Compiler *comp, Token *tk, bool isMutable)
{
  for (int i = comp->numVariables - 1; i > -1; --i)
  {
    Variable *var = &comp->variables[i];
    if (var->depth < comp->scopeDepth)
      break;
    if (variable_match(tk, var))
    {
      print_semantic_error(comp->fn->name, comp->scan->file->chars, tk->line,tk->col,
        "variable `%.*s` is already defined in this scope", tk->length, tk->start);
      if (!analyze(comp))
        exit(EXIT_FAILURE);
      return;
    }
  }
  add_local(comp, tk, isMutable);
}

static inline Variable resolve_variable(Compiler *comp, Token *tk)
{
  Variable *var = lookup_variable(comp, tk);
  if (var)
    return *var;
  if (!nonlocal_exists(comp->parent, tk) && lookup_global(tk->length, tk->start) == -1)
  {
    print_semantic_error(comp->fn->name, comp->scan->file->chars, tk->line,
      tk->col, "variable `%.*s` is used but not defined", tk->length, tk->start);
    if (!analyze(comp))
      exit(EXIT_FAILURE);
  }
  return (Variable) { .isMutable = false };
}

static inline Variable *lookup_variable(Compiler *comp, Token *tk)
{
  for (int i = comp->numVariables - 1; i > -1; --i)
  {
    Variable *var = &comp->variables[i];
    if (variable_match(tk, var))
      return var;
  }
  return NULL;
}

static inline bool nonlocal_exists(Compiler *comp, Token *tk)
{
  if (!comp)
    return false;
  return lookup_variable(comp, tk) || nonlocal_exists(comp->parent, tk);
}

static inline int emit_jump(HkChunk *chunk, HkOpCode op)
{
  hk_chunk_emit_opcode(chunk, op);
  int offset = chunk->codeLength;
  hk_chunk_emit_word(chunk, 0);
  return offset;
}

static inline void patch_jump(Compiler *comp, int offset)
{
  HkChunk *chunk = &comp->fn->chunk;
  Scanner *scan = comp->scan;
  Token *tk = &scan->token;
  int jump = chunk->codeLength;
  if (jump > UINT16_MAX)
    compilation_error(comp->fn->name, scan->file->chars, tk->line, tk->col,
      "code too large");
  *((uint16_t *) &chunk->code[offset]) = (uint16_t) jump;
}

static inline void patch_opcode(HkChunk *chunk, int offset, HkOpCode op)
{
  chunk->code[offset] = (uint8_t) op;
}

static inline void start_loop(Compiler *comp, Loop *loop)
{
  loop->parent = comp->loop;
  loop->scopeDepth = comp->scopeDepth;
  loop->jump = (uint16_t) comp->fn->chunk.codeLength;
  loop->numOffsets = 0;
  comp->loop = loop;
}

static inline void end_loop(Compiler *comp)
{
  Loop *loop = comp->loop;
  for (int i = 0; i < loop->numOffsets; ++i)
    patch_jump(comp, loop->offsets[i]);
  comp->loop = comp->loop->parent;
}

static inline void compiler_init(Compiler *comp, Compiler *parent, int flags,
  Scanner *scan, HkString *fnName)
{
  comp->parent = parent;
  comp->flags = flags;
  comp->scan = scan;
  comp->scopeDepth = 0;
  comp->numVariables = 0;
  comp->nextIndex = 1;
  comp->loop = NULL;
  comp->fn = hk_function_new(0, fnName, scan->file);
}

static void compile_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  hk_chunk_add_line(&comp->fn->chunk, scan->token.line);
  if (match(scan, TOKEN_KIND_IMPORT_KW))
  {
    compile_import_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_LET_KW))
  {
    compile_constant_declaration(comp);
    consume(comp, TOKEN_KIND_SEMICOLON);
    return;
  }
  if (match(scan, TOKEN_KIND_VAR_KW))
  {
    compile_variable_declaration(comp);
    consume(comp, TOKEN_KIND_SEMICOLON);
    return;
  }
  if (match(scan, TOKEN_KIND_NAME))
  {
    Token tk = scan->token;
    scanner_next_token(scan);
    compile_assign_statement(comp, &tk);
    consume(comp, TOKEN_KIND_SEMICOLON);
    return;
  }
  if (match(scan, TOKEN_KIND_STRUCT_KW))
  {
    compile_struct_declaration(comp, false);
    return;
  }
  if (match(scan, TOKEN_KIND_FN_KW))
  {
    compile_function_declaration(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_DEL_KW))
  {
    compile_del_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_IF_KW))
  {
    compile_if_statement(comp, false);
    return;
  }
  if (match(scan, TOKEN_KIND_IFBANG_KW))
  {
    compile_if_statement(comp, true);
    return;
  }
  if (match(scan, TOKEN_KIND_MATCH_KW))
  {
    compile_match_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_LOOP_KW))
  {
    compile_loop_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_WHILE_KW))
  {
    compile_while_statement(comp, false);
    return;
  }
  if (match(scan, TOKEN_KIND_WHILEBANG_KW))
  {
    compile_while_statement(comp, true);
    return;
  }
  if (match(scan, TOKEN_KIND_DO_KW))
  {
    compile_do_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_FOR_KW))
  {
    compile_for_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_FOREACH_KW))
  {
    compile_foreach_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_CONTINUE_KW))
  {
    compile_continue_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_BREAK_KW))
  {
    compile_break_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_RETURN_KW))
  {
    compile_return_statement(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_LBRACE))
  {
    compile_block(comp);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_import_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  if (match(scan, TOKEN_KIND_NAME))
  {
    Token tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    if (match(scan, TOKEN_KIND_AS_KW))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_KIND_NAME))
        syntax_error_unexpected(comp);
      tk = scan->token;
      scanner_next_token(scan);
    }
    define_local(comp, &tk, false);
    consume(comp, TOKEN_KIND_SEMICOLON);
    hk_chunk_emit_opcode(chunk, HK_OP_LOAD_MODULE);
    return;
  }
  if (match(scan, TOKEN_KIND_STRING))
  {
    Token tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    consume(comp, TOKEN_KIND_AS_KW);
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    consume(comp, TOKEN_KIND_SEMICOLON);
    hk_chunk_emit_opcode(chunk, HK_OP_LOAD_MODULE);
    return;
  }
  if (match(scan, TOKEN_KIND_LBRACE))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    Token tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    uint8_t n = 1;
    while (match(scan, TOKEN_KIND_COMMA))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_KIND_NAME))
        syntax_error_unexpected(comp);
      tk = scan->token;
      scanner_next_token(scan);
      define_local(comp, &tk, false);
      index = add_string_constant(comp, &tk);
      hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
      hk_chunk_emit_byte(chunk, index);
      ++n;
    }
    consume(comp, TOKEN_KIND_RBRACE);
    consume(comp, TOKEN_KIND_FROM_KW);
    if (!match(scan, TOKEN_KIND_NAME) && !match(scan, TOKEN_KIND_STRING))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    consume(comp, TOKEN_KIND_SEMICOLON);
    index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    hk_chunk_emit_opcode(chunk, HK_OP_LOAD_MODULE);
    hk_chunk_emit_opcode(chunk, HK_OP_UNPACK_STRUCT);
    hk_chunk_emit_byte(chunk, n);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_constant_declaration(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  if (match(scan, TOKEN_KIND_NAME))
  {
    Token tk = scan->token;
    scanner_next_token(scan);
    consume(comp, TOKEN_KIND_EQ);
    compile_expression(comp);
    define_local(comp, &tk, false);
    return;
  }
  if (match(scan, TOKEN_KIND_LBRACKET))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_UNDERSCORE_KW))
      add_placeholder(comp);
    else
    {
      if (!match(scan, TOKEN_KIND_NAME))
        syntax_error_unexpected(comp);
      define_local(comp, &scan->token, false);
    }
    scanner_next_token(scan);
    uint8_t n = 1;
    while (match(scan, TOKEN_KIND_COMMA))
    {
      scanner_next_token(scan);
      if (match(scan, TOKEN_KIND_UNDERSCORE_KW))
        add_placeholder(comp);
      else
      {
        if (!match(scan, TOKEN_KIND_NAME))
          syntax_error_unexpected(comp);
        // FIXME: This is a bug, we should not define the local here
        define_local(comp, &scan->token, false);
      }
      scanner_next_token(scan);
      ++n;
    }
    consume(comp, TOKEN_KIND_RBRACKET);
    consume(comp, TOKEN_KIND_EQ);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_UNPACK_ARRAY);
    hk_chunk_emit_byte(chunk, n);
    return;
  }
  if (match(scan, TOKEN_KIND_LBRACE))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    Token tk = scan->token;
    scanner_next_token(scan);
    // FIXME: This is a bug, we should not define the local here
    define_local(comp, &tk, false);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    uint8_t n = 1;
    while (match(scan, TOKEN_KIND_COMMA))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_KIND_NAME))
        syntax_error_unexpected(comp);
      tk = scan->token;
      scanner_next_token(scan);
      // FIXME: This is a bug, we should not define the local here
      define_local(comp, &tk, false);
      index = add_string_constant(comp, &tk);
      hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
      hk_chunk_emit_byte(chunk, index);
      ++n;
    }
    consume(comp, TOKEN_KIND_RBRACE);
    consume(comp, TOKEN_KIND_EQ);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_UNPACK_STRUCT);
    hk_chunk_emit_byte(chunk, n);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_variable_declaration(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  if (match(scan, TOKEN_KIND_NAME))
  {
    Token tk = scan->token;
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      define_local(comp, &tk, true);
      return;  
    }
    hk_chunk_emit_opcode(chunk, HK_OP_NIL);
    define_local(comp, &tk, true);
    return;
  }
  if (match(scan, TOKEN_KIND_LBRACKET))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_UNDERSCORE_KW))
      add_placeholder(comp);
    else
    {
      if (!match(scan, TOKEN_KIND_NAME))
        syntax_error_unexpected(comp);
      // FIXME: This is a bug, we should not define the local here
      define_local(comp, &scan->token, false);
    }
    scanner_next_token(scan);
    uint8_t n = 1;
    while (match(scan, TOKEN_KIND_COMMA))
    {
      scanner_next_token(scan);
      if (match(scan, TOKEN_KIND_UNDERSCORE_KW))
        add_placeholder(comp);
      else
      {
        if (!match(scan, TOKEN_KIND_NAME))
          syntax_error_unexpected(comp);
        // FIXME: This is a bug, we should not define the local here
        define_local(comp, &scan->token, false);
      }
      scanner_next_token(scan);
      ++n;
    }
    consume(comp, TOKEN_KIND_RBRACKET);
    consume(comp, TOKEN_KIND_EQ);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_UNPACK_ARRAY);
    hk_chunk_emit_byte(chunk, n);
    return;
  }
  if (match(scan, TOKEN_KIND_LBRACE))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    Token tk = scan->token;
    scanner_next_token(scan);
    // FIXME: This is a bug, we should not define the local here
    define_local(comp, &tk, true);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    uint8_t n = 1;
    while (match(scan, TOKEN_KIND_COMMA))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_KIND_NAME))
        syntax_error_unexpected(comp);
      tk = scan->token;
      scanner_next_token(scan);
      // FIXME: This is a bug, we should not define the local here
      define_local(comp, &tk, true);
      index = add_string_constant(comp, &tk);
      hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
      hk_chunk_emit_byte(chunk, index);
      ++n;
    }
    consume(comp, TOKEN_KIND_RBRACE);
    consume(comp, TOKEN_KIND_EQ);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_UNPACK_STRUCT);
    hk_chunk_emit_byte(chunk, n);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_assign_statement(Compiler *comp, Token *tk)
{
  Scanner *scan = comp->scan;
  HkFunction *fn = comp->fn;
  HkChunk *chunk = &fn->chunk;
  Variable var;
  if (match(scan, TOKEN_KIND_EQ))
  {
    var = compile_variable(comp, tk, false);
    scanner_next_token(scan);
    compile_expression(comp);
    goto end;
  }
  var = compile_variable(comp, tk, true);
  if (compile_assign(comp, PRODUCTION_NONE, true) == PRODUCTION_CALL)
  {
    hk_chunk_emit_opcode(chunk, HK_OP_POP);
    return;
  }
end:
  if (!var.isMutable)
  {
    print_semantic_error(fn->name, scan->file->chars, tk->line, tk->col,
      "cannot assign to immutable variable `%.*s`", tk->length, tk->start);
    if (!analyze(comp))
      exit(EXIT_FAILURE);
  }
  hk_chunk_emit_opcode(chunk, HK_OP_SET_LOCAL);
  hk_chunk_emit_byte(chunk, var.index);
}

static int compile_assign(Compiler *comp, Production prod, bool inplace)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  if (match(scan, TOKEN_KIND_PIPEEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_BITWISE_OR);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_CARETEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_BITWISE_XOR);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_AMPEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_BITWISE_AND);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_LTLTEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_LEFT_SHIFT);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_GTGTEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_RIGHT_SHIFT);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_PLUSEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_ADD);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_DASHEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_SUBTRACT);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_STAREQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_MULTIPLY);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_SLASHEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_DIVIDE);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_TILDESLASHEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_QUOTIENT);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_PERCENTEQ))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_REMAINDER);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_PLUSPLUS))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_INCREMENT);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_DASHDASH))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_DECREMENT);
    return PRODUCTION_ASSIGN;
  }
  if (match(scan, TOKEN_KIND_LBRACKET))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_RBRACKET))
    {
      scanner_next_token(scan);
      consume(comp, TOKEN_KIND_EQ);
      compile_expression(comp);
      hk_chunk_emit_opcode(chunk, inplace ? HK_OP_INPLACE_ADD_ELEMENT : HK_OP_ADD_ELEMENT);
      return PRODUCTION_ASSIGN;
    }
    compile_expression(comp);
    consume(comp, TOKEN_KIND_RBRACKET);
    if (match(scan, TOKEN_KIND_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      hk_chunk_emit_opcode(chunk, inplace ? HK_OP_INPLACE_PUT_ELEMENT : HK_OP_PUT_ELEMENT);
      return PRODUCTION_ASSIGN;
    }
    int offset = chunk->codeLength;
    hk_chunk_emit_opcode(chunk, HK_OP_GET_ELEMENT);
    Production _prod = compile_assign(comp, PRODUCTION_SUBSCRIPT, false);
    if (_prod == PRODUCTION_ASSIGN)
    {
      patch_opcode(chunk, offset, HK_OP_FETCH_ELEMENT);
      hk_chunk_emit_opcode(chunk, HK_OP_SET_ELEMENT);
    }
    return _prod;
  }
  if (match(scan, TOKEN_KIND_DOT))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    Token tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    if (match(scan, TOKEN_KIND_EQ))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      hk_chunk_emit_opcode(chunk, inplace ? HK_OP_INPLACE_PUT_FIELD : HK_OP_PUT_FIELD);
      hk_chunk_emit_byte(chunk, index);
      return PRODUCTION_ASSIGN;
    }
    int offset = chunk->codeLength;
    hk_chunk_emit_opcode(chunk, HK_OP_GET_FIELD);
    hk_chunk_emit_byte(chunk, index);
    Production _prod = compile_assign(comp, PRODUCTION_SUBSCRIPT, false);
    if (_prod == PRODUCTION_ASSIGN)
    {
      patch_opcode(chunk, offset, HK_OP_FETCH_FIELD);
      hk_chunk_emit_opcode(chunk, HK_OP_SET_FIELD);
    }
    return _prod;
  }
  if (match(scan, TOKEN_KIND_LPAREN))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_RPAREN))
    {
      scanner_next_token(scan);
      hk_chunk_emit_opcode(chunk, HK_OP_CALL);
      hk_chunk_emit_byte(chunk, 0);
      return compile_assign(comp, PRODUCTION_CALL, false);
    }
    compile_expression(comp);
    uint8_t numArgs = 1;
    while (match(scan, TOKEN_KIND_COMMA))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      ++numArgs;
    }
    consume(comp, TOKEN_KIND_RPAREN);
    hk_chunk_emit_opcode(chunk, HK_OP_CALL);
    hk_chunk_emit_byte(chunk, numArgs);
    return compile_assign(comp, PRODUCTION_CALL, false);
  }
  if (prod == PRODUCTION_NONE || prod == PRODUCTION_SUBSCRIPT)
    syntax_error_unexpected(comp);
  return prod;
}

static void compile_struct_declaration(Compiler *comp, bool isAnonymous)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  Token tk;
  uint8_t index;
  if (isAnonymous)
    hk_chunk_emit_opcode(chunk, HK_OP_NIL);
  else
  {
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    define_local(comp, &tk, false);
    index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
  }
  consume(comp, TOKEN_KIND_LBRACE);
  if (match(scan, TOKEN_KIND_RBRACE))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_STRUCT);
    hk_chunk_emit_byte(chunk, 0);
    return;
  }
  if (!match(scan, TOKEN_KIND_STRING) && !match(scan, TOKEN_KIND_NAME))
    syntax_error_unexpected(comp);
  tk = scan->token;
  scanner_next_token(scan);
  index = add_string_constant(comp, &tk);
  hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
  hk_chunk_emit_byte(chunk, index);
  uint8_t length = 1;
  while (match(scan, TOKEN_KIND_COMMA))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_STRING) && !match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    ++length;
  }
  consume(comp, TOKEN_KIND_RBRACE);
  hk_chunk_emit_opcode(chunk, HK_OP_STRUCT);
  hk_chunk_emit_byte(chunk, length);
}

static void compile_function_declaration(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkFunction *fn = comp->fn;
  HkChunk *chunk = &fn->chunk;
  scanner_next_token(scan);
  Compiler childComp;
  if (!match(scan, TOKEN_KIND_NAME))
    syntax_error_unexpected(comp);
  Token tk = scan->token;
  scanner_next_token(scan);
  define_local(comp, &tk, false);
  HkString *fnName = hk_string_from_chars(tk.length, tk.start);
  compiler_init(&childComp, comp, comp->flags, scan, fnName);
  add_variable(&childComp, true, 0, &tk, false);
  HkChunk *childChunk = &childComp.fn->chunk;
  consume(comp, TOKEN_KIND_LPAREN);
  if (match(scan, TOKEN_KIND_RPAREN))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_ARROW))
    {
      scanner_next_token(scan);
      compile_expression(&childComp);
      consume(comp, TOKEN_KIND_SEMICOLON);
      hk_chunk_emit_opcode(childChunk, HK_OP_RETURN);
      goto end;
    }
    if (!match(scan, TOKEN_KIND_LBRACE))
      syntax_error_unexpected(comp);
    compile_block(&childComp);
    hk_chunk_emit_opcode(childChunk, HK_OP_RETURN_NIL);
    goto end;
  }
  if (!match(scan, TOKEN_KIND_NAME))
    syntax_error_unexpected(comp);
  define_local(&childComp, &scan->token, true);
  scanner_next_token(scan);
  int arity = 1;
  while (match(scan, TOKEN_KIND_COMMA))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    define_local(&childComp, &scan->token, true);
    scanner_next_token(scan);
    ++arity;
  }
  childComp.fn->arity = arity;
  consume(comp, TOKEN_KIND_RPAREN);
  if (match(scan, TOKEN_KIND_ARROW))
  {
    scanner_next_token(scan);
    compile_expression(&childComp);
    consume(comp, TOKEN_KIND_SEMICOLON);
    hk_chunk_emit_opcode(childChunk, HK_OP_RETURN);
    goto end;
  }
  if (!match(scan, TOKEN_KIND_LBRACE))
    syntax_error_unexpected(comp);
  compile_block(&childComp);
  hk_chunk_emit_opcode(childChunk, HK_OP_RETURN_NIL);
  uint8_t index;
end:
  index = fn->functionsLength;
  hk_function_add_child(fn, childComp.fn);
  hk_chunk_emit_opcode(chunk, HK_OP_CLOSURE);
  hk_chunk_emit_byte(chunk, index);
}

static void compile_anonymous_function(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkFunction *fn = comp->fn;
  HkChunk *chunk = &fn->chunk;
  scanner_next_token(scan);
  Compiler childComp;
  compiler_init(&childComp, comp, comp->flags, scan, NULL);
  HkChunk *childChunk = &childComp.fn->chunk;
  if (match(scan, TOKEN_KIND_PIPE))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_ARROW))
    {
      scanner_next_token(scan);
      compile_expression(&childComp);
      hk_chunk_emit_opcode(childChunk, HK_OP_RETURN);
      goto end;
    }
    if (!match(scan, TOKEN_KIND_LBRACE))
      syntax_error_unexpected(comp);
    compile_block(&childComp);
    hk_chunk_emit_opcode(childChunk, HK_OP_RETURN_NIL);
    goto end;
  }
  if (!match(scan, TOKEN_KIND_NAME))
    syntax_error_unexpected(comp);
  define_local(&childComp, &scan->token, true);
  scanner_next_token(scan);
  int arity = 1;
  while (match(scan, TOKEN_KIND_COMMA))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    define_local(&childComp, &scan->token, true);
    scanner_next_token(scan);
    ++arity;
  }
  childComp.fn->arity = arity;
  consume(comp, TOKEN_KIND_PIPE);
  if (match(scan, TOKEN_KIND_ARROW))
  {
    scanner_next_token(scan);
    compile_expression(&childComp);
    hk_chunk_emit_opcode(childChunk, HK_OP_RETURN);
    goto end;
  }
  if (!match(scan, TOKEN_KIND_LBRACE))
    syntax_error_unexpected(comp);
  compile_block(&childComp);
  hk_chunk_emit_opcode(childChunk, HK_OP_RETURN_NIL);
  uint8_t index;
end:
  index = fn->functionsLength;
  hk_function_add_child(fn, childComp.fn);
  hk_chunk_emit_opcode(chunk, HK_OP_CLOSURE);
  hk_chunk_emit_byte(chunk, index);
}

static void compile_anonymous_function_without_params(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkFunction *fn = comp->fn;
  HkChunk *chunk = &fn->chunk;
  scanner_next_token(scan);
  Compiler childComp;
  compiler_init(&childComp, comp, comp->flags, scan, NULL);
  HkChunk *childChunk = &childComp.fn->chunk;
  if (match(scan, TOKEN_KIND_ARROW))
  {
    scanner_next_token(scan);
    compile_expression(&childComp);
    hk_chunk_emit_opcode(childChunk, HK_OP_RETURN);
    goto end;
  }
  if (!match(scan, TOKEN_KIND_LBRACE))
    syntax_error_unexpected(comp);
  compile_block(&childComp);
  hk_chunk_emit_opcode(childChunk, HK_OP_RETURN_NIL);
  uint8_t index;
end:
  index = fn->functionsLength;
  hk_function_add_child(fn, childComp.fn);
  hk_chunk_emit_opcode(chunk, HK_OP_CLOSURE);
  hk_chunk_emit_byte(chunk, index);
}

static void compile_del_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkFunction *fn = comp->fn;
  HkChunk *chunk = &fn->chunk;
  scanner_next_token(scan);
  if (!match(scan, TOKEN_KIND_NAME))
    syntax_error_unexpected(comp);
  Token tk = scan->token;
  scanner_next_token(scan);
  Variable var = resolve_variable(comp, &tk);
  if (!var.isMutable)
  {
    print_semantic_error(fn->name, scan->file->chars, tk.line, tk.col,
      "cannot delete element from immutable variable `%.*s`", tk.length, tk.start);
    if (!analyze(comp))
      exit(EXIT_FAILURE);
  }
  hk_chunk_emit_opcode(chunk, HK_OP_GET_LOCAL);
  hk_chunk_emit_byte(chunk, var.index);
  compile_delete(comp, true);
  hk_chunk_emit_opcode(chunk, HK_OP_SET_LOCAL);
  hk_chunk_emit_byte(chunk, var.index);
}

static void compile_delete(Compiler *comp, bool inplace)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  if (match(scan, TOKEN_KIND_LBRACKET))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    consume(comp, TOKEN_KIND_RBRACKET);
    if (match(scan, TOKEN_KIND_SEMICOLON))
    {
      scanner_next_token(scan);
      hk_chunk_emit_opcode(chunk, inplace ? HK_OP_INPLACE_DELETE_ELEMENT : HK_OP_DELETE_ELEMENT);
      return;
    }
    hk_chunk_emit_opcode(chunk, HK_OP_FETCH_ELEMENT);
    compile_delete(comp, false);
    hk_chunk_emit_opcode(chunk, HK_OP_SET_ELEMENT);
    return;
  }
  if (match(scan, TOKEN_KIND_DOT))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    Token tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_FETCH_FIELD);
    hk_chunk_emit_byte(chunk, index);
    compile_delete(comp, false);
    hk_chunk_emit_opcode(chunk, HK_OP_SET_FIELD);
    return;
  }
  syntax_error_unexpected(comp); 
}

static void compile_if_statement(Compiler *comp, bool not)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_KIND_LPAREN);
  push_scope(comp);
  if (match(scan, TOKEN_KIND_LET_KW))
  {
    compile_constant_declaration(comp);
    consume(comp, TOKEN_KIND_SEMICOLON);
  }
  else if (match(scan, TOKEN_KIND_VAR_KW))
  {
    compile_variable_declaration(comp);
    consume(comp, TOKEN_KIND_SEMICOLON);
  }
  compile_expression(comp);
  consume(comp, TOKEN_KIND_RPAREN);
  HkOpCode op = not ? HK_OP_JUMP_IF_TRUE : HK_OP_JUMP_IF_FALSE;
  int offset1 = emit_jump(chunk, op);
  compile_statement(comp);
  int offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  if (match(scan, TOKEN_KIND_ELSE_KW))
  {
    scanner_next_token(scan);
    compile_statement(comp);
  }
  patch_jump(comp, offset2);
  pop_scope(comp);
}

static void compile_match_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_KIND_LPAREN);
  push_scope(comp);
  if (match(scan, TOKEN_KIND_LET_KW))
  {
    compile_constant_declaration(comp);
    consume(comp, TOKEN_KIND_SEMICOLON);
  }
  else if (match(scan, TOKEN_KIND_VAR_KW))
  {
    compile_variable_declaration(comp);
    consume(comp, TOKEN_KIND_SEMICOLON);
  }
  compile_expression(comp);
  consume(comp, TOKEN_KIND_RPAREN);
  consume(comp, TOKEN_KIND_LBRACE);
  compile_expression(comp);
  consume(comp, TOKEN_KIND_ARROW);
  int offset1 = emit_jump(chunk, HK_OP_JUMP_IF_NOT_EQUAL);
  compile_statement(comp);
  int offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  compile_match_statement_member(comp);
  patch_jump(comp, offset2);
  pop_scope(comp);
}

static void compile_match_statement_member(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  if (match(scan, TOKEN_KIND_RBRACE))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_POP);
    return;
  }
  if (match(scan, TOKEN_KIND_UNDERSCORE_KW))
  {
    scanner_next_token(scan);
    consume(comp, TOKEN_KIND_ARROW);
    hk_chunk_emit_opcode(chunk, HK_OP_POP);
    compile_statement(comp);
    consume(comp, TOKEN_KIND_RBRACE);
    return;
  }
  compile_expression(comp);
  consume(comp, TOKEN_KIND_ARROW);
  int offset1 = emit_jump(chunk, HK_OP_JUMP_IF_NOT_EQUAL);
  compile_statement(comp);
  int offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  compile_match_statement_member(comp);
  patch_jump(comp, offset2);
}

static void compile_loop_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  if (!match(scan, TOKEN_KIND_LBRACE))
    syntax_error_unexpected(comp);
  Loop loop;
  start_loop(comp, &loop);
  compile_statement(comp);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, loop.jump);
  end_loop(comp);
}

static void compile_while_statement(Compiler *comp, bool not)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_KIND_LPAREN);
  Loop loop;
  start_loop(comp, &loop);
  compile_expression(comp);
  consume(comp, TOKEN_KIND_RPAREN);
  HkOpCode op = not ? HK_OP_JUMP_IF_TRUE : HK_OP_JUMP_IF_FALSE;
  int offset = emit_jump(chunk, op);
  compile_statement(comp);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, loop.jump);
  patch_jump(comp, offset);
  end_loop(comp);
}

static void compile_do_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  Loop loop;
  start_loop(comp, &loop);
  compile_statement(comp);  
  HkOpCode op = HK_OP_JUMP_IF_FALSE;
  if (match(scan, TOKEN_KIND_WHILEBANG_KW))
  {
    scanner_next_token(scan);
    op = HK_OP_JUMP_IF_TRUE;
  }
  else
    consume(comp, TOKEN_KIND_WHILE_KW);
  consume(comp, TOKEN_KIND_LPAREN);
  compile_expression(comp);
  consume(comp, TOKEN_KIND_RPAREN);
  consume(comp, TOKEN_KIND_SEMICOLON);
  int offset = emit_jump(chunk, op);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, loop.jump);
  patch_jump(comp, offset);
  end_loop(comp);
}

static void compile_for_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_KIND_LPAREN);
  push_scope(comp);
  if (match(scan, TOKEN_KIND_SEMICOLON))
    scanner_next_token(scan);
  else
  {
    if (match(scan, TOKEN_KIND_LET_KW))
    {
      compile_constant_declaration(comp);
      consume(comp, TOKEN_KIND_SEMICOLON);
    }
    else if (match(scan, TOKEN_KIND_VAR_KW))
    {
      compile_variable_declaration(comp);
      consume(comp, TOKEN_KIND_SEMICOLON);
    }
    else if (match(scan, TOKEN_KIND_NAME))
    {
      Token tk = scan->token;
      scanner_next_token(scan);
      compile_assign_statement(comp, &tk);
      consume(comp, TOKEN_KIND_SEMICOLON);
    }
    else
      syntax_error_unexpected(comp);
  }
  uint16_t jump1 = (uint16_t) chunk->codeLength;
  bool missing = match(scan, TOKEN_KIND_SEMICOLON);
  int offset1 = 0;
  if (missing)
    scanner_next_token(scan);
  else
  {
    compile_expression(comp);
    consume(comp, TOKEN_KIND_SEMICOLON);
    offset1 = emit_jump(chunk, HK_OP_JUMP_IF_FALSE);
  }
  int offset2 = emit_jump(chunk, HK_OP_JUMP);
  uint16_t jump2 = (uint16_t) chunk->codeLength;
  Loop loop;
  start_loop(comp, &loop);
  if (match(scan, TOKEN_KIND_RPAREN))
    scanner_next_token(scan);
  else
  {
    if (!match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    Token tk = scan->token;
    scanner_next_token(scan);
    compile_assign_statement(comp, &tk);
    consume(comp, TOKEN_KIND_RPAREN);
  }
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, jump1);
  patch_jump(comp, offset2);
  compile_statement(comp);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, jump2);
  if (!missing)
  {
    hk_assert(offset1, "offset1 is zero");
    patch_jump(comp, offset1);
  }
  end_loop(comp);
  pop_scope(comp);
}

static void compile_foreach_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_KIND_LPAREN);
  push_scope(comp);
  if (!match(scan, TOKEN_KIND_NAME))
    syntax_error_unexpected(comp);
  Token tk = scan->token;
  scanner_next_token(scan);
  add_local(comp, &tk, false);
  hk_chunk_emit_opcode(chunk, HK_OP_NIL);
  consume(comp, TOKEN_KIND_IN_KW);
  compile_expression(comp);
  consume(comp, TOKEN_KIND_RPAREN);
  hk_chunk_emit_opcode(chunk, HK_OP_ITERATOR);
  int offset1 = emit_jump(chunk, HK_OP_JUMP);
  Loop loop;
  start_loop(comp, &loop);
  hk_chunk_emit_opcode(chunk, HK_OP_NEXT);
  patch_jump(comp, offset1);
  int offset2 = emit_jump(chunk, HK_OP_JUMP_IF_NOT_VALID);
  hk_chunk_emit_opcode(chunk, HK_OP_CURRENT);
  compile_statement(comp);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, loop.jump);
  patch_jump(comp, offset2);
  hk_chunk_emit_opcode(chunk, HK_OP_POP);
  end_loop(comp);
  pop_scope(comp);
}

static void compile_continue_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkFunction *fn = comp->fn;
  HkChunk *chunk = &fn->chunk;
  Token tk = scan->token;
  scanner_next_token(scan);
  if (!comp->loop)
  {
    print_semantic_error(fn->name, scan->file->chars, tk.line, tk.col,
      "cannot use continue outside of a loop");
    if (!analyze(comp))
      exit(EXIT_FAILURE);
  }
  consume(comp, TOKEN_KIND_SEMICOLON);
  if (analyze(comp))
    return; 
  (void) discard_variables(comp, comp->loop->scopeDepth + 1);
  hk_chunk_emit_opcode(chunk, HK_OP_JUMP);
  hk_chunk_emit_word(chunk, comp->loop->jump);
}

static void compile_break_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkString *fnName = comp->fn->name;
  char *file = scan->file->chars;
  Token tk = scan->token;
  scanner_next_token(scan);
  if (!comp->loop)
  {
    print_semantic_error(fnName, file, tk.line, tk.col,
      "cannot use break outside of a loop");
    if (!analyze(comp))
      exit(EXIT_FAILURE);
  }
  consume(comp, TOKEN_KIND_SEMICOLON);
  if (analyze(comp))
    return;
  (void) discard_variables(comp, comp->loop->scopeDepth + 1);
  Loop *loop = comp->loop;
  if (loop->numOffsets == MAX_BREAKS)
    compilation_error(fnName, file, tk.line, tk.col,
      "cannot use more than %d breaks", MAX_BREAKS);
  int offset = emit_jump(&comp->fn->chunk, HK_OP_JUMP);
  loop->offsets[loop->numOffsets++] = offset;
}

static void compile_return_statement(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  if (match(scan, TOKEN_KIND_SEMICOLON))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_RETURN_NIL);
    return;
  }
  compile_expression(comp);
  consume(comp, TOKEN_KIND_SEMICOLON);
  hk_chunk_emit_opcode(chunk, HK_OP_RETURN);
}

static void compile_block(Compiler *comp)
{
  Scanner *scan = comp->scan;
  scanner_next_token(scan);
  push_scope(comp);
  while (!match(scan, TOKEN_KIND_RBRACE))
    compile_statement(comp);
  scanner_next_token(scan);
  pop_scope(comp);
}

static void compile_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  compile_and_expression(comp);
  while (match(scan, TOKEN_KIND_PIPEPIPE))
  {
    scanner_next_token(scan);
    int offset = emit_jump(&comp->fn->chunk, HK_OP_JUMP_IF_TRUE_OR_POP);
    compile_and_expression(comp);
    patch_jump(comp, offset);
  }
}

static void compile_and_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  compile_equal_expression(comp);
  while (match(scan, TOKEN_KIND_AMPAMP))
  {
    scanner_next_token(scan);
    int offset = emit_jump(&comp->fn->chunk, HK_OP_JUMP_IF_FALSE_OR_POP);
    compile_equal_expression(comp);
    patch_jump(comp, offset);
  }
}

static void compile_equal_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_comp_expression(comp);
  for (;;)
  {
    if (match(scan, TOKEN_KIND_EQEQ))
    {
      scanner_next_token(scan);
      compile_comp_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_EQUAL);
      continue;
    }
    if (match(scan, TOKEN_KIND_BANGEQ))
    {
      scanner_next_token(scan);
      compile_comp_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_NOT_EQUAL);
      continue;
    }
    break;
  }
}

static void compile_comp_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_bitwise_or_expression(comp);
  for (;;)
  {
    if (match(scan, TOKEN_KIND_GT))
    {
      scanner_next_token(scan);
      compile_bitwise_or_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_GREATER);
      continue;
    }
    if (match(scan, TOKEN_KIND_GTEQ))
    {
      scanner_next_token(scan);
      compile_bitwise_or_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_NOT_LESS);
      continue;
    }
    if (match(scan, TOKEN_KIND_LT))
    {
      scanner_next_token(scan);
      compile_bitwise_or_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_LESS);
      continue;
    }
    if (match(scan, TOKEN_KIND_LTEQ))
    {
      scanner_next_token(scan);
      compile_bitwise_or_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_NOT_GREATER);
      continue;
    }
    break;
  }
}

static void compile_bitwise_or_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_bitwise_xor_expression(comp);
  while (match(scan, TOKEN_KIND_PIPE))
  {
    scanner_next_token(scan);
    compile_bitwise_xor_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_BITWISE_OR);
  }
}

static void compile_bitwise_xor_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_bitwise_and_expression(comp);
  while (match(scan, TOKEN_KIND_CARET))
  {
    scanner_next_token(scan);
    compile_bitwise_and_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_BITWISE_XOR);
  }
}

static void compile_bitwise_and_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_left_shift_expression(comp);
  while (match(scan, TOKEN_KIND_AMP))
  {
    scanner_next_token(scan);
    compile_left_shift_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_BITWISE_AND);
  }
}

static void compile_left_shift_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_right_shift_expression(comp);
  while (match(scan, TOKEN_KIND_LTLT))
  {
    scanner_next_token(scan);
    compile_right_shift_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_LEFT_SHIFT);
  }
}

static void compile_right_shift_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_range_expression(comp);
  while (match(scan, TOKEN_KIND_GTGT))
  {
    scanner_next_token(scan);
    compile_range_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_RIGHT_SHIFT);
  }
}

static void compile_range_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_add_expression(comp);
  if (match(scan, TOKEN_KIND_DOTDOT))
  {
    scanner_next_token(scan);
    compile_add_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_RANGE);
  }
}

static void compile_add_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_mul_expression(comp);
  for (;;)
  {
    if (match(scan, TOKEN_KIND_PLUS))
    {
      scanner_next_token(scan);
      compile_mul_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_ADD);
      continue;
    }
    if (match(scan, TOKEN_KIND_DASH))
    {
      scanner_next_token(scan);
      compile_mul_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_SUBTRACT);
      continue;
    }
    break;
  }
}

static void compile_mul_expression(Compiler *comp)
{
  compile_unary_expression(comp);
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  for (;;)
  {
    if (match(scan, TOKEN_KIND_STAR))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_MULTIPLY);
      continue;
    }
    if (match(scan, TOKEN_KIND_SLASH))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_DIVIDE);
      continue;
    }
    if (match(scan, TOKEN_KIND_TILDESLASH))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_QUOTIENT);
      continue;
    }
    if (match(scan, TOKEN_KIND_PERCENT))
    {
      scanner_next_token(scan);
      compile_unary_expression(comp);
      hk_chunk_emit_opcode(chunk, HK_OP_REMAINDER);
      continue;
    }
    break;
  }
}

static void compile_unary_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  if (match(scan, TOKEN_KIND_DASH))
  {
    scanner_next_token(scan);
    compile_unary_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_NEGATE);
    return;
  }
  if (match(scan, TOKEN_KIND_BANG))
  {
    scanner_next_token(scan);
    compile_unary_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_NOT);
    return;
  }
  if (match(scan, TOKEN_KIND_TILDE))
  {
    scanner_next_token(scan);
    compile_unary_expression(comp);
    hk_chunk_emit_opcode(chunk, HK_OP_BITWISE_NOT);
    return;
  }
  compile_prim_expression(comp);
}

static void compile_prim_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  if (match(scan, TOKEN_KIND_NIL_KW))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_NIL);
    return;
  }
  if (match(scan, TOKEN_KIND_FALSE_KW))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_FALSE);
    return;
  }
  if (match(scan, TOKEN_KIND_TRUE_KW))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_TRUE);
    return;
  }
  if (match(scan, TOKEN_KIND_INT))
  {
    double data = parse_double(comp);
    scanner_next_token(scan);
    if (data <= UINT16_MAX)
    {
      hk_chunk_emit_opcode(chunk, HK_OP_INT);
      hk_chunk_emit_word(chunk, (uint16_t) data);
      return;
    }
    uint8_t index = add_number_constant(comp, data);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    return;
  }
  if (match(scan, TOKEN_KIND_FLOAT))
  {
    double data = parse_double(comp);
    scanner_next_token(scan);
    uint8_t index = add_number_constant(comp, data);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    return;
  }
  if (match(scan, TOKEN_KIND_STRING))
  {
    Token tk = scan->token;
    scanner_next_token(scan);
    uint8_t index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    return;
  }
  if (match(scan, TOKEN_KIND_LBRACKET))
  {
    compile_array_constructor(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_LBRACE))
  {
    compile_struct_constructor(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_STRUCT_KW))
  {
    compile_struct_declaration(comp, true);
    return;
  }
  if (match(scan, TOKEN_KIND_PIPE))
  {
    compile_anonymous_function(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_PIPEPIPE))
  {
    compile_anonymous_function_without_params(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_IF_KW))
  {
    compile_if_expression(comp, false);
    return;
  }
  if (match(scan, TOKEN_KIND_IFBANG_KW))
  {
    compile_if_expression(comp, true);
    return;
  }
  if (match(scan, TOKEN_KIND_MATCH_KW))
  {
    compile_match_expression(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_NAME))
  {
    compile_subscript(comp);
    return;
  }
  if (match(scan, TOKEN_KIND_LPAREN))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    consume(comp, TOKEN_KIND_RPAREN);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_array_constructor(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  uint8_t length = 0;
  if (match(scan, TOKEN_KIND_RBRACKET))
  {
    scanner_next_token(scan);
    goto end;
  }
  compile_expression(comp);
  ++length;
  while (match(scan, TOKEN_KIND_COMMA))
  {
    scanner_next_token(scan);
    compile_expression(comp);
    ++length;
  }
  consume(comp, TOKEN_KIND_RBRACKET);
end:
  hk_chunk_emit_opcode(chunk, HK_OP_ARRAY);
  hk_chunk_emit_byte(chunk, length);
  return;
}

static void compile_struct_constructor(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  hk_chunk_emit_opcode(chunk, HK_OP_NIL);
  if (match(scan, TOKEN_KIND_RBRACE))
  {
    scanner_next_token(scan);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTRUCT);
    hk_chunk_emit_byte(chunk, 0);
    return;
  }
  if (!match(scan, TOKEN_KIND_STRING) && !match(scan, TOKEN_KIND_NAME))
    syntax_error_unexpected(comp);
  Token tk = scan->token;
  scanner_next_token(scan);
  uint8_t index = add_string_constant(comp, &tk);
  hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
  hk_chunk_emit_byte(chunk, index);
  consume(comp, TOKEN_KIND_COLON);
  compile_expression(comp);
  uint8_t length = 1;
  while (match(scan, TOKEN_KIND_COMMA))
  {
    scanner_next_token(scan);
    if (!match(scan, TOKEN_KIND_STRING) && !match(scan, TOKEN_KIND_NAME))
      syntax_error_unexpected(comp);
    tk = scan->token;
    scanner_next_token(scan);
    index = add_string_constant(comp, &tk);
    hk_chunk_emit_opcode(chunk, HK_OP_CONSTANT);
    hk_chunk_emit_byte(chunk, index);
    consume(comp, TOKEN_KIND_COLON);
    compile_expression(comp);
    ++length;
  }
  consume(comp, TOKEN_KIND_RBRACE);
  hk_chunk_emit_opcode(chunk, HK_OP_CONSTRUCT);
  hk_chunk_emit_byte(chunk, length);
}

static void compile_if_expression(Compiler *comp, bool not)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_KIND_LPAREN);
  compile_expression(comp);
  consume(comp, TOKEN_KIND_RPAREN);
  HkOpCode op = not ? HK_OP_JUMP_IF_TRUE : HK_OP_JUMP_IF_FALSE;
  int offset1 = emit_jump(chunk, op);
  compile_expression(comp);
  int offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  consume(comp, TOKEN_KIND_ELSE_KW);
  compile_expression(comp);
  patch_jump(comp, offset2);
}

static void compile_match_expression(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  scanner_next_token(scan);
  consume(comp, TOKEN_KIND_LPAREN);
  compile_expression(comp);
  consume(comp, TOKEN_KIND_RPAREN);
  consume(comp, TOKEN_KIND_LBRACE);
  compile_expression(comp);
  consume(comp, TOKEN_KIND_ARROW);
  int offset1 = emit_jump(chunk, HK_OP_JUMP_IF_NOT_EQUAL);
  compile_expression(comp);
  int offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  if (match(scan, TOKEN_KIND_COMMA))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_UNDERSCORE_KW))
    {
      scanner_next_token(scan);
      consume(comp, TOKEN_KIND_ARROW);
      hk_chunk_emit_opcode(chunk, HK_OP_POP);
      compile_expression(comp);
      consume(comp, TOKEN_KIND_RBRACE);
      patch_jump(comp, offset2);
      return;
    }
    compile_match_expression_member(comp);
    patch_jump(comp, offset2);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_match_expression_member(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_expression(comp);
  consume(comp, TOKEN_KIND_ARROW);
  int offset1 = emit_jump(chunk, HK_OP_JUMP_IF_NOT_EQUAL);
  compile_expression(comp);
  int offset2 = emit_jump(chunk, HK_OP_JUMP);
  patch_jump(comp, offset1);
  if (match(scan, TOKEN_KIND_COMMA))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_UNDERSCORE_KW))
    {
      scanner_next_token(scan);
      consume(comp, TOKEN_KIND_ARROW);
      hk_chunk_emit_opcode(chunk, HK_OP_POP);
      compile_expression(comp);
      consume(comp, TOKEN_KIND_RBRACE);
      patch_jump(comp, offset2);
      return;
    }
    compile_match_expression_member(comp);
    patch_jump(comp, offset2);
    return;
  }
  syntax_error_unexpected(comp);
}

static void compile_subscript(Compiler *comp)
{
  Scanner *scan = comp->scan;
  HkChunk *chunk = &comp->fn->chunk;
  compile_variable(comp, &scan->token, true);
  scanner_next_token(scan);
  for (;;)
  {
    if (match(scan, TOKEN_KIND_LBRACKET))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      consume(comp, TOKEN_KIND_RBRACKET);
      hk_chunk_emit_opcode(chunk, HK_OP_GET_ELEMENT);
      continue;
    }
    if (match(scan, TOKEN_KIND_DOT))
    {
      scanner_next_token(scan);
      if (!match(scan, TOKEN_KIND_NAME))
        syntax_error_unexpected(comp);
      Token tk = scan->token;
      scanner_next_token(scan);
      uint8_t index = add_string_constant(comp, &tk);
      hk_chunk_emit_opcode(chunk, HK_OP_GET_FIELD);
      hk_chunk_emit_byte(chunk, index);
      continue;
    }
    if (match(scan, TOKEN_KIND_LPAREN))
    {
      scanner_next_token(scan);
      if (match(scan, TOKEN_KIND_RPAREN))
      {
        scanner_next_token(scan);
        hk_chunk_emit_opcode(chunk, HK_OP_CALL);
        hk_chunk_emit_byte(chunk, 0);
        return;
      }
      compile_expression(comp);
      uint8_t numArgs = 1;
      while (match(scan, TOKEN_KIND_COMMA))
      {
        scanner_next_token(scan);
        compile_expression(comp);
        ++numArgs;
      }
      consume(comp, TOKEN_KIND_RPAREN);
      hk_chunk_emit_opcode(chunk, HK_OP_CALL);
      hk_chunk_emit_byte(chunk, numArgs);
      continue;
    }
    break;
  }
  if (match(scan, TOKEN_KIND_LBRACE))
  {
    scanner_next_token(scan);
    if (match(scan, TOKEN_KIND_RBRACE))
    {
      scanner_next_token(scan);
      hk_chunk_emit_opcode(chunk, HK_OP_INSTANCE);
      hk_chunk_emit_byte(chunk, 0);
      return;
    }
    compile_expression(comp);
    uint8_t numArgs = 1;
    while (match(scan, TOKEN_KIND_COMMA))
    {
      scanner_next_token(scan);
      compile_expression(comp);
      ++numArgs;
    }
    consume(comp, TOKEN_KIND_RBRACE);
    hk_chunk_emit_opcode(chunk, HK_OP_INSTANCE);
    hk_chunk_emit_byte(chunk, numArgs);
  }
}

static Variable compile_variable(Compiler *comp, Token *tk, bool emit)
{
  HkFunction *fn = comp->fn;
  HkChunk *chunk = &fn->chunk;
  Variable *var = lookup_variable(comp, tk);
  if (var)
  {
    if (!emit)
      return *var;
    hk_chunk_emit_opcode(chunk, var->isLocal ? HK_OP_GET_LOCAL : HK_OP_NONLOCAL);
    hk_chunk_emit_byte(chunk, var->index);
    return *var;
  }
  var = compile_nonlocal(comp->parent, tk);
  if (var)
  {
    uint8_t index = add_nonlocal(comp, tk);
    hk_chunk_emit_opcode(chunk, HK_OP_NONLOCAL);
    hk_chunk_emit_byte(chunk, index);
    return *var;
  }
  int index = lookup_global(tk->length, tk->start);
  if (index == -1)
  {
    print_semantic_error(fn->name, comp->scan->file->chars, tk->line, tk->col,
      "variable `%.*s` is used but not defined", tk->length, tk->start);
    if (!analyze(comp))
      exit(EXIT_FAILURE);
  }
  hk_chunk_emit_opcode(chunk, HK_OP_GLOBAL);
  hk_chunk_emit_byte(chunk, (uint8_t) index);
  return (Variable) {
    .isLocal = false,
    .depth = -1,
    .index = (uint8_t) index,
    .length = tk->length,
    .start = tk->start,
    .isMutable = false
  };
}

static Variable *compile_nonlocal(Compiler *comp, Token *tk)
{
  if (!comp)
    return NULL;
  HkFunction *fn = comp->fn;
  HkChunk *chunk = &fn->chunk;
  Variable *var = lookup_variable(comp, tk);
  if (var)
  {
    HkOpCode op = HK_OP_NONLOCAL;
    if (var->isLocal)
    {
      if (var->isMutable)
      {
        print_semantic_error(fn->name, comp->scan->file->chars, tk->line, tk->col,
          "cannot capture mutable variable `%.*s`", tk->length, tk->start);
        if (!analyze(comp))
          exit(EXIT_FAILURE);
        return var;
      }
      op = HK_OP_GET_LOCAL;
    }
    hk_chunk_emit_opcode(chunk, op);
    hk_chunk_emit_byte(chunk, var->index);
    return var;
  }
  var = compile_nonlocal(comp->parent, tk);
  if (var)
  {
    uint8_t index = add_nonlocal(comp, tk);
    hk_chunk_emit_opcode(chunk, HK_OP_NONLOCAL);
    hk_chunk_emit_byte(chunk, index);
    return var;
  }
  return NULL;
}

HkClosure *hk_compile(HkString *file, HkString *source, int flags)
{
  Scanner scan;
  scanner_init(&scan, file, source);
  Compiler comp;
  compiler_init(&comp, NULL, flags, &scan, hk_string_from_chars(-1, "main"));
  char name[] = "args";
  Token tk = { .length = sizeof(name) - 1, .start = name };
  add_local(&comp, &tk, false);
  while (!match(comp.scan, TOKEN_KIND_EOF))
    compile_statement(&comp);
  HkFunction *fn = comp.fn;
  fn->arity = 1;
  HkChunk *chunk = &fn->chunk;
  hk_chunk_emit_opcode(chunk, HK_OP_RETURN_NIL);
  HkClosure *cl = hk_closure_new(fn);
  scanner_deinit(&scan);
  return cl;
}
