//
// The Hook Programming Language
// hk_utf8.c
//

#include "hk_utf8.h"
#include "hk_status.h"

static inline int32_t decode_char(unsigned char c);
static int32_t len_call(hk_vm_t *vm, hk_value_t *args);

static inline int32_t decode_char(unsigned char c)
{
  if ((c & 0xc0) == 0x80)
    return 0;
  if ((c & 0xf8) == 0xf0)
    return 4;
  if ((c & 0xf0) == 0xe0)
    return 3;
  if ((c & 0xe0) == 0xc0)
    return 2;
  return 1;
}

static int32_t len_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t result = 0;
  for (int32_t i = 0; i < str->length;)
  {
    int32_t length = decode_char((unsigned char) str->chars[i]);
    if (!length)
      break;
    i += length;
    ++result;
  }
  return hk_vm_push_float(vm, result);
}

HK_LOAD_FN(utf8)
{
  if (hk_vm_push_string_from_chars(vm, -1, "utf8") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "len") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "len", 1, &len_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 1);
}
