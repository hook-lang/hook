//
// Hook Programming Language
// builtin.c
//

#include "builtin.h"

static void println_call(vm_t *vm, value_t *frame);

static void println_call(vm_t *vm, value_t *frame)
{
  value_print(frame[1], false);
  printf("\n");
  vm_push_null(vm);
}

void globals_init(vm_t *vm)
{
  vm_push_native(vm, native_new(string_from_chars(-1, "println"), 1, &println_call));
}
