//
// Hook Programming Language
// hook_io.c
//

#include "hook_io.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

typedef struct
{
  HK_USERDATA_HEADER
  FILE *stream;
} file_t;

static inline file_t *file_new(FILE *stream);
static void file_deinit(hk_userdata_t *udata);
static int open_call(hk_vm_t *vm, hk_value_t *args);
static int close_call(hk_vm_t *vm, hk_value_t *args);
static int popen_call(hk_vm_t *vm, hk_value_t *args);
static int pclose_call(hk_vm_t *vm, hk_value_t *args);
static int eof_call(hk_vm_t *vm, hk_value_t *args);
static int flush_call(hk_vm_t *vm, hk_value_t *args);
static int sync_call(hk_vm_t *vm, hk_value_t *args);
static int tell_call(hk_vm_t *vm, hk_value_t *args);
static int rewind_call(hk_vm_t *vm, hk_value_t *args);
static int seek_call(hk_vm_t *vm, hk_value_t *args);
static int read_call(hk_vm_t *vm, hk_value_t *args);
static int write_call(hk_vm_t *vm, hk_value_t *args);
static int readln_call(hk_vm_t *vm, hk_value_t *args);
static int writeln_call(hk_vm_t *vm, hk_value_t *args);

static inline file_t *file_new(FILE *stream)
{
  file_t *file = (file_t *) hk_allocate(sizeof(*file));
  hk_userdata_init((hk_userdata_t *) file, &file_deinit);
  file->stream = stream;
  return file;
}

static void file_deinit(hk_userdata_t *udata)
{
  FILE *stream = ((file_t *) udata)->stream;
  if (stream == stdin || stream == stdout || stream == stderr)
    return;
  fclose(stream);
}

static int open_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *filename = hk_as_string(args[1]);
  hk_string_t *mode = hk_as_string(args[2]);
  FILE *stream = fopen(filename->chars, mode->chars);
  if (!stream)
    return hk_vm_push_nil(vm);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) file_new(stream));
}

static int close_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, fclose(((file_t *) hk_as_userdata(args[1]))->stream));
}

static int popen_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *command = hk_as_string(args[1]);
  hk_string_t *mode = hk_as_string(args[2]);
  FILE *stream;
  stream = popen(command->chars, mode->chars);
  if (!stream)
    return hk_vm_push_nil(vm);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) file_new(stream));
}

static int pclose_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  int status;
  status = pclose(stream);
  return hk_vm_push_number(vm, status);
}

static int eof_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  return hk_vm_push_boolean(vm, (bool) feof(stream));
}

static int flush_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  return hk_vm_push_number(vm, fflush(stream));
}

static int sync_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  int fd = fileno(stream);
  bool result;
#ifdef _WIN32
  result = FlushFileBuffers(fd);
#else
  result = !fsync(fd);
#endif
  return hk_vm_push_boolean(vm, result);
}

static int tell_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  return hk_vm_push_number(vm, ftell(stream));
}

static int rewind_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  rewind(stream);
  return hk_vm_push_nil(vm);
}

static int seek_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_integer(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  long offset = (long) args[2].as.number;
  int whence = (int) args[3].as.number;
  return hk_vm_push_number(vm, fseek(stream, offset, whence));
}

static int read_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_integer(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  long size = (long) args[2].as.number;
  hk_string_t *str = hk_string_allocate(size);
  int length = (int) fread(str->chars, 1, size, stream);
  if (length < size && !feof(stream))
  {
    hk_string_free(str);
    return hk_vm_push_nil(vm);
  }
  str->length = length;
  str->chars[length] = '\0';
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int write_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  hk_string_t *str = hk_as_string(args[2]);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size)
    return hk_vm_push_nil(vm);
  return hk_vm_push_number(vm, size);
}

static int readln_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  return hk_vm_push_string_from_stream(vm, stream, '\n');
}

static int writeln_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((file_t *) hk_as_userdata(args[1]))->stream;
  hk_string_t *str = hk_as_string(args[2]);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size || fwrite("\n", 1, 1, stream) < 1)
    return hk_vm_push_nil(vm);
  return hk_vm_push_number(vm, size + 1);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_io(hk_vm_t *vm)
#else
int load_io(hk_vm_t *vm)
#endif
{
  if (hk_vm_push_string_from_chars(vm, -1, "io") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "StdIn") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_userdata(vm, (hk_userdata_t *) file_new(stdin)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "StdOut") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_userdata(vm, (hk_userdata_t *) file_new(stdout)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "StdErr") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_userdata(vm, (hk_userdata_t *) file_new(stderr)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "SeekSet") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, SEEK_SET) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "SeekCurrent") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, SEEK_CUR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "SeeEnd") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, SEEK_END) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "open") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "open", 2, &open_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "close") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "close", 1, &close_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "popen") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "popen", 2, &popen_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "pclose") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "pclose", 1, &pclose_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "eof") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "eof", 1, &eof_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "flush") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "flush", 1, &flush_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sync") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sync", 1, &sync_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "tell") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "tell", 1, &tell_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "rewind") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "rewind", 1, &rewind_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "seek") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "seek", 3, &seek_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "read") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "read", 2, &read_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "write") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "write", 2, &write_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "readln") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "readln", 1, &readln_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "writeln") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "writeln", 2, &writeln_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 20);
}
