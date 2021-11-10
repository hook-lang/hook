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

static int _argc;
static const char **_argv;

static inline const char *argument(int index);
static inline int option(const char *opt);
static inline const char *option_value(const char *opt);
static inline int option_length(const char *opt);
static inline void print_help(void);
static inline void print_version(void);
static inline array_t *args_array(void);
static inline void save(function_t *fn);
static inline function_t *load(void);
static inline int run(function_t *fn);

static inline const char *argument(int index)
{
  int j = 0;
  for (int i = 1; i < _argc; ++i)
    if (_argv[i][0] != '-' && index == j++)
      return _argv[i];
  return NULL;
}

static inline int option(const char *opt)
{
  for (int i = 1; i < _argc; ++i)
    if (!strcmp(_argv[i], opt))
      return i;
  return 0;
}

static inline const char *option_value(const char *opt)
{
  int len = option_length(opt);
  for (int i = 1; i < _argc; ++i)
  {
    const char *arg = _argv[i];
    if (!memcmp(arg, opt, len))
      return arg[len] == '=' ? &arg[len + 1] : &arg[len];
  }
  return NULL;  
}

static inline int option_length(const char *opt)
{
  int len = 0;
  while (opt[len] && opt[len] != '=')
    ++len;
  return len;
}

static inline void print_help(void)
{
  printf(
    "usage: %s [options] [filename]\n"
    "\n"
    "options:\n"
    "  -h, --help     prints this message\n"
    "  -v, --version  shows version information\n"
    "  -d, --dump     shows the bytecode\n"
    "  -c, --compile  compiles source code\n"
    "  -r, --run      Runs directly from bytecode\n"
    "  -s=<size>      sets the stack size\n"
    "  -o=<output>    sets output directory\n"
    "\n",
  _argv[0]);
}

static inline void print_version(void)
{
  printf("hook version %s\n", VERSION);
}

static inline array_t *args_array(void)
{
  array_t *args = array_allocate(_argc);
  args->length = _argc;
  for (int i = 0; i < _argc; ++i)
  {
    string_t *arg = string_from_chars(-1, _argv[i]);
    INCR_REF(arg);
    args->elements[i] = STRING_VALUE(arg);
  }
  return args;
}

static inline void save(function_t *fn)
{
  const char *o = option_value("-o");
  const char *filename = o ? o : "a.out";
  ensure_path(filename);
  FILE *stream = fopen(filename, "w");
  if (!stream)
    fatal_error("unable to open file '%s'", filename);
  prototype_serialize(fn->proto, stream);
  fclose(stream);
}

static inline function_t *load(void)
{
  const char *filename = argument(0);
  FILE *stream = fopen(filename, "r");
  if (!stream)
    fatal_error("unable to open file '%s'", filename);
  prototype_t *proto = prototype_deserialize(stream);
  fclose(stream);
  return function_new(proto);
}

static inline int run(function_t *fn)
{
  const char *s = option_value("-s");
  int stack_size = s ? atoi(s) : 0;
  vm_t vm;
  vm_init(&vm, stack_size);
  load_globals(&vm);
  vm_push_function(&vm, fn);
  vm_push_array(&vm, args_array());
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
  _argc = argc;
  _argv = argv;
  if (option("-h") || option("--help"))
  {
    print_help();
    return EXIT_SUCCESS;
  }
  if (option("-v") || option("--version"))
  {
    print_version();
    return EXIT_SUCCESS;
  }
  if (option("-r") || option("--run"))
  {
    function_t *fn = load();
    return run(fn);
  }
  const char *filename = argument(0);
  string_t *file = string_from_chars(-1, filename ? filename : "<stdin>");
  string_t *source = filename ? string_from_file(filename) : string_from_stream(stdin, '\0');
  function_t *fn = compile(file, source);
  if (option("-d") || option("--dump"))
  {
    dump(fn->proto);
    function_free(fn);
    return EXIT_SUCCESS;
  }
  if (option("-c") || option("--compile"))
  {
    save(fn);
    function_free(fn);
    return EXIT_SUCCESS;
  }
  return run(fn);
}
