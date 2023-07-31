//
// The Hook Programming Language
// curl.c
//

#include "curl.h"
#include <curl/curl.h>
#include <hook/memory.h>
#include <hook/check.h>
#include <hook/status.h>
#include <hook/error.h>

typedef struct
{
  HK_USERDATA_HEADER
  CURL *curl;
  CURLcode res;
  HkString *url;
  HkArray *headers;
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
static int init_call(HkState *state, HkValue *args);
static int setopt_call(HkState *state, HkValue *args);
static int close_call(HkState *state, HkValue *args);
static int exec_call(HkState *state, HkValue *args);
static int errno_call(HkState *state, HkValue *args);
static int error_call(HkState *state, HkValue *args);
static int getinfo_call(HkState *state, HkValue *args);

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
  hk_userdata_init((HkUserdata *) wrapper, &curl_wrapper_deinit);
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

static int init_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_NIL, HK_TYPE_STRING};
  if (hk_check_argument_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkValue val = args[1];
  initialize();
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    deinitialize();
    return hk_state_push_nil(state);
  }
  CURLcode res = CURLE_OK;
  if (hk_is_string(val))
  {
    HkString *str = hk_as_string(val);
    if ((res = curl_easy_setopt(curl, CURLOPT_URL, str->chars)) == CURLE_OK)
      res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  }
  return hk_state_push_userdata(state, (HkUserdata *) curl_wrapper_new(curl, res));
}

static int setopt_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkType types[] = {HK_TYPE_STRING, HK_TYPE_ARRAY};
  if (hk_check_argument_types(args, 3, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  int opt = (int) hk_as_number(args[2]);
  HkValue val = args[3];
  wrapper->res = curl_wrapper_setopt(wrapper, opt, val);
  return hk_state_push_nil(state);
}

static int close_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  bool result = false;
  if (wrapper->curl)
  {
    curl_easy_cleanup(wrapper->curl);
    wrapper->curl = NULL;
    result = true;
  }
  return hk_state_push_bool(state, result);
}

static int exec_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
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
    return hk_state_push_nil(state);
  }
  return hk_state_push_string(state, result);
}

static int errno_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  return hk_state_push_number(state, (double) wrapper->res);
}

static int error_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CurlWrapper *wrapper = (CurlWrapper *) hk_as_userdata(args[1]);
  const char *chars = curl_easy_strerror(wrapper->res);
  return hk_state_push_string_from_chars(state, -1, chars);
}

static int getinfo_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CURL *curl = ((CurlWrapper *) hk_as_userdata(args[1]))->curl;
  int info = (int) hk_as_number(args[2]);
  // TODO: The type of value depends on info
  long value = 0;
  curl_easy_getinfo(curl, info, &value);
  return hk_state_push_number(state, (double) value);
}

HK_LOAD_FN(curl)
{
  if (hk_state_push_string_from_chars(state, -1, "curl") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "OPT_URL") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, CURLOPT_URL) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "OPT_FOLLOWLOCATION") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, CURLOPT_FOLLOWLOCATION) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "OPT_POST") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, CURLOPT_POST) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "OPT_POSTFIELDS") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, CURLOPT_POSTFIELDS) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "OPT_HTTPHEADER") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, CURLOPT_HTTPHEADER) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "INFO_RESPONSE_CODE") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, CURLINFO_RESPONSE_CODE) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "init") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "init", 1, &init_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "setopt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "setopt", 3, &setopt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "close") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "close", 1, &close_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "exec") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "exec", 1, &exec_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "errno") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "errno", 1, &errno_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "error") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "error", 1, &error_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "getinfo") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "getinfo", 2, &getinfo_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 13);
}
