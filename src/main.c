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
static inline void save_to_file(function_t *fn, const char *filename);
static inline function_t *load_from_file(const char *filename);
static inline function_t *load_from_stream(FILE *stream);
static inline int run(function_t *fn, parsed_args_t *parsed_args);

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

static inline void save_to_file(function_t *fn, const char *filename)
{
  filename = filename ? filename : "a.out";
  ensure_path(filename);
  FILE *stream = fopen(filename, "w");
  if (!stream)
    fatal_error("unable to open file `%s`", filename);
  prototype_serialize(fn->proto, stream);
  fclose(stream);
}

static inline function_t *load_from_file(const char *filename)
{
  FILE *stream = fopen(filename, "r");
  if (!stream)
    fatal_error("unable to open file `%s`", filename);
  function_t *fn = load_from_stream(stream);
  if (!fn)
    fatal_error("unable to load file `%s`", filename);
  fclose(stream);
  return fn;
}

static inline function_t *load_from_stream(FILE *stream)
{
  prototype_t *proto = prototype_deserialize(stream);
  if (!proto)
    return NULL;
  return function_new(proto);
}

static inline int run(function_t *fn, parsed_args_t *parsed_args)
{
  vm_t vm;
  vm_init(&vm, parsed_args->stack_size);
  load_globals(&vm);
  vm_push_function(&vm, fn);
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
      function_t *fn = load_from_file(input);
      return run(fn, &parsed_args);
    }
    function_t *fn = load_from_stream(stdin);
    if (!fn)
      fatal_error("unable to load bytecode");
    return run(fn, &parsed_args);
  }
  const char *input = parsed_args.input;
  string_t *file = string_from_chars(-1, input ? input : "<stdin>");
  string_t *source = input ? string_from_file(input) : string_from_stream(stdin, '\0');
  function_t *fn = compile(file, source);
  if (parsed_args.opt_dump)
  {
    dump(fn->proto);
    function_free(fn);
    return EXIT_SUCCESS;
  }
  if (parsed_args.opt_compile)
  {
    save_to_file(fn, parsed_args.output);
    function_free(fn);
    return EXIT_SUCCESS;
  }
  return run(fn, &parsed_args);
}
