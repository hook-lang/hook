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
  CURL *curl;
} curl_wrapper_t;

static inline curl_wrapper_t *curl_wrapper_new(CURL *curl);
static void curl_wrapper_deinit(hk_userdata_t *udata);
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data);
static int32_t init_call(hk_state_t *state, hk_value_t *args);
static int32_t setopt_call(hk_state_t *state, hk_value_t *args);
static int32_t cleanup_call(hk_state_t *state, hk_value_t *args);
static int32_t perform_call(hk_state_t *state, hk_value_t *args);

static inline curl_wrapper_t *curl_wrapper_new(CURL *curl)
{
  curl_wrapper_t *wrapper = (curl_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &curl_wrapper_deinit);
  wrapper->curl = curl;
  return wrapper;
}

static void curl_wrapper_deinit(hk_userdata_t *udata)
{
  curl_easy_cleanup(((curl_wrapper_t *) udata)->curl);
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data)
{
  hk_string_t *str = (hk_string_t *) data;
  size *= nmemb;
  hk_string_inplace_concat_chars(str, size, ptr);
  return size;
}

static int32_t init_call(hk_state_t *state, hk_value_t *args)
{
  hk_type_t types[] = {HK_TYPE_NIL, HK_TYPE_STRING};
  if (hk_check_argument_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    hk_runtime_error("cannot initialize cURL");
    return HK_STATUS_ERROR;
  }
  if (hk_is_string(val))
  {
    hk_string_t *str = hk_as_string(val);
    CURLcode res;
    res = curl_easy_setopt(curl, CURLOPT_URL, str->chars);
    res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (res != CURLE_OK)
    {
      hk_runtime_error("cannot set option: %s", curl_easy_strerror(res));
      return HK_STATUS_ERROR;
    }
  }
  return hk_state_push_userdata(state, (hk_userdata_t *) curl_wrapper_new(curl));
}

static int32_t setopt_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CURL *curl = ((curl_wrapper_t *) hk_as_userdata(args[1]))->curl;
  int32_t opt = (int32_t) hk_as_number(args[2]);
  hk_string_t *value = hk_as_string(args[3]);
  CURLcode res = curl_easy_setopt(curl, opt, value->chars);
  if (res != CURLE_OK)
  {
    hk_runtime_error("cannot set option: %s", curl_easy_strerror(res));
    return HK_STATUS_ERROR;
  }
  return hk_state_push_nil(state);
}

static int32_t cleanup_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  curl_easy_cleanup(((curl_wrapper_t *) hk_as_userdata(args[1]))->curl);
  return hk_state_push_nil(state);
}

static int32_t perform_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CURL *curl = ((curl_wrapper_t *) hk_as_userdata(args[1]))->curl;
  hk_string_t *str = hk_string_new();
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
