//
// The Hook Programming Language
// main.c
//

#include <stdlib.h>
#include <string.h>
#include "hk_compiler.h"
#include "hk_dump.h"
#include "hk_state.h"
#include "hk_status.h"
#include "hk_error.h"
#include "hk_utils.h"
#include "hk_version.h"

typedef struct
{
  const char *cmd;
  bool opt_help;
  bool opt_version;
  bool opt_eval;
  bool opt_dump;
  bool opt_compile;
  bool opt_run;
  int32_t stack_size; 
  const char *input;
  const char *output;
  const char **args;
  int32_t num_args;
} parsed_args_t;

static inline void parse_args(parsed_args_t *parsed_args, int32_t argc, const char **argv);
static inline void parse_option(parsed_args_t *parsed_args, const char *arg);
static inline const char *option(const char *arg, const char *opt);
static inline hk_array_t *args_array(parsed_args_t *parsed_args);
static inline void print_help(const char *cmd);
static inline void print_version(void);
static inline FILE *open_file(const char *filename, const char *mode);
static inline hk_string_t *load_source_from_file(const char *filename);
static inline hk_closure_t *load_bytecode_from_file(const char *filename);
static inline hk_closure_t *load_bytecode_from_stream(FILE *stream);
static inline void save_bytecode_to_file(hk_closure_t *cl, const char *filename);
static inline void dump_bytecode_to_file(hk_function_t *fn, const char *filename);
static inline int32_t run_bytecode(hk_closure_t *cl, parsed_args_t *parsed_args);

static inline void parse_args(parsed_args_t *parsed_args, int32_t argc, const char **argv)
{
  parsed_args->cmd = argv[0];
  parsed_args->opt_help = false;
  parsed_args->opt_version = false;
  parsed_args->opt_eval = false;
  parsed_args->opt_dump = false;
  parsed_args->opt_compile = false;
  parsed_args->opt_run = false;
  parsed_args->stack_size = 0;
  parsed_args->input = NULL;
  parsed_args->output = NULL;
  int32_t i = 1;
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
  if (option(arg, "-e") || option(arg, "--eval"))
  {
    parsed_args->opt_eval = true;
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
  hk_fatal_error("unknown option `%s`\n", arg);
}

static inline const char *option(const char *arg, const char *opt)
{
  int32_t length = 0;
  while (opt[length] && opt[length] != '=')
    ++length;
  if (memcmp(arg, opt, length))
    return NULL;
  return arg[length] == '=' ? &arg[length + 1] : &arg[length];
}

static inline hk_array_t *args_array(parsed_args_t *parsed_args)
{
  int32_t length = parsed_args->num_args;
  hk_array_t *args = hk_array_new_with_capacity(length);
  args->length = length;
  for (int32_t i = 0; i < length; ++i)
  {
    hk_string_t *arg = hk_string_from_chars(-1, parsed_args->args[i]);
    hk_incr_ref(arg);
    args->elements[i] = hk_string_value(arg);
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
    "  -e, --eval     evaluates a string from the terminal\n"
    "  -d, --dump     shows the bytecode\n"
    "  -c, --compile  compiles source code\n"
    "  -r, --run      runs directly from bytecode\n"
    "  -s=<size>      sets the stack size\n"
    "\n",
  cmd);
}

static inline void print_version(void)
{
  printf("hook %s %s\n", HK_VERSION, HK_REVISION);
}

static inline FILE *open_file(const char *filename, const char *mode)
{
  FILE *stream = fopen(filename, mode);
  if (!stream)
    hk_fatal_error("unable to open file `%s`", filename);
  return stream;
}

static inline hk_string_t *load_source_from_file(const char *filename)
{
  FILE *stream = open_file(filename, "r");
  hk_string_t *source = hk_string_from_stream(stream, '\0');
  fclose(stream);
  return source;
}

static inline hk_closure_t *load_bytecode_from_file(const char *filename)
{
  FILE *stream = open_file(filename, "rb");
  hk_closure_t *cl = load_bytecode_from_stream(stream);
  if (!cl)
    hk_fatal_error("unable to load file `%s`", filename);
  fclose(stream);
  return cl;
}

static inline hk_closure_t *load_bytecode_from_stream(FILE *stream)
{
  hk_function_t *fn = hk_function_deserialize(stream);
  if (!fn)
    return NULL;
  return hk_closure_new(fn);
}

static inline void save_bytecode_to_file(hk_closure_t *cl, const char *filename)
{
  filename = filename ? filename : "a.out";
  hk_ensure_path(filename);
  FILE *stream = open_file(filename, "wb");
  hk_function_serialize(cl->fn, stream);
  fclose(stream);
}

static inline void dump_bytecode_to_file(hk_function_t *fn, const char *filename)
{
  hk_ensure_path(filename);
  FILE *stream = open_file(filename, "w");
  hk_dump(fn, stream);
  fclose(stream);
}

static inline int32_t run_bytecode(hk_closure_t *cl, parsed_args_t *parsed_args)
{
  hk_state_t state;
  hk_state_init(&state, parsed_args->stack_size);
  hk_state_push_closure(&state, cl);
  hk_state_push_array(&state, args_array(parsed_args));
  if (hk_state_call(&state, 1) == HK_STATUS_ERROR)
  {
    hk_state_free(&state);
    return EXIT_FAILURE;
  }
  hk_value_t result = state.stack[state.stack_top];
  int32_t status = hk_is_int(result) ? (int32_t) hk_as_number(result) : 0;
  --state.stack_top;
  hk_state_free(&state);
  return status;
}

int32_t main(int32_t argc, const char **argv)
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
  const char *input = parsed_args.input;
  if (parsed_args.opt_eval)
  {
    if (!input)
      hk_fatal_error("no input string");
    hk_string_t *file = hk_string_from_chars(-1, "<terminal>");
    hk_string_t *source = hk_string_from_chars(-1, input);
    hk_closure_t *cl = hk_compile(file, source);
    return run_bytecode(cl, &parsed_args);
  }
  if (parsed_args.opt_run)
  {
    if (input)
    {
      hk_closure_t *cl = load_bytecode_from_file(input);
      return run_bytecode(cl, &parsed_args);
    }
    hk_closure_t *cl = load_bytecode_from_stream(stdin);
    if (!cl)
      hk_fatal_error("unable to load bytecode");
    return run_bytecode(cl, &parsed_args);
  }
  hk_string_t *file = hk_string_from_chars(-1, input ? input : "<stdin>");
  hk_string_t *source = input ? load_source_from_file(input) : hk_string_from_stream(stdin, '\0');
  hk_closure_t *cl = hk_compile(file, source);
  const char *output = parsed_args.output;
  if (parsed_args.opt_dump)
  {
    if (output)
    {
      dump_bytecode_to_file(cl->fn, output);
      hk_closure_free(cl);
      return EXIT_SUCCESS;
    }
    hk_dump(cl->fn, stdout);
    hk_closure_free(cl);
    return EXIT_SUCCESS;
  }
  if (parsed_args.opt_compile)
  {
    save_bytecode_to_file(cl, output);
    hk_closure_free(cl);
    return EXIT_SUCCESS;
  }
  return run_bytecode(cl, &parsed_args);
}
