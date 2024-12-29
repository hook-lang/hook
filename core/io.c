//
// io.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "io.h"
#include <stdio.h>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
#endif

#ifndef _WIN32
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
} File;

static inline File *file_new(FILE *stream);
static void file_deinit(HkUserdata *udata);
static void open_call(HkVM *vm, HkValue *args);
static void close_call(HkVM *vm, HkValue *args);
static void popen_call(HkVM *vm, HkValue *args);
static void pclose_call(HkVM *vm, HkValue *args);
static void eof_call(HkVM *vm, HkValue *args);
static void flush_call(HkVM *vm, HkValue *args);
static void sync_call(HkVM *vm, HkValue *args);
static void tell_call(HkVM *vm, HkValue *args);
static void rewind_call(HkVM *vm, HkValue *args);
static void seek_call(HkVM *vm, HkValue *args);
static void read_call(HkVM *vm, HkValue *args);
static void write_call(HkVM *vm, HkValue *args);
static void readln_call(HkVM *vm, HkValue *args);
static void writeln_call(HkVM *vm, HkValue *args);

static inline File *file_new(FILE *stream)
{
  File *file = (File *) hk_allocate(sizeof(*file));
  hk_userdata_init((HkUserdata *) file, file_deinit);
  file->stream = stream;
  return file;
}

static void file_deinit(HkUserdata *udata)
{
  FILE *stream = ((File *) udata)->stream;
  if (stream == stdin || stream == stdout || stream == stderr)
    return;
  (void) fclose(stream);
}

static void open_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkString *filename = hk_as_string(args[1]);
  HkString *mode = hk_as_string(args[2]);
  FILE *stream = NULL;
#ifdef _WIN32
  (void) fopen_s(&stream, filename->chars, mode->chars);
#else
  stream = fopen(filename->chars, mode->chars);
#endif
  if (!stream)
  {
    hk_vm_push_nil(vm);
    return;
  }
  hk_vm_push_userdata(vm, (HkUserdata *) file_new(stream));
}

static void close_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkValue val = args[1];
  FILE *stream = ((File *) hk_as_userdata(val))->stream;
  int status = fclose(stream);
  hk_vm_push_number(vm, status);
}

static void popen_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkString *command = hk_as_string(args[1]);
  HkString *mode = hk_as_string(args[2]);
  FILE *stream;
  stream = popen(command->chars, mode->chars);
  if (!stream)
  {
    hk_vm_push_nil(vm);
    return;
  }
  hk_vm_push_userdata(vm, (HkUserdata *) file_new(stream));
}

static void pclose_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int status = pclose(stream);
  hk_vm_push_number(vm, status);
}

static void eof_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  hk_vm_push_bool(vm, (bool) feof(stream));
}

static void flush_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  hk_vm_push_number(vm, fflush(stream));
}

static void sync_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  bool result;
#ifdef _WIN32
  int fd = _fileno(stream);
  HANDLE handle = (HANDLE) _get_osfhandle(fd);
  result = FlushFileBuffers(handle);
#else
  int fd = fileno(stream);
  result = !fsync(fd);
#endif
  hk_vm_push_bool(vm, result);
}

static void tell_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  hk_vm_push_number(vm, ftell(stream));
}

static void rewind_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  rewind(stream);
  hk_vm_push_nil(vm);
}

static void seek_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  long offset = (long) hk_as_number(args[2]);
  int whence = (int) hk_as_number(args[3]);
  hk_vm_push_number(vm, fseek(stream, offset, whence));
}

static void read_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int size = (int) hk_as_number(args[2]);
  HkString *str = hk_string_new_with_capacity(size);
  int length = (int) fread(str->chars, 1, size, stream);
  if (length < size && !feof(stream))
  {
    hk_string_free(str);
    hk_vm_push_nil(vm);
    return;
  }
  str->length = length;
  str->chars[length] = '\0';
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

static void write_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  HkString *str = hk_as_string(args[2]);
  int size = str->length;
  if ((int) fwrite(str->chars, 1, size, stream) < size)
  {
    hk_vm_push_nil(vm);
    return;
  }
  hk_vm_push_number(vm, size);
}

static void readln_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  hk_vm_push_string_from_stream(vm, stream, '\n');
}

static void writeln_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  HkString *str = hk_as_string(args[2]);
  int size = str->length;
  if ((int) fwrite(str->chars, 1, size, stream) < size
   || fwrite("\n", 1, 1, stream) < 1)
  {
    hk_vm_push_nil(vm);
    return;
  }
  hk_vm_push_number(vm, size + 1);
}

HK_LOAD_MODULE_HANDLER(io)
{
  hk_vm_push_string_from_chars(vm, -1, "io");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "stdin");
  hk_return_if_not_ok(vm);
  hk_vm_push_userdata(vm, (HkUserdata *) file_new(stdin));
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "stdout");
  hk_return_if_not_ok(vm);
  hk_vm_push_userdata(vm, (HkUserdata *) file_new(stdout));
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "stderr");
  hk_return_if_not_ok(vm);
  hk_vm_push_userdata(vm, (HkUserdata *) file_new(stderr));
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SEEK_SET");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SEEK_SET);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SEEK_CUR");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SEEK_CUR);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SEEK_END");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SEEK_END);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "open");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "open", 2, open_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "close");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "close", 1, close_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "popen");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "popen", 2, popen_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "pclose");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "pclose", 1, pclose_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "eof");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "eof", 1, eof_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "flush");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "flush", 1, flush_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "sync");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "sync", 1, sync_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "tell");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "tell", 1, tell_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "rewind");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "rewind", 1, rewind_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "seek");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "seek", 3, seek_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "read");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "read", 2, read_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "write");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "write", 2, write_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "readln");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "readln", 1, readln_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "writeln");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "writeln", 2, writeln_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 20);
}
