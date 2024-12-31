//
// regex.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "regex.h"
#include <pcre.h>

typedef struct
{
  HK_USERDATA_HEADER
  pcre *pcre;
} Regex;

static inline Regex *regex_new(pcre *pcre);
static void regex_deinit(HkUserdata *udata);
static void new_call(HkVM *vm, HkValue *args);
static void is_match_call(HkVM *vm, HkValue *args);

static inline Regex *regex_new(pcre *pcre)
{
  Regex *regex = (Regex *) hk_allocate(sizeof(*regex));
  hk_userdata_init((HkUserdata *) regex, regex_deinit);
  regex->pcre = pcre;
  return regex;
}

static void regex_deinit(HkUserdata *udata)
{
  pcre_free(((Regex *) udata)->pcre);
}

static void new_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *pattern = hk_as_string(args[1]);
  const char *error;
  int offset;
  pcre *pcre = pcre_compile(pattern->chars, 0, &error, &offset, NULL);
  HkArray *arr = hk_array_new_with_capacity(2);
  if (!pcre)
  {
    char message[128];
    snprintf(message, sizeof(message), "compilation failed at offset %d: %s", offset, error);
    hk_array_inplace_append_element(arr, hk_nil_value());
    hk_array_inplace_append_element(arr, hk_string_value(hk_string_from_chars(-1, message)));
    hk_vm_push_array(vm, arr);
    return;
  }
  HkUserdata *udata = (HkUserdata *) regex_new(pcre);
  hk_array_inplace_append_element(arr, hk_userdata_value(udata));
  hk_array_inplace_append_element(arr, hk_nil_value());
  hk_vm_push_array(vm, arr);
  if (!hk_vm_is_ok(vm))
    regex_deinit(udata);
}

static void is_match_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  pcre *pcre = ((Regex *) hk_as_userdata(args[1]))->pcre;
  HkString *subject = hk_as_string(args[2]);
  int rc = pcre_exec(pcre, NULL, subject->chars, subject->length, 0, 0, NULL, 0);
  hk_vm_push_bool(vm, rc >= 0);
}

HK_LOAD_MODULE_HANDLER(regex)
{
  hk_vm_push_string_from_chars(vm, -1, "regex");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new", 1, new_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "is_match");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "is_match", 2, is_match_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 2);
}
