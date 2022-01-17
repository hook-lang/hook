//
// Hook Programming Language
// main.c
//

#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "compiler.h"
#include "dump.h"
#include "vm.h"
#include "builtin.h"
#include "common.h"
#include "error.h"

#define VERSION "0.1.0"

typedef struct
{
  const char *cmd;
  bool opt_help;
  bool opt_version;
  bool opt_dump;
  bool opt_compile;
  bool opt_run;
  int stack_size; 
  const char *input;
  const char *output;
  const char **args;
  int num_args;
} parsed_args_t;

static inline void parse_args(parsed_args_t *parsed_args, int argc, const char **argv);
static inline void parse_option(parsed_args_t *parsed_args, const char *arg);
static inline const char *option(const char *arg, const char *opt);
static inline array_t *args_array(parsed_args_t *parsed_args);
static inline void print_help(const char *cmd);
static inline void print_version(void);
static inline closure_t *load_bytecode_from_file(const char *filename);
static inline closure_t *load_bytecode_from_stream(FILE *stream);
static inline void save_bytecode_to_file(closure_t *cl, const char *filename);
static inline string_t *load_source_from_file(const char *filename);
static inline closure_t *compile_source(const char *input);
static inline int run_bytecode(closure_t *cl, parsed_args_t *parsed_args);

static inline void parse_args(parsed_args_t *parsed_args, int argc, const char **argv)
{
  parsed_args->cmd = argv[0];
  parsed_args->opt_help = false;
  parsed_args->opt_version = false;
  parsed_args->opt_dump = false;
  parsed_args->opt_compile = false;
  parsed_args->opt_run = false;
  parsed_args->stack_size = 0;
  parsed_args->input = NULL;
  parsed_args->output = NULL;
  int i = 1;
  for (; i < argc; ++i)
  {
    const char *arg = argv[i];
    if (arg[0] != '-')
      break;
    parse_option(parsed_args, arg);
  }
  if (i < argc)
  {
    parsed_args->input = argv[i];
    ++i;
  }
  if (i < argc)
    parsed_args->output = argv[i];
  parsed_args->args = &argv[i];
  parsed_args->num_args = argc - i;
}

static inline void parse_option(parsed_args_t *parsed_args, const char *arg)
{
  if (option(arg, "-h") || option(arg, "--help"))
  {
    parsed_args->opt_help = true;
    return;
  }
  if (option(arg, "-v") || option(arg, "--version"))
  {
    parsed_args->opt_version = true;
    return;
  }
  if (option(arg, "-d") || option(arg, "--dump"))
  {
    parsed_args->opt_dump = true;
    return;
  }
  if (option(arg, "-c") || option(arg, "--compile"))
  {
    parsed_args->opt_compile = true;
    return;
  }
  if (option(arg, "-r") || option(arg, "--run"))
  {
    parsed_args->opt_run = true;
    return;
  }
  const char *opt_val = option(arg, "-s");
  if (opt_val)
  {
    parsed_args->stack_size = atoi(opt_val);
    return;
  }
  fatal_error("unknown option `%s`\n", arg);
}

static inline const char *option(const char *arg, const char *opt)
{
  int len = 0;
  while (opt[len] && opt[len] != '=')
    ++len;
  if (memcmp(arg, opt, len))
    return NULL;
  return arg[len] == '=' ? &arg[len + 1] : &arg[len];
}

static inline array_t *args_array(parsed_args_t *parsed_args)
{
  int length = parsed_args->num_args;
  array_t *args = array_allocate(length);
  args->length = length;
  for (int i = 0; i < length; ++i)
  {
    string_t *arg = string_from_chars(-1, parsed_args->args[i]);
    INCR_REF(arg);
    args->elements[i] = STRING_VALUE(arg);
  }
  return args;
}

static inline void print_help(const char *cmd)
{
  printf(
    "usage: %s [options] [input] [output]\n"
    "\n"
    "options:\n"
    "  -h, --help     prints this message\n"
    "  -v, --version  shows version information\n"
    "  -d, --dump     shows the bytecode\n"
    "  -c, --compile  compiles source code\n"
    "  -r, --run      Runs directly from bytecode\n"
    "  -s=<size>      sets the stack size\n"
    "\n",
  cmd);
}

static inline void print_version(void)
{
  printf("hook version %s\n", VERSION);
}

static inline closure_t *load_bytecode_from_file(const char *filename)
{
  FILE *stream = fopen(filename, "rb");
  if (!stream)
    fatal_error("unable to open file `%s`", filename);
  closure_t *cl = load_bytecode_from_stream(stream);
  if (!cl)
    fatal_error("unable to load file `%s`", filename);
  fclose(stream);
  return cl;
}

static inline closure_t *load_bytecode_from_stream(FILE *stream)
{
  function_t *fn = function_deserialize(stream);
  if (!fn)
    return NULL;
  return closure_new(fn);
}

static inline void save_bytecode_to_file(closure_t *cl, const char *filename)
{
  filename = filename ? filename : "a.out";
  ensure_path(filename);
  FILE *stream = fopen(filename, "wb");
  if (!stream)
    fatal_error("unable to open file `%s`", filename);
  function_serialize(cl->fn, stream);
  fclose(stream);
}

static inline string_t *load_source_from_file(const char *filename)
{
  FILE *stream = fopen(filename, "rb");
  if (!stream)
    fatal_error("unable to open file `%s`", filename);
  fseek(stream, 0L, SEEK_END);
  int length = ftell(stream);
  rewind(stream);
  string_t *str = string_allocate(length);
  str->length = length;
  ASSERT(fread(str->chars, length, 1, stream) == 1, "unexpected error on fread()");
  str->chars[length] = '\0';
  fclose(stream);
  return str;
}

static inline closure_t *compile_source(const char *input)
{
  string_t *file = string_from_chars(-1, input ? input : "<stdin>");
  string_t *source = input ? load_source_from_file(input) : string_from_stream(stdin, '\0');
  return compile(file, source);
}

static inline int run_bytecode(closure_t *cl, parsed_args_t *parsed_args)
{
  vm_t vm;
  vm_init(&vm, parsed_args->stack_size);
  load_globals(&vm);
  vm_push_closure(&vm, cl);
  vm_push_array(&vm, args_array(parsed_args));
  if (vm_call(&vm, 1) == STATUS_ERROR)
  {
    vm_free(&vm);
    return EXIT_FAILURE;
  }
  value_t result = vm.slots[vm.top];
  int status = IS_INTEGER(result) ? (int) result.as.number : 0;
  --vm.top;
  ASSERT(vm.top == num_globals() - 1, "stack must contain the globals");
  vm_free(&vm);
  return status;
}

int main(int argc, const char **argv)
{
  parsed_args_t parsed_args;
  parse_args(&parsed_args, argc, argv);
  if (parsed_args.opt_help)
  {
    print_help(parsed_args.cmd);
    return EXIT_SUCCESS;
  }
  if (parsed_args.opt_version)
  {
    print_version();
    return EXIT_SUCCESS;
  }
  if (parsed_args.opt_run)
  {
    const char *input = parsed_args.input;
    if (input)
    {
      closure_t *cl = load_bytecode_from_file(input);
      return run_bytecode(cl, &parsed_args);
    }
    closure_t *cl = load_bytecode_from_stream(stdin);
    if (!cl)
      fatal_error("unable to load bytecode");
    return run_bytecode(cl, &parsed_args);
  }
  closure_t *cl = compile_source(parsed_args.input);
  if (parsed_args.opt_dump)
  {
    dump(cl->fn);
    closure_free(cl);
    return EXIT_SUCCESS;
  }
  if (parsed_args.opt_compile)
  {
    save_bytecode_to_file(cl, parsed_args.output);
    closure_free(cl);
    return EXIT_SUCCESS;
  }
  return run_bytecode(cl, &parsed_args);
}
