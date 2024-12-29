//
// curl.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "curl.h"
#include <curl/curl.h>

typedef struct
{
  HK_USERDATA_HEADER
  CURL     *curl;
  CURLcode res;
  HkString *url;
  HkArray  *headers;
} CurlWrapper;

static int initialized = 0;

static inline void initialize(void);
static inline void deinitialize(void);
static inline CurlWrapper *curl_wrapper_new(CURL *curl, CURLcode res);
static inline CURLcode curl_wrapper_setopt(CurlWrapper *wrapper, int opt, HkValue val);
static inline void curl_wrapper_setopt_url(CurlWrapper *wrapper, HkString *url);
static inline void curl_wrapper_setopt_headers(CurlWrapper *wrapper, HkArray *headers);
static inline struct curl_slist *array_to_slist(HkArray *arr);
static void curl_wrapper_deinit(HkUserdata *udata);
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data);
static void init_call(HkVM *vm, HkValue *args);
static void setopt_call(HkVM *vm, HkValue *args);
static void close_call(HkVM *vm, HkValue *args);
static void exec_call(HkVM *vm, HkValue *args);
static void errno_call(HkVM *vm, HkValue *args);
static void error_call(HkVM *vm, HkValue *args);
static void getinfo_call(HkVM *vm, HkValue *args);

static inline void initialize(void)
{
  if (initialized)
  {
    ++initialized;
    return;
  }
  curl_global_init(CURL_GLOBAL_ALL);
  initialized = 1;
}

static inline void deinitialize(void)
{
  --initialized;
  if (initialized)
    return;
  curl_global_cleanup();
}

static inline CurlWrapper *curl_wrapper_new(CURL *curl, CURLcode res)
{
  CurlWrapper *wrapper = (CurlWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, curl_wrapper_deinit);
  wrapper->curl = curl;
  wrapper->res = res;
  wrapper->url = NULL;
  wrapper->headers = NULL;
  return wrapper;
}

static inline CURLcode curl_wrapper_setopt(CurlWrapper *wrapper, int opt, HkValue val)
{
  if (opt == CURLOPT_URL)
  {
    // TODO: Check if val is an array
    curl_wrapper_setopt_url(wrapper, hk_as_string(val));
    return CURLE_OK;
  }
  if (opt == CURLOPT_HTTPHEADER)
  {
    // TODO: Check if val is an array
    curl_wrapper_setopt_headers(wrapper, hk_as_array(val));
    return CURLE_OK;
  }
  // TODO: Is default case long?
  long num = hk_is_int(val) ? (long) hk_as_number(val) : 0;
  return curl_easy_setopt(wrapper->curl, opt, num);
}

static inline void curl_wrapper_setopt_url(CurlWrapper *wrapper, HkString *url)
{
  HkString *_url = wrapper->url;
  if (_url)
    hk_string_release(_url);
  hk_incr_ref(url);
  wrapper->url = url;
}

static inline void curl_wrapper_setopt_headers(CurlWrapper *wrapper, HkArray *headers)
{
  HkArray *_headers = wrapper->headers;
  if (_headers)
    hk_array_release(_headers);
  hk_incr_ref(headers);
  wrapper->headers = headers;
}

static inline struct curl_slist *array_to_slist(HkArray *arr)
{
  struct curl_slist *list = NULL;
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    char *chars = elem.type == HK_TYPE_STRING ? hk_as_string(elem)->chars : "";
    list = curl_slist_append(list, chars);
  }
  return list;
}

static void curl_wrapper_deinit(HkUserdata *udata)
{
  CurlWrapper *wrapper = (CurlWrapper *) udata;
  CURL *curl = wrapper->curl;
  if (curl)
    curl_easy_cleanup(curl);
  HkString *url = wrapper->url;
  if (url)
    hk_string_release(url);
  HkArray *headers = wrapper->headers;
  if (headers)
    hk_array_release(headers);
  deinitialize();
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data)
{
  HkString *str = (HkString *) data;
  size *= nmemb;
  hk_string_inplace_concat_chars(str, size, ptr);
  return size;
}

static void init_call(HkVM *vm, HkValue *args)
{
  HkType types[] = { HK_TYPE_NIL, HK_TYPE_STRING };
  hk_vm_check_argument_types(vm, args, 1, 2, types);
  hk_return_if_not_ok(vm);
  HkValue val = args[1];
  initialize();
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    deinitialize();
    hk_vm_push_nil(vm);
    return;
  }
  CURLcode res = CURLE_OK;
  if (hk_is_string(val))
  {
    HkString *str = hk_as_string(val);
    if ((res = curl_easy_setopt(curl, CURLOPT_URL, str->chars)) == CURLE_OK)
      res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  }
  hk_vm_push_userdata(vm, (HkUserdata *) curl_wrapper_new(curl, res));
}

static void setopt_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkType types[] = { HK_TYPE_STRING, HK_TYPE_ARRAY };
  hk_vm_check_argument_types(vm, args, 3, 2, types);
  hk_return_if_not_ok(vm);
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  int opt = (int) hk_as_number(args[2]);
  HkValue val = args[3];
  wrapper->res = curl_wrapper_setopt(wrapper, opt, val);
  hk_vm_push_nil(vm);
}

static void close_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  bool result = false;
  if (wrapper->curl)
  {
    curl_easy_cleanup(wrapper->curl);
    wrapper->curl = NULL;
    result = true;
  }
  hk_vm_push_bool(vm, result);
}

static void exec_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  CURL *curl = wrapper->curl;
  HkString *url = wrapper->url;
  HkArray *headers = wrapper->headers;
  if (url)
    curl_easy_setopt(curl, CURLOPT_URL, url->chars);
  struct curl_slist *list = NULL;
  if (headers)
  {
    list = array_to_slist(headers);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
  }
  HkString *result = hk_string_new();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) result);
  CURLcode res = curl_easy_perform(curl);
  if (list)
    curl_slist_free_all(list);
  wrapper->res = res;
  if (res != CURLE_OK)
  {
    hk_string_free(result);
    hk_vm_push_nil(vm);
    return;
  }
  hk_vm_push_string(vm, result);
}

static void errno_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  hk_vm_push_number(vm, (double) wrapper->res);
}

static void error_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  const char *chars = curl_easy_strerror(wrapper->res);
  hk_vm_push_string_from_chars(vm, -1, chars);
}

static void getinfo_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  CURL *curl = ((CurlWrapper *) hk_as_userdata(args[1]))->curl;
  int info = (int) hk_as_number(args[2]);
  // TODO: The type of value depends on info
  long value = 0;
  curl_easy_getinfo(curl, info, &value);
  hk_vm_push_number(vm, (double) value);
}

HK_LOAD_MODULE_HANDLER(curl)
{
  hk_vm_push_string_from_chars(vm, -1, "curl");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "OPT_URL");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, CURLOPT_URL);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "OPT_FOLLOWLOCATION");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, CURLOPT_FOLLOWLOCATION);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "OPT_POST");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, CURLOPT_POST);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "OPT_POSTFIELDS");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, CURLOPT_POSTFIELDS);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "OPT_HTTPHEADER");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, CURLOPT_HTTPHEADER);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "INFO_RESPONSE_CODE");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, CURLINFO_RESPONSE_CODE);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "init");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "init", 1, init_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "setopt");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "setopt", 3, setopt_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "close");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "close", 1, close_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "exec");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "exec", 1, exec_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "errno");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "errno", 1, errno_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "error");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "error", 1, error_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "getinfo");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "getinfo", 2, getinfo_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 13);
}
