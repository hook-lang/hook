//
// The Hook Programming Language
// hk_utils.c
//

#include "hk_utils.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#ifdef __linux__
  #include <linux/limits.h>
#endif

#ifdef __APPLE__
  #include <sys/syslimits.h>
#endif

#ifdef _WIN32
  #define PATH_MAX MAX_PATH
  #define mkdir _mkdir
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
  mkdir(path, 0777); 
}

int32_t hk_power_of_two_ceil(int32_t n)
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

bool hk_double_from_chars(double *result, const char *chars)
{
  errno = 0;
  double _result = strtod(chars, NULL);
  if (errno == ERANGE)
    return false;
  *result = _result;
  return true;
}

void hk_copy_cstring(char *dest, const char *src, int32_t max_len)
{
#ifdef _WIN32
  strncpy_s(dest, max_len, src, _TRUNCATE);
#else
  strncpy(dest, src, max_len);
#endif
}
