//
// The Hook Programming Language
// curl.c
//

#include "curl.h"
#include <curl/curl.h>
#include "hk_memory.h"
#include "hk_status.h"
#include "hk_error.h"

typedef struct
{
  HK_USERDATA_HEADER
  CURL *handle;
} curl_t;

static inline curl_t *url_new(CURL *handle);
static void url_deinit(hk_userdata_t *udata);
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data);
static int32_t init_call(hk_vm_t *vm, hk_value_t *args);
static int32_t setopt_call(hk_vm_t *vm, hk_value_t *args);
static int32_t cleanup_call(hk_vm_t *vm, hk_value_t *args);
static int32_t perform_call(hk_vm_t *vm, hk_value_t *args);

static inline curl_t *url_new(CURL *handle)
{
  curl_t *curl = (curl_t *) hk_allocate(sizeof(*curl));
  hk_userdata_init((hk_userdata_t *) curl, &url_deinit);
  curl->handle = handle;
  return curl;
}

static void url_deinit(hk_userdata_t *udata)
{
  curl_easy_cleanup(((curl_t *) udata)->handle);
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data)
{
  hk_string_t *str = (hk_string_t *) data;
  size *= nmemb;
  hk_string_inplace_concat_chars(str, size, ptr);
  return size;
}

static int32_t init_call(hk_vm_t *vm, hk_value_t *args)
{
  int32_t types[] = {HK_TYPE_NIL, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  CURL *handle = curl_easy_init();
  if (!handle)
  {
    hk_runtime_error("cannot initialize cURL");
    return HK_STATUS_ERROR;
  }
  if (hk_is_string(val))
  {
    hk_string_t *str = hk_as_string(val);
    CURLcode res;
    res = curl_easy_setopt(handle, CURLOPT_URL, str->chars);
    res = curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    if (res != CURLE_OK)
    {
      hk_runtime_error("cannot set option: %s", curl_easy_strerror(res));
      return HK_STATUS_ERROR;
    }
  }
  return hk_vm_push_userdata(vm, (hk_userdata_t *) url_new(handle));
}

static int32_t setopt_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CURL *handle = ((curl_t *) hk_as_userdata(args[1]))->handle;
  int32_t opt = (int32_t) hk_as_float(args[2]);
  hk_string_t *value = hk_as_string(args[3]);
  CURLcode res = curl_easy_setopt(handle, opt, value->chars);
  if (res != CURLE_OK)
  {
    hk_runtime_error("cannot set option: %s", curl_easy_strerror(res));
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_nil(vm);
}

static int32_t cleanup_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  curl_easy_cleanup(((curl_t *) hk_as_userdata(args[1]))->handle);
  return hk_vm_push_nil(vm);
}

static int32_t perform_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CURL *handle = ((curl_t *) hk_as_userdata(args[1]))->handle;
  hk_string_t *str = hk_string_new();
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *) str);
  CURLcode res = curl_easy_perform(handle);
  if (res != CURLE_OK)
  {
    hk_runtime_error("cannot perform: %s", curl_easy_strerror(res));
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_string(vm, str);
}

HK_LOAD_FN(curl)
{
  if (hk_vm_push_string_from_chars(vm, -1, "curl") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "OPT_URL") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_float(vm, CURLOPT_URL) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "OPT_FOLLOWLOCATION") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_float(vm, CURLOPT_FOLLOWLOCATION) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "OPT_POST") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_float(vm, CURLOPT_POST) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "OPT_POSTFIELDS") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_float(vm, CURLOPT_POSTFIELDS) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "init") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "init", 1, &init_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "setopt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "setopt", 3, &setopt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "cleanup") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "cleanup", 1, &cleanup_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "perform") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "perform", 1, &perform_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 8);
}
