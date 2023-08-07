//
// The Hook Programming Language
// main.c
//

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <hook/compiler.h>
#include <hook/dump.h>
#include <hook/state.h>
#include <hook/utils.h>

#define VERSION "0.1.0"

typedef struct
{
  const char *cmd;
  bool optHelp;
  bool optVersion;
  bool optEval;
  bool optDump;
  bool optCompile;
  bool optRun;
  int stackSize; 
  const char *input;
  const char *output;
  const char **args;
  int num_args;
} ParsedArgs;

static inline void fatal_error(const char *fmt, ...);
static inline void parse_args(ParsedArgs *parsedArgs, int argc, const char **argv);
static inline void parse_option(ParsedArgs *parsedArgs, const char *arg);
static inline const char *option(const char *arg, const char *opt);
static inline HkArray *args_array(ParsedArgs *parsedArgs);
static inline void print_help(const char *cmd);
static inline void print_version(void);
static inline FILE *open_file(const char *filename, const char *mode);
static inline HkString *load_source_from_file(const char *filename);
static inline HkClosure *load_bytecode_from_file(const char *filename);
static inline HkClosure *load_bytecode_from_stream(FILE *stream);
static inline void save_bytecode_to_file(HkClosure *cl, const char *filename);
static inline void dump_bytecode_to_file(HkFunction *fn, const char *filename);
static inline int run_bytecode(HkClosure *cl, ParsedArgs *parsedArgs);

static inline void fatal_error(const char *fmt, ...)
{
  fprintf(stderr, "fatal error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
  exit(EXIT_FAILURE);
}

static inline void parse_args(ParsedArgs *parsedArgs, int argc, const char **argv)
{
  parsedArgs->cmd = argv[0];
  parsedArgs->optHelp = false;
  parsedArgs->optVersion = false;
  parsedArgs->optEval = false;
  parsedArgs->optDump = false;
  parsedArgs->optCompile = false;
  parsedArgs->optRun = false;
  parsedArgs->stackSize = 0;
  parsedArgs->input = NULL;
  parsedArgs->output = NULL;
  int i = 1;
  for (; i < argc; ++i)
  {
    const char *arg = argv[i];
    if (arg[0] != '-')
      break;
    parse_option(parsedArgs, arg);
  }
  if (i < argc)
  {
    parsedArgs->input = argv[i];
    ++i;
  }
  if (i < argc)
    parsedArgs->output = argv[i];
  parsedArgs->args = &argv[i];
  parsedArgs->num_args = argc - i;
}

static inline void parse_option(ParsedArgs *parsedArgs, const char *arg)
{
  if (option(arg, "-h") || option(arg, "--help"))
  {
    parsedArgs->optHelp = true;
    return;
  }
  if (option(arg, "-v") || option(arg, "--version"))
  {
    parsedArgs->optVersion = true;
    return;
  }
  if (option(arg, "-e") || option(arg, "--eval"))
  {
    parsedArgs->optEval = true;
    return;
  }
  if (option(arg, "-d") || option(arg, "--dump"))
  {
    parsedArgs->optDump = true;
    return;
  }
  if (option(arg, "-c") || option(arg, "--compile"))
  {
    parsedArgs->optCompile = true;
    return;
  }
  if (option(arg, "-r") || option(arg, "--run"))
  {
    parsedArgs->optRun = true;
    return;
  }
  const char *opt_val = option(arg, "-s");
  if (opt_val)
  {
    parsedArgs->stackSize = atoi(opt_val);
    return;
  }
  fatal_error("unknown option `%s`\n", arg);
}

static inline const char *option(const char *arg, const char *opt)
{
  int length = 0;
  while (opt[length] && opt[length] != '=')
    ++length;
  if (memcmp(arg, opt, length))
    return NULL;
  return arg[length] == '=' ? &arg[length + 1] : &arg[length];
}

static inline HkArray *args_array(ParsedArgs *parsedArgs)
{
  int length = parsedArgs->num_args;
  HkArray *args = hk_array_new_with_capacity(length);
  args->length = length;
  for (int i = 0; i < length; ++i)
  {
    HkString *arg = hk_string_from_chars(-1, parsedArgs->args[i]);
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
  printf("hook %s\n", VERSION);
}

static inline FILE *open_file(const char *filename, const char *mode)
{
  FILE *stream = fopen(filename, mode);
  if (!stream)
    fatal_error("unable to open file `%s`", filename);
  return stream;
}

static inline HkString *load_source_from_file(const char *filename)
{
  FILE *stream = open_file(filename, "r");
  HkString *source = hk_string_from_stream(stream, '\0');
  (void) fclose(stream);
  return source;
}

static inline HkClosure *load_bytecode_from_file(const char *filename)
{
  FILE *stream = open_file(filename, "rb");
  HkClosure *cl = load_bytecode_from_stream(stream);
  if (!cl)
    fatal_error("unable to load file `%s`", filename);
  (void) fclose(stream);
  return cl;
}

static inline HkClosure *load_bytecode_from_stream(FILE *stream)
{
  HkFunction *fn = hk_function_deserialize(stream);
  if (!fn)
    return NULL;
  return hk_closure_new(fn);
}

static inline void save_bytecode_to_file(HkClosure *cl, const char *filename)
{
  filename = filename ? filename : "a.out";
  hk_ensure_path(filename);
  FILE *stream = open_file(filename, "wb");
  hk_function_serialize(cl->fn, stream);
  (void) fclose(stream);
}

static inline void dump_bytecode_to_file(HkFunction *fn, const char *filename)
{
  hk_ensure_path(filename);
  FILE *stream = open_file(filename, "w");
  hk_dump(fn, stream);
  (void) fclose(stream);
}

static inline int run_bytecode(HkClosure *cl, ParsedArgs *parsedArgs)
{
  HkState state;
  hk_state_init(&state, parsedArgs->stackSize);
  hk_state_push_closure(&state, cl);
  hk_state_push_array(&state, args_array(parsedArgs));
  hk_state_call(&state, 1);
  int exitCode = EXIT_FAILURE;
  if (hk_state_is_ok(&state))
  {
    HkValue result = state.stackSlots[state.stackTop];
    exitCode = hk_is_int(result) ? (int) hk_as_number(result) : EXIT_SUCCESS;
    hk_state_pop(&state);
    goto end;
  }
  if (hk_state_is_exit(&state))
  {
    HkValue result = state.stackSlots[state.stackTop];
    hk_assert(hk_is_int(result), "exit code must be an integer");
    exitCode = (int) hk_as_number(result);
    hk_state_pop(&state);
  }
end:
  hk_state_deinit(&state);
  return exitCode;
}

int main(int argc, const char **argv)
{
  ParsedArgs parsedArgs;
  parse_args(&parsedArgs, argc, argv);
  if (parsedArgs.optHelp)
  {
    print_help(parsedArgs.cmd);
    return EXIT_SUCCESS;
  }
  if (parsedArgs.optVersion)
  {
    print_version();
    return EXIT_SUCCESS;
  }
  const char *input = parsedArgs.input;
  if (parsedArgs.optEval)
  {
    if (!input)
      fatal_error("no input string");
    HkString *file = hk_string_from_chars(-1, "<terminal>");
    HkString *source = hk_string_from_chars(-1, input);
    HkClosure *cl = hk_compile(file, source);
    return run_bytecode(cl, &parsedArgs);
  }
  if (parsedArgs.optRun)
  {
    if (input)
    {
      HkClosure *cl = load_bytecode_from_file(input);
      return run_bytecode(cl, &parsedArgs);
    }
    HkClosure *cl = load_bytecode_from_stream(stdin);
    if (!cl)
      fatal_error("unable to load bytecode");
    return run_bytecode(cl, &parsedArgs);
  }
  HkString *file = hk_string_from_chars(-1, input ? input : "<stdin>");
  HkString *source = input ? load_source_from_file(input) : hk_string_from_stream(stdin, '\0');
  HkClosure *cl = hk_compile(file, source);
  const char *output = parsedArgs.output;
  if (parsedArgs.optDump)
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
  if (parsedArgs.optCompile)
  {
    save_bytecode_to_file(cl, output);
    hk_closure_free(cl);
    return EXIT_SUCCESS;
  }
  return run_bytecode(cl, &parsedArgs);
}
