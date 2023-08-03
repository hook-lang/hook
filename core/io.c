//
// The Hook Programming Language
// io.c
//

#include "io.h"
#include <stdio.h>
#include <hook/memory.h>
#include <hook/check.h>

#ifdef _WIN32
  #include <windows.h>
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
static void open_call(HkState *state, HkValue *args);
static void close_call(HkState *state, HkValue *args);
static void popen_call(HkState *state, HkValue *args);
static void pclose_call(HkState *state, HkValue *args);
static void eof_call(HkState *state, HkValue *args);
static void flush_call(HkState *state, HkValue *args);
static void sync_call(HkState *state, HkValue *args);
static void tell_call(HkState *state, HkValue *args);
static void rewind_call(HkState *state, HkValue *args);
static void seek_call(HkState *state, HkValue *args);
static void read_call(HkState *state, HkValue *args);
static void write_call(HkState *state, HkValue *args);
static void readln_call(HkState *state, HkValue *args);
static void writeln_call(HkState *state, HkValue *args);

static inline File *file_new(FILE *stream)
{
  File *file = (File *) hk_allocate(sizeof(*file));
  hk_userdata_init((HkUserdata *) file, &file_deinit);
  file->stream = stream;
  return file;
}

static void file_deinit(HkUserdata *udata)
{
  FILE *stream = ((File *) udata)->stream;
  if (stream == stdin || stream == stdout || stream == stderr)
    return;
  fclose(stream);
}

static void open_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  HkString *filename = hk_as_string(args[1]);
  HkString *mode = hk_as_string(args[2]);
  FILE *stream = fopen(filename->chars, mode->chars);
  if (!stream)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) file_new(stream));
}

static void close_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  FILE *stream = ((File *) hk_as_userdata(val))->stream;
  int status = fclose(stream);
  hk_state_push_number(state, status);
}

static void popen_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  HkString *command = hk_as_string(args[1]);
  HkString *mode = hk_as_string(args[2]);
  FILE *stream;
  stream = popen(command->chars, mode->chars);
  if (!stream)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) file_new(stream));
}

static void pclose_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int status = pclose(stream);
  hk_state_push_number(state, status);
}

static void eof_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  hk_state_push_bool(state, (bool) feof(stream));
}

static void flush_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  hk_state_push_number(state, fflush(stream));
}

static void sync_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int fd = fileno(stream);
  bool result;
#ifdef _WIN32
  result = FlushFileBuffers(fd);
#else
  result = !fsync(fd);
#endif
  hk_state_push_bool(state, result);
}

static void tell_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  hk_state_push_number(state, ftell(stream));
}

static void rewind_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  rewind(stream);
  hk_state_push_nil(state);
}

static void seek_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int64_t offset = (int64_t) hk_as_number(args[2]);
  int whence = (int) hk_as_number(args[3]);
  hk_state_push_number(state, fseek(stream, offset, whence));
}

static void read_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int64_t size = (int64_t) hk_as_number(args[2]);
  HkString *str = hk_string_new_with_capacity(size);
  int length = (int) fread(str->chars, 1, size, stream);
  if (length < size && !feof(stream))
  {
    hk_string_free(str);
    hk_state_push_nil(state);
    return;
  }
  str->length = length;
  str->chars[length] = '\0';
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void write_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  HkString *str = hk_as_string(args[2]);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_number(state, size);
}

static void readln_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  hk_state_push_string_from_stream(state, stream, '\n');
}

static void writeln_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  HkString *str = hk_as_string(args[2]);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size || fwrite("\n", 1, 1, stream) < 1)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_number(state, size + 1);
}

HK_LOAD_MODULE_HANDLER(io)
{
  hk_state_push_string_from_chars(state, -1, "io");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "stdin");
  hk_return_if_not_ok(state);
  hk_state_push_userdata(state, (HkUserdata *) file_new(stdin));
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "stdout");
  hk_return_if_not_ok(state);
  hk_state_push_userdata(state, (HkUserdata *) file_new(stdout));
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "stderr");
  hk_return_if_not_ok(state);
  hk_state_push_userdata(state, (HkUserdata *) file_new(stderr));
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SEEK_SET");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SEEK_SET);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SEEK_CUR");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SEEK_CUR);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SEEK_END");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SEEK_END);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "open");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "open", 2, &open_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "close");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "close", 1, &close_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "popen");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "popen", 2, &popen_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "pclose");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "pclose", 1, &pclose_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "eof");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "eof", 1, &eof_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "flush");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "flush", 1, &flush_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sync");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sync", 1, &sync_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "tell");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "tell", 1, &tell_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "rewind");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "rewind", 1, &rewind_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "seek");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "seek", 3, &seek_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "read");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "read", 2, &read_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "write");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "write", 2, &write_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "readln");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "readln", 1, &readln_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "writeln");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "writeln", 2, &writeln_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 20);
}
