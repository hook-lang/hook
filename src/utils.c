//
// utils.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <hook/utils.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <hook/memory.h>

#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>
#else
  #include <sys/stat.h>
  #include <limits.h>
#endif

#ifdef __linux__
  #include <linux/limits.h>
#endif

#ifdef __APPLE__
  #include <sys/syslimits.h>
#endif

#ifdef _WIN32
  #define PATH_MAX MAX_PATH
#endif

static void make_directory(char *path);

static void make_directory(char *path)
{
  char *sep = strrchr(path, '/');
  if (sep)
  {
    *sep = '\0';
    make_directory(path);
    *sep = '/';
  }
#ifdef _WIN32
  _mkdir(path);
#else
  mkdir(path, 0777); 
#endif
}

int hk_power_of_two_ceil(int n)
{
  --n;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  ++n;  
  return n;
}

void hk_ensure_path(const char *filename)
{
  char *sep = strrchr(filename, '/');
  if (sep)
  {
    char path[PATH_MAX + 1];
    hk_copy_cstring(path, filename, PATH_MAX);
    path[sep - filename] = '\0';
    make_directory(path);
  }
}

bool hk_long_from_chars(long *result, const char *chars)
{
  errno = 0;
  char *end = (char *) chars;
  long _result = strtol(chars, &end, 10);
  if (errno == ERANGE)
    return false;
  if (*end)
    return false;
  *result = _result;
  return true;
}

bool hk_double_from_chars(double *result, const char *chars, bool strict)
{
  errno = 0;
  char *end;
  double _result = strtod(chars, &end);
  if (errno == ERANGE)
    return false;
  if (strict && *end)
    return false;
  *result = _result;
  return true;
}

void hk_copy_cstring(char *dest, const char *src, int max_len)
{
#ifdef _WIN32
  strncpy_s(dest, max_len + 1, src, _TRUNCATE);
#else
  strncpy(dest, src, max_len);
  dest[max_len] = '\0';
#endif
}

char *hk_duplicate_cstring(const char *str)
{
  int length = (int) strnlen(str, INT_MAX);
  char *_str = (char *) hk_allocate(length + 1);
  memcpy(_str, str, length);
  _str[length] = '\0';
  return _str;
}
