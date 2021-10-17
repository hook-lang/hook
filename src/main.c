//
// Hook Programming Language
// main.c
//

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "common.h"
#include "disasm.h"
#include "vm.h"

static inline const char *get_argument(int argc, const char **argv, int index);
static inline bool has_option(int argc, const char **argv, const char *option);

static inline const char *get_argument(int argc, const char **argv, int index)
{
  int j = 0;
  for (int i = 1; i < argc; ++i)
    if (argv[i][0] != '-' && index == j++)
      return argv[i];
  return NULL;
}

static inline bool has_option(int argc, const char **argv, const char *option)
{
  for (int i = 1; i < argc; ++i)
    if (!strcmp(argv[i], option))
      return true;
  return false;
}

int main(int argc, const char **argv)
{
  const char *filename = get_argument(argc, argv, 0);
  string_t *file = string_from_chars(-1, filename ? filename : "<stdin>");
  string_t *source = filename ? string_from_file(filename) : string_from_stream(stdin);
  vm_t vm;
  vm_init(&vm, 0);
  vm_push_string(&vm, file);
  vm_push_string(&vm, source);
  vm_compile(&vm);
  if (has_option(argc, argv, "--disasm"))
  {
    function_t *fn = AS_FUNCTION(vm.slots[vm.index]);
    dump(fn);
    vm_free(&vm);
    return EXIT_SUCCESS;
  }
  if (vm_call(&vm, 0) == STATUS_ERROR)
  {
    vm_free(&vm);
    return EXIT_FAILURE;
  }
  vm_pop(&vm);
  vm_free(&vm);
  return EXIT_SUCCESS;
}
