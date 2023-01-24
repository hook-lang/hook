//
// The Hook Programming Language
// hk_regex.c
//

#include "hk_regex.h"
#include <regex.h>
#include "hk_memory.h"
#include "hk_status.h"
#include "hk_error.h"

typedef struct
{
  HK_USERDATA_HEADER
  regex_t regex;
} regex_wrapper_t;

static inline regex_wrapper_t *regex_wrapper_new(regex_t regex);
static void regex_wrapper_deinit(hk_userdata_t *udata);
static int32_t new_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_match_call(hk_vm_t *vm, hk_value_t *args);

static inline regex_wrapper_t *regex_wrapper_new(regex_t regex)
{
  regex_wrapper_t *wrapper = (regex_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &regex_wrapper_deinit);
  wrapper->regex = regex;
  return wrapper;
}

static void regex_wrapper_deinit(hk_userdata_t *udata)
{
  regfree(&((regex_wrapper_t *) udata)->regex);
}

static int32_t new_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  regex_t regex;
  int32_t errcode = regcomp(&regex, str->chars, REG_EXTENDED);
  if (errcode)
  {
    char errbuf[1024];
    regerror(errcode, &regex, errbuf, sizeof(errbuf));
    hk_runtime_error("cannot compile regex: %s", errbuf);
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_userdata(vm, (hk_userdata_t *) regex_wrapper_new(regex));
}

static int32_t is_match_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  regex_wrapper_t *wrapper = (regex_wrapper_t *) hk_as_userdata(args[1]);
  hk_string_t *str = hk_as_string(args[2]);
  int32_t errcode = regexec(&wrapper->regex, str->chars, 0, NULL, 0);
  if (errcode)
  {
    if (errcode == REG_NOMATCH)
      return hk_vm_push_bool(vm, false);
    char errbuf[1024];
    regerror(errcode, &wrapper->regex, errbuf, sizeof(errbuf));
    hk_runtime_error("cannot match regex: %s", errbuf);
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_bool(vm, true);
}

HK_LOAD_FN(regex)
{
  if (hk_vm_push_string_from_chars(vm, -1, "regex") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "new") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "new", 1, &new_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "is_match") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "is_match", 1, &is_match_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 2);
}
