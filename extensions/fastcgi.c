//
// The Hook Programming Language
// fastcgi.c
//

#include "fastcgi.h"
#include <fcgi_stdio.h>
#include "hk_status.h"

static int32_t accept_call(hk_vm_t *vm, hk_value_t *args);

static int32_t accept_call(hk_vm_t *vm, hk_value_t *args)
{
  (void) args;
  return hk_vm_push_float(vm, FCGI_Accept());
}

HK_LOAD_FN(fastcgi)
{
  if (hk_vm_push_string_from_chars(vm, -1, "fastcgi") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "accept") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "accept", 0, &accept_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 1);
}
