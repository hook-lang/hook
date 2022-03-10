//
// Hook Programming Language
// url.c
//

#include "url.h"
#include <stdlib.h>
#include <curl/curl.h>

typedef struct
{
  USERDATA_HEADER
  CURL *curl;
} url_t;

static inline url_t *url_new(CURL *curl);
static void url_deinit(userdata_t *udata);
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data);
static int new_call(vm_t *vm, value_t *args);
static int cleanup_call(vm_t *vm, value_t *args);
static int perform_call(vm_t *vm, value_t *args);

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

static int new_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *url = AS_STRING(args[1]);
  CURL *curl = curl_easy_init();
  if (!curl)
    return vm_push_nil(vm);
  curl_easy_setopt(curl, CURLOPT_URL, url->chars);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  return vm_push_userdata(vm, (userdata_t *) url_new(curl));
}

static int cleanup_call(vm_t *vm, value_t *args)
{
  if (vm_check_userdata(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  curl_easy_cleanup(((url_t *) AS_USERDATA(args[1]))->curl);
  return vm_push_nil(vm);
}

static int perform_call(vm_t *vm, value_t *args)
{
  if (vm_check_userdata(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  CURL *curl = ((url_t *) AS_USERDATA(args[1]))->curl;
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
int __declspec(dllexport) __stdcall load_url(vm_t *vm)
#else
int load_url(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "url") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "new") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "new", 1, &new_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "cleanup") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "cleanup", 1, &cleanup_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "perform") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "perform", 1, &perform_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 3);
}
