//
// Hook Programming Language
// io.c
//

#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "common.h"
#include "memory.h"
#include "error.h"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

typedef struct
{
  USERDATA_HEADER
  FILE *stream;
} file_t;

static inline file_t *file_new(FILE *stream);
static void file_deinit(userdata_t *udata);
static int open_call(vm_t *vm, value_t *args);
static int close_call(vm_t *vm, value_t *args);
static int popen_call(vm_t *vm, value_t *args);
static int pclose_call(vm_t *vm, value_t *args);
static int eof_call(vm_t *vm, value_t *args);
static int flush_call(vm_t *vm, value_t *args);
static int sync_call(vm_t *vm, value_t *args);
static int tell_call(vm_t *vm, value_t *args);
static int rewind_call(vm_t *vm, value_t *args);
static int seek_call(vm_t *vm, value_t *args);
static int read_call(vm_t *vm, value_t *args);
static int write_call(vm_t *vm, value_t *args);
static int readln_call(vm_t *vm, value_t *args);
static int writeln_call(vm_t *vm, value_t *args);

static inline file_t *file_new(FILE *stream)
{
  file_t *file = (file_t *) allocate(sizeof(*file));
  userdata_init((userdata_t *) file, &file_deinit);
  file->stream = stream;
  return file;
}

static void file_deinit(userdata_t *udata)
{
  FILE *stream = ((file_t *) udata)->stream;
  if (stream == stdin || stream == stdout || stream == stderr)
    return;
  fclose(stream);
}

static int open_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_STRING(val1))
  {
    runtime_error("type error: expected string but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("type error: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  string_t *filename = AS_STRING(val1);
  string_t *mode = AS_STRING(val2);
  FILE *stream = fopen(filename->chars, mode->chars);
  if (!stream)
    return vm_push_nil(vm);
  return vm_push_userdata(vm, (userdata_t *) file_new(stream));
}

static int close_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, fclose(((file_t *) AS_USERDATA(val))->stream));
}

static int popen_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_STRING(val1))
  {
    runtime_error("type error: expected string but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("type error: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  string_t *command = AS_STRING(val1);
  string_t *mode = AS_STRING(val2);
  FILE *stream;
  stream = popen(command->chars, mode->chars);
  if (!stream)
    return vm_push_nil(vm);
  return vm_push_userdata(vm, (userdata_t *) file_new(stream));
}

static int pclose_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  int status;
  status = pclose(stream);
  return vm_push_number(vm, status);
}

static int eof_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  return vm_push_boolean(vm, (bool) feof(stream));
}

static int flush_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  return vm_push_number(vm, fflush(stream));
}

static int sync_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  int fd = fileno(stream);
  bool result;
#ifdef _WIN32
  result = FlushFileBuffers(fd);
#else
  result = !fsync(fd);
#endif
  return vm_push_boolean(vm, result);
}

static int tell_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  return vm_push_number(vm, ftell(stream));
}

static int rewind_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  rewind(stream);
  return vm_push_nil(vm);
}

static int seek_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  value_t val3 = args[3];
  if (!IS_USERDATA(val1))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("type error: expected integer but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val3))
  {
    runtime_error("type error: expected integer but got `%s`", type_name(val3.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val1))->stream;
  long offset = (long) val2.as.number;
  int whence = (int) val3.as.number;
  return vm_push_number(vm, fseek(stream, offset, whence));
}

static int read_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_USERDATA(val1))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("type error: expected integer but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val1))->stream;
  long size = (long) val2.as.number;
  string_t *str = string_allocate(size);
  int length = (int) fread(str->chars, 1, size, stream);
  if (length < size && !feof(stream))
  {
    string_free(str);
    return vm_push_nil(vm);
  }
  str->length = length;
  return vm_push_string(vm, str);
}

static int write_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_USERDATA(val1))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("type error: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val1))->stream;
  string_t *str = AS_STRING(val2);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size)
    return vm_push_nil(vm);
  return vm_push_number(vm, size);
}

static int readln_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  return vm_push_string_from_stream(vm, stream, '\n');
}

static int writeln_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_USERDATA(val1))
  {
    runtime_error("type error: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("type error: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val1))->stream;
  string_t *str = AS_STRING(val2);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size || fwrite("\n", 1, 1, stream) < 1)
    return vm_push_nil(vm);
  return vm_push_number(vm, size + 1);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_io(vm_t *vm)
#else
int load_io(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "io") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "StdIn") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_userdata(vm, (userdata_t *) file_new(stdin)) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "StdOut") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_userdata(vm, (userdata_t *) file_new(stdout)) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "StdErr") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_userdata(vm, (userdata_t *) file_new(stderr)) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "SeekSet") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_number(vm, SEEK_SET) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "SeekCurrent") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_number(vm, SEEK_CUR) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "SeeEnd") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_number(vm, SEEK_END) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "open") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "open", 2, &open_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "close") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "close", 1, &close_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "popen") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "popen", 2, &popen_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "pclose") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "pclose", 1, &pclose_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "eof") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "eof", 1, &eof_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "flush") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "flush", 1, &flush_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "sync") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sync", 1, &sync_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "tell") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "tell", 1, &tell_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "rewind") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "rewind", 1, &rewind_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "seek") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "seek", 3, &seek_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "read") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "read", 2, &read_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "write") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "write", 2, &write_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "readln") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "readln", 1, &readln_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "writeln") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "writeln", 2, &writeln_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 20);
}
