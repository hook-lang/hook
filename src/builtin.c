//
// Hook Programming Language
// builtin.c
//

#include "builtin.h"
#include "error.h"

static void println_call(vm_t *vm, value_t *frame);
static void len_call(vm_t *vm, value_t *frame);

static void println_call(vm_t *vm, value_t *frame)
{
  value_print(frame[1], false);
  printf("\n");
  vm_push_null(vm);
}

static void len_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    vm_push_number(vm, AS_STRING(val)->length);
    return;
  case TYPE_ARRAY:
    vm_push_number(vm, AS_ARRAY(val)->length);
    return;
  default:
    break;
  }
  fatal_error("value of type '%s' has no length", type_name(val.type));
}

void globals_init(vm_t *vm)
{
  vm_push_native(vm, native_new(string_from_chars(-1, "println"), 1, &println_call));
  vm_push_native(vm, native_new(string_from_chars(-1, "len"), 1, &len_call));
}
