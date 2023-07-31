//
// The Hook Programming Language
// io.c
//

#include "io.h"
#include <stdio.h>
#include <hook/memory.h>
#include <hook/check.h>
#include <hook/status.h>

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
static int open_call(HkState *state, HkValue *args);
static int close_call(HkState *state, HkValue *args);
static int popen_call(HkState *state, HkValue *args);
static int pclose_call(HkState *state, HkValue *args);
static int eof_call(HkState *state, HkValue *args);
static int flush_call(HkState *state, HkValue *args);
static int sync_call(HkState *state, HkValue *args);
static int tell_call(HkState *state, HkValue *args);
static int rewind_call(HkState *state, HkValue *args);
static int seek_call(HkState *state, HkValue *args);
static int read_call(HkState *state, HkValue *args);
static int write_call(HkState *state, HkValue *args);
static int readln_call(HkState *state, HkValue *args);
static int writeln_call(HkState *state, HkValue *args);

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

static int open_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *filename = hk_as_string(args[1]);
  HkString *mode = hk_as_string(args[2]);
  FILE *stream = fopen(filename->chars, mode->chars);
  if (!stream)
    return hk_state_push_nil(state);
  return hk_state_push_userdata(state, (HkUserdata *) file_new(stream));
}

static int close_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, fclose(((File *) hk_as_userdata(args[1]))->stream));
}

static int popen_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *command = hk_as_string(args[1]);
  HkString *mode = hk_as_string(args[2]);
  FILE *stream;
  stream = popen(command->chars, mode->chars);
  if (!stream)
    return hk_state_push_nil(state);
  return hk_state_push_userdata(state, (HkUserdata *) file_new(stream));
}

static int pclose_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int status;
  status = pclose(stream);
  return hk_state_push_number(state, status);
}

static int eof_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  return hk_state_push_bool(state, (bool) feof(stream));
}

static int flush_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  return hk_state_push_number(state, fflush(stream));
}

static int sync_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int fd = fileno(stream);
  bool result;
#ifdef _WIN32
  result = FlushFileBuffers(fd);
#else
  result = !fsync(fd);
#endif
  return hk_state_push_bool(state, result);
}

static int tell_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  return hk_state_push_number(state, ftell(stream));
}

static int rewind_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  rewind(stream);
  return hk_state_push_nil(state);
}

static int seek_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int64_t offset = (int64_t) hk_as_number(args[2]);
  int whence = (int) hk_as_number(args[3]);
  return hk_state_push_number(state, fseek(stream, offset, whence));
}

static int read_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  int64_t size = (int64_t) hk_as_number(args[2]);
  HkString *str = hk_string_new_with_capacity(size);
  int length = (int) fread(str->chars, 1, size, stream);
  if (length < size && !feof(stream))
  {
    hk_string_free(str);
    return hk_state_push_nil(state);
  }
  str->length = length;
  str->chars[length] = '\0';
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int write_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  HkString *str = hk_as_string(args[2]);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size)
    return hk_state_push_nil(state);
  return hk_state_push_number(state, size);
}

static int readln_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  return hk_state_push_string_from_stream(state, stream, '\n');
}

static int writeln_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  FILE *stream = ((File *) hk_as_userdata(args[1]))->stream;
  HkString *str = hk_as_string(args[2]);
  size_t size = str->length;
  if (fwrite(str->chars, 1, size, stream) < size || fwrite("\n", 1, 1, stream) < 1)
    return hk_state_push_nil(state);
  return hk_state_push_number(state, size + 1);
}

HK_LOAD_FN(io)
{
  if (hk_state_push_string_from_chars(state, -1, "io") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "stdin") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_userdata(state, (HkUserdata *) file_new(stdin)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "stdout") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_userdata(state, (HkUserdata *) file_new(stdout)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "stderr") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_userdata(state, (HkUserdata *) file_new(stderr)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "SEEK_SET") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, SEEK_SET) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "SEEK_CUR") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, SEEK_CUR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "SEEK_END") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, SEEK_END) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "open") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "open", 2, &open_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "close") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "close", 1, &close_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "popen") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "popen", 2, &popen_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "pclose") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "pclose", 1, &pclose_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "eof") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "eof", 1, &eof_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "flush") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "flush", 1, &flush_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sync") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sync", 1, &sync_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "tell") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "tell", 1, &tell_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "rewind") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "rewind", 1, &rewind_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "seek") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "seek", 3, &seek_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "read") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "read", 2, &read_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "write") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "write", 2, &write_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "readln") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "readln", 1, &readln_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "writeln") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "writeln", 2, &writeln_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 20);
}
