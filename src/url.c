//
// Hook Programming Language
// url.c
//

#include "url.h"
#include <curl/curl.h>
#include "memory.h"
#include "common.h"
#include "error.h"

typedef struct
{
  USERDATA_HEADER
  CURL *curl;
} url_t;

static inline url_t *url_new(CURL *curl);
static void url_deinit(userdata_t *udata);
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data);
static int new_call(vm_t *vm, value_t *frame);
static int cleanup_call(vm_t *vm, value_t *frame);
static int perform_call(vm_t *vm, value_t *frame);

static inline url_t *url_new(CURL *curl)
{
  url_t *url = (url_t *) allocate(sizeof(*url));
  userdata_init((userdata_t *) curl, &url_deinit);
  url->curl = curl;
  return url;
}

static void url_deinit(userdata_t *udata)
{
  curl_easy_cleanup(((url_t *) udata)->curl);
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data)
{
  string_t *str = (string_t *) data;
  size *= nmemb;
  string_inplace_concat_chars(str, size, ptr);
  return size;
}

static int new_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  string_t *url = AS_STRING(val);
  CURL *curl = curl_easy_init();
  if (!curl)
    vm_push_null(vm);
  curl_easy_setopt(curl, CURLOPT_URL, url->chars);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  return vm_push_userdata(vm, (userdata_t *) url_new(curl));
}

static int cleanup_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  curl_easy_cleanup(((url_t *) AS_USERDATA(val))->curl);
  return vm_push_null(vm);
}

static int perform_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  CURL *curl = ((url_t *) AS_USERDATA(val))->curl;
  string_t *str = string_new(0);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) str);
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK)
  {
    runtime_error("cannot perform: %s", curl_easy_strerror(res));
    string_free(str);
    return STATUS_ERROR;
  }
  return vm_push_string(vm, str);
}

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_url(vm_t *vm)
#else
void load_url(vm_t *vm)
#endif
{
  vm_push_string(vm, string_from_chars(-1, "url"));
  vm_push_string(vm, string_from_chars(-1, "new"));
  vm_push_native(vm, native_new(string_from_chars(-1, "new"), 1, &new_call));
  vm_push_string(vm, string_from_chars(-1, "cleanup"));
  vm_push_native(vm, native_new(string_from_chars(-1, "cleanup"), 1, &cleanup_call));
  vm_push_string(vm, string_from_chars(-1, "perform"));
  vm_push_native(vm, native_new(string_from_chars(-1, "perform"), 1, &perform_call));
  vm_construct(vm, 3);
}
