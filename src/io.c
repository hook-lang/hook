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
static int open_call(vm_t *vm, value_t *frame);
static int close_call(vm_t *vm, value_t *frame);
static int popen_call(vm_t *vm, value_t *frame);
static int pclose_call(vm_t *vm, value_t *frame);
static int eof_call(vm_t *vm, value_t *frame);
static int flush_call(vm_t *vm, value_t *frame);
static int sync_call(vm_t *vm, value_t *frame);
static int tell_call(vm_t *vm, value_t *frame);
static int rewind_call(vm_t *vm, value_t *frame);
static int seek_call(vm_t *vm, value_t *frame);
static int read_call(vm_t *vm, value_t *frame);
static int write_call(vm_t *vm, value_t *frame);
static int readln_call(vm_t *vm, value_t *frame);
static int writeln_call(vm_t *vm, value_t *frame);

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

static int open_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_STRING(val1))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  string_t *filename = AS_STRING(val1);
  string_t *mode = AS_STRING(val2);
  FILE *stream = fopen(filename->chars, mode->chars);
  if (!stream)
    vm_push_null(vm);
  return vm_push_userdata(vm, (userdata_t *) file_new(stream));
}

static int close_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, fclose(((file_t *) AS_USERDATA(val))->stream));
}

static int popen_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_STRING(val1))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  string_t *command = AS_STRING(val1);
  string_t *mode = AS_STRING(val2);
  FILE *stream;
  stream = popen(command->chars, mode->chars);
  if (!stream)
    vm_push_null(vm);
  return vm_push_userdata(vm, (userdata_t *) file_new(stream));
}

static int pclose_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  int status;
  status = pclose(stream);
  return vm_push_number(vm, status);
}

static int eof_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  return vm_push_boolean(vm, (bool) feof(stream));
}

static int flush_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  return vm_push_number(vm, fflush(stream));
}

static int sync_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
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

static int tell_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  return vm_push_number(vm, ftell(stream));
}

static int rewind_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  rewind(stream);
  return vm_push_null(vm);
}

static int seek_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  value_t val3 = frame[3];
  if (!IS_USERDATA(val1))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("invalid type: expected integer but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val3))
  {
    runtime_error("invalid type: expected integer but got `%s`", type_name(val3.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val1))->stream;
  long offset = (long) val2.as.number;
  int whence = (int) val3.as.number;
  return vm_push_number(vm, fseek(stream, offset, whence));
}

static int read_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_USERDATA(val1))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("invalid type: expected integer but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val1))->stream;
  long size = (long) val2.as.number;
  string_t *str = string_allocate(size);
  int length = (int) fread(str->chars, 1, size, stream);
  if (length < size && !feof(stream))
  {
    string_free(str);
    return vm_push_null(vm);
  }
  str->length = length;
  return vm_push_string(vm, str);
}

static int write_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_USERDATA(val1))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val1))->stream;
  string_t *str = AS_STRING(val2);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size)
    return vm_push_null(vm);
  return vm_push_number(vm, size);
}

static int readln_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val))->stream;
  string_t *str = string_from_stream(stream, '\n');
  return vm_push_string(vm, str);
}

static int writeln_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_USERDATA(val1))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  FILE *stream = ((file_t *) AS_USERDATA(val1))->stream;
  string_t *str = AS_STRING(val2);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size || fwrite("\n", 1, 1, stream) < 1)
    return vm_push_null(vm);
  return vm_push_number(vm, size + 1);
}

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_io(vm_t *vm)
#else
void load_io(vm_t *vm)
#endif
{
  vm_push_string(vm, string_from_chars(-1, "io"));
  vm_push_string(vm, string_from_chars(-1, "StdIn"));
  vm_push_userdata(vm, (userdata_t *) file_new(stdin));
  vm_push_string(vm, string_from_chars(-1, "StdOut"));
  vm_push_userdata(vm, (userdata_t *) file_new(stdout));
  vm_push_string(vm, string_from_chars(-1, "StdErr"));
  vm_push_userdata(vm, (userdata_t *) file_new(stderr));
  vm_push_string(vm, string_from_chars(-1, "SeekSet"));
  vm_push_number(vm, SEEK_SET);
  vm_push_string(vm, string_from_chars(-1, "SeekCurrent"));
  vm_push_number(vm, SEEK_CUR);
  vm_push_string(vm, string_from_chars(-1, "SeeEnd"));
  vm_push_number(vm, SEEK_END);
  vm_push_string(vm, string_from_chars(-1, "open"));
  vm_push_native(vm, native_new(string_from_chars(-1, "open"), 2, &open_call));
  vm_push_string(vm, string_from_chars(-1, "close"));
  vm_push_native(vm, native_new(string_from_chars(-1, "close"), 1, &close_call));
  vm_push_string(vm, string_from_chars(-1, "popen"));
  vm_push_native(vm, native_new(string_from_chars(-1, "popen"), 2, &popen_call));
  vm_push_string(vm, string_from_chars(-1, "pclose"));
  vm_push_native(vm, native_new(string_from_chars(-1, "pclose"), 1, &pclose_call));
  vm_push_string(vm, string_from_chars(-1, "eof"));
  vm_push_native(vm, native_new(string_from_chars(-1, "eof"), 1, &eof_call));
  vm_push_string(vm, string_from_chars(-1, "flush"));
  vm_push_native(vm, native_new(string_from_chars(-1, "flush"), 1, &flush_call));
  vm_push_string(vm, string_from_chars(-1, "sync"));
  vm_push_native(vm, native_new(string_from_chars(-1, "sync"), 1, &sync_call));
  vm_push_string(vm, string_from_chars(-1, "tell"));
  vm_push_native(vm, native_new(string_from_chars(-1, "tell"), 1, &tell_call));
  vm_push_string(vm, string_from_chars(-1, "rewind"));
  vm_push_native(vm, native_new(string_from_chars(-1, "rewind"), 1, &rewind_call));
  vm_push_string(vm, string_from_chars(-1, "seek"));
  vm_push_native(vm, native_new(string_from_chars(-1, "seek"), 3, &seek_call));
  vm_push_string(vm, string_from_chars(-1, "read"));
  vm_push_native(vm, native_new(string_from_chars(-1, "read"), 2, &read_call));
  vm_push_string(vm, string_from_chars(-1, "write"));
  vm_push_native(vm, native_new(string_from_chars(-1, "write"), 2, &write_call));
  vm_push_string(vm, string_from_chars(-1, "readln"));
  vm_push_native(vm, native_new(string_from_chars(-1, "readln"), 1, &readln_call));
  vm_push_string(vm, string_from_chars(-1, "writeln"));
  vm_push_native(vm, native_new(string_from_chars(-1, "writeln"), 2, &writeln_call));
  vm_construct(vm, 20);
}
