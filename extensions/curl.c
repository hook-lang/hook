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
} CurlWrapper;

static inline CurlWrapper *curl_wrapper_new(CURL *curl);
static void curl_wrapper_deinit(HkUserdata *udata);
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data);
static int init_call(HkState *state, HkValue *args);
static int setopt_call(HkState *state, HkValue *args);
static int cleanup_call(HkState *state, HkValue *args);
static int perform_call(HkState *state, HkValue *args);

static inline CurlWrapper *curl_wrapper_new(CURL *curl)
{
  CurlWrapper *wrapper = (CurlWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &curl_wrapper_deinit);
  wrapper->curl = curl;
  return wrapper;
}

static void curl_wrapper_deinit(HkUserdata *udata)
{
  curl_easy_cleanup(((CurlWrapper *) udata)->curl);
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
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    hk_runtime_error("cannot initialize cURL");
    return HK_STATUS_ERROR;
  }
  if (hk_is_string(val))
  {
    HkString *str = hk_as_string(val);
    CURLcode res;
    res = curl_easy_setopt(curl, CURLOPT_URL, str->chars);
    res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (res != CURLE_OK)
    {
      hk_runtime_error("cannot set option: %s", curl_easy_strerror(res));
      return HK_STATUS_ERROR;
    }
  }
  return hk_state_push_userdata(state, (HkUserdata *) curl_wrapper_new(curl));
}

static int setopt_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CURL *curl = ((CurlWrapper *) hk_as_userdata(args[1]))->curl;
  int opt = (int) hk_as_number(args[2]);
  HkString *value = hk_as_string(args[3]);
  CURLcode res = curl_easy_setopt(curl, opt, value->chars);
  if (res != CURLE_OK)
  {
    hk_runtime_error("cannot set option: %s", curl_easy_strerror(res));
    return HK_STATUS_ERROR;
  }
  return hk_state_push_nil(state);
}

static int cleanup_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  curl_easy_cleanup(((CurlWrapper *) hk_as_userdata(args[1]))->curl);
  return hk_state_push_nil(state);
}

static int perform_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CURL *curl = ((CurlWrapper *) hk_as_userdata(args[1]))->curl;
  HkString *str = hk_string_new();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) str);
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK)
  {
    hk_runtime_error("cannot perform: %s", curl_easy_strerror(res));
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return hk_state_push_string(state, str);
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
  if (hk_state_push_string_from_chars(state, -1, "init") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "init", 1, &init_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "setopt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "setopt", 3, &setopt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "cleanup") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "cleanup", 1, &cleanup_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "perform") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "perform", 1, &perform_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 8);
}
