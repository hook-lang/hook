//
// The Hook Programming Language
// url.c
//

#include "url.h"
#include <stdlib.h>
#include <curl/curl.h>
#include "hk_memory.h"
#include "hk_status.h"
#include "hk_error.h"

typedef struct
{
  HK_USERDATA_HEADER
  CURL *curl;
} url_t;

static inline url_t *url_new(CURL *curl);
static void url_deinit(hk_userdata_t *udata);
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data);
static int32_t new_call(hk_vm_t *vm, hk_value_t *args);
static int32_t cleanup_call(hk_vm_t *vm, hk_value_t *args);
static int32_t perform_call(hk_vm_t *vm, hk_value_t *args);

static inline url_t *url_new(CURL *curl)
{
  url_t *url = (url_t *) hk_allocate(sizeof(*url));
  hk_userdata_init((hk_userdata_t *) curl, &url_deinit);
  url->curl = curl;
  return url;
}

static void url_deinit(hk_userdata_t *udata)
{
  curl_easy_cleanup(((url_t *) udata)->curl);
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data)
{
  hk_string_t *str = (hk_string_t *) data;
  size *= nmemb;
  hk_string_inplace_concat_chars(str, size, ptr);
  return size;
}

static int32_t new_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *url = hk_as_string(args[1]);
  CURL *curl = curl_easy_init();
  if (!curl)
    return hk_vm_push_nil(vm);
  curl_easy_setopt(curl, CURLOPT_URL, url->chars);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) url_new(curl));
}

static int32_t cleanup_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  curl_easy_cleanup(((url_t *) hk_as_userdata(args[1]))->curl);
  return hk_vm_push_nil(vm);
}

static int32_t perform_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  CURL *curl = ((url_t *) hk_as_userdata(args[1]))->curl;
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
  return hk_vm_push_string(vm, str);
}

HK_LOAD_FN(url)
{
  if (hk_vm_push_string_from_chars(vm, -1, "url") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "new") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "new", 1, &new_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "cleanup") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "cleanup", 1, &cleanup_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "perform") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "perform", 1, &perform_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 3);
}
