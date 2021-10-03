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

static inline bool has_option(int argc, const char **argv, const char *option);

static inline bool has_option(int argc, const char **argv, const char *option)
{
  for (int i = 1; i < argc; ++i)
    if (!strcmp(argv[i], option))
      return true;
  return false;
}

int main(int argc, const char **argv)
{
  string_t *str = string_from_stream(stdin);
  vm_t vm;
  vm_init(&vm, 0);
  vm_push_string(&vm, str);
  vm_compile(&vm);
  if (has_option(argc, argv, "--disasm"))
  {
    function_t *fn = AS_FUNCTION(vm.slots[vm.index]);
    dump(fn);
    vm_free(&vm);
    return EXIT_SUCCESS;
  }
  vm_call(&vm, 0);
  vm_pop(&vm);
  ASSERT(vm.index == -1, "stack must be empty");
  vm_free(&vm);
  return EXIT_SUCCESS;
}
