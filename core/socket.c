//
// socket.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "socket.h"
#include <string.h>

#ifdef _WIN32
  #include <winsock2.h>
#endif

#ifndef _WIN32
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
#endif

#ifdef _WIN32
  #define Socket    SOCKET
  #define socklen_t int
#endif

#ifndef _WIN32
  #define Socket         int
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR   -1
#endif

#define ADDRESS_MAX_LEN 15

typedef struct
{
  HK_USERDATA_HEADER
  int    domain;
  int    type;
  int    protocol;
  Socket sock;
} SocketUserdata;

#ifdef _WIN32
  static int initialized = 0;
#endif

static inline void socket_startup(void);
static inline void socket_cleanup(void);
static inline void socket_close(Socket sock);
static inline bool socket_resolve(int domain, int type, const char *host, char *address);
static inline int send_all(HkVM *vm, Socket sock, char *chars, int size);
static inline SocketUserdata *socket_userdata_new(Socket sock, int domain, int type, int protocol);
static void socket_userdata_deinit(HkUserdata *udata);
static void new_call(HkVM *vm, HkValue *args);
static void close_call(HkVM *vm, HkValue *args);
static void connect_call(HkVM *vm, HkValue *args);
static void accept_call(HkVM *vm, HkValue *args);
static void bind_call(HkVM *vm, HkValue *args);
static void listen_call(HkVM *vm, HkValue *args);
static void send_call(HkVM *vm, HkValue *args);
static void recv_call(HkVM *vm, HkValue *args);
static void writeln_call(HkVM *vm, HkValue *args);
static void readln_call(HkVM *vm, HkValue *args);
static void set_option_call(HkVM *vm, HkValue *args);
static void get_option_call(HkVM *vm, HkValue *args);
static void set_block_call(HkVM *vm, HkValue *args);
static void set_nonblock_call(HkVM *vm, HkValue *args);

static inline void socket_startup(void)
{
#ifdef _WIN32
  if (!initialized)
  {
    WSADATA wsa;
    (void) WSAStartup(MAKEWORD(2, 2), &wsa);
  }
  ++initialized;
#endif
}

static inline void socket_cleanup(void)
{
#ifdef _WIN32
  --initialized;
  if (!initialized)
    (void) WSACleanup();
#endif
}

static inline void socket_close(Socket sock)
{
#ifdef _WIN32
  closesocket(sock);
  socket_cleanup();
#else
  close(sock);
#endif
}

static inline bool socket_resolve(int domain, int type, const char *host, char *address)
{
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = domain;
  hints.ai_socktype = type;
  if (getaddrinfo(host, NULL, &hints, &res))
    return false;
  struct sockaddr_in *sock_addr = (struct sockaddr_in *) res->ai_addr;
  void *ptr = &sock_addr->sin_addr;
  inet_ntop(res->ai_family, ptr, address, ADDRESS_MAX_LEN);
  freeaddrinfo(res);
  return true;
}

static inline int send_all(HkVM *vm, Socket sock, char *chars, int size)
{
  int n = 0;
  while (n < size)
  {
    int sent = (int) send(sock, &chars[n], size - n, 0);
    if (!sent)
      break;
    if (sent == SOCKET_ERROR)
    {
      hk_vm_runtime_error(vm, "cannot write to socket");
      return 0;
    }
    n += sent;
  }
  return n;
}

static inline SocketUserdata *socket_userdata_new(Socket sock, int domain, int type, int protocol)
{
  SocketUserdata *udata = (SocketUserdata *) hk_allocate(sizeof(*udata));
  hk_userdata_init((HkUserdata *) udata, socket_userdata_deinit);
  udata->domain = domain;
  udata->type = type;
  udata->protocol = protocol;
  udata->sock = sock;
  return udata;
}

static void socket_userdata_deinit(HkUserdata *udata)
{
  Socket sock = ((SocketUserdata *) udata)->sock;
  if (sock == INVALID_SOCKET)
    return;
  socket_close(sock);
}

static void new_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  int domain = (int) hk_as_number(args[1]);
  int type = (int) hk_as_number(args[2]);
  int protocol = (int) hk_as_number(args[3]);
  socket_startup();
  Socket sock = socket(domain, type, protocol);
  if (sock == INVALID_SOCKET)
  {
    socket_cleanup();
    hk_vm_push_nil(vm);
    return;
  }
  SocketUserdata *udata = socket_userdata_new(sock, domain, type, protocol);
  hk_vm_push_userdata(vm, (HkUserdata *) udata);
}

static void close_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  Socket sock = udata->sock;
  if (sock != INVALID_SOCKET)
  {
    socket_close(sock);
    udata->sock = INVALID_SOCKET;
  }
  hk_vm_push_nil(vm);
}

static void connect_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  HkString *host = hk_as_string(args[2]);
  int port = (int) hk_as_number(args[3]);
  char address[ADDRESS_MAX_LEN];
  if (!socket_resolve(udata->domain, udata->type, host->chars, address))
  {
    hk_vm_runtime_error(vm, "cannot resolve host '%s'", host->chars);
    return;
  }
  struct sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons((uint16_t) port);
  if (inet_pton(AF_INET, address, &sock_addr.sin_addr) < 1)
  {
    hk_vm_push_nil(vm);
    return;
  }
  Socket sock = udata->sock;
  if (connect(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
  {
    hk_vm_runtime_error(vm, "cannot connect to address '%s'", address);
    return;
  }
  hk_vm_push_nil(vm);
}

static void accept_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  Socket sock;
  struct sockaddr_in sock_addr;
  socklen_t sock_addr_len = sizeof(sock_addr);
  for (;;)
  {
    sock = accept(udata->sock, (struct sockaddr *) &sock_addr, &sock_addr_len);
    if (sock != INVALID_SOCKET)
      break;
  #ifdef _WIN32
    if (WSAGetLastError() == WSAEINTR)
      continue;
  #else
    if (errno == EINTR)
      continue;
  #endif
    hk_vm_push_nil(vm);
    return;
  }
  SocketUserdata *result = socket_userdata_new(sock, udata->domain, udata->type, udata->protocol);
  hk_vm_push_userdata(vm, (HkUserdata *) result);
}

static void bind_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  HkString *host = hk_as_string(args[2]);
  int port = (int) hk_as_number(args[3]);
  char address[ADDRESS_MAX_LEN];
  if (!socket_resolve(udata->domain, udata->type, host->chars, address))
  {
    hk_vm_runtime_error(vm, "cannot resolve host '%s'", host->chars);
    return;
  }
  struct sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = (unsigned short) udata->domain;
  sock_addr.sin_port = htons((uint16_t) port);
  if (inet_pton(AF_INET, address, &sock_addr.sin_addr) < 1)
  {
    hk_vm_push_nil(vm);
    return;
  }
  if (bind(udata->sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
  {
    hk_vm_runtime_error(vm, "cannot bind to address '%s'", address);
    return;
  }
  hk_vm_push_nil(vm);
}

static void listen_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  int backlog = (int) hk_as_number(args[2]);
  if (listen(udata->sock, backlog) == SOCKET_ERROR)
  {
    hk_vm_runtime_error(vm, "cannot listen on socket");
    return;
  }
  hk_vm_push_nil(vm);
}

static void send_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  Socket sock = ((SocketUserdata *) hk_as_userdata(args[1]))->sock;
  HkString *str = hk_as_string(args[2]);
  int flags = (int) hk_as_number(args[3]);
  int length = (int) send(sock, str->chars, str->length, flags);
  hk_vm_push_number(vm, length);
}

static void recv_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  Socket sock = ((SocketUserdata *) hk_as_userdata(args[1]))->sock;
  int size = (int) hk_as_number(args[2]);
  int flags = (int) hk_as_number(args[3]);
  HkString *str = hk_string_new_with_capacity(size);
  int length = (int) recv(sock, str->chars, size, flags);
  if (!length)
  {
    hk_string_free(str);
    hk_vm_push_nil(vm);
    return;
  }
  str->length = length;
  hk_vm_push_string(vm, str);
}

static void writeln_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  Socket sock = ((SocketUserdata *) hk_as_userdata(args[1]))->sock;
  HkString *str = hk_as_string(args[2]);
  int n = send_all(vm, sock, str->chars, str->length);
  hk_return_if_not_ok(vm);
  if (n < str->length)
    goto end;
  n += send_all(vm, sock, "\n", 1);
  hk_return_if_not_ok(vm);
end:
  hk_vm_push_number(vm, n);
}

static void readln_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  Socket sock = ((SocketUserdata *) hk_as_userdata(args[1]))->sock;
  HkString *str = hk_string_new();
  char c;
  for (;;)
  {
    int received = (int) recv(sock, &c, 1, 0);
    if (!received)
      break;
    if (received == SOCKET_ERROR)
    {
      hk_string_free(str);
      hk_vm_runtime_error(vm, "cannot read from socket");
      return;
    }
    if (c == '\n')
      break;
    hk_string_inplace_concat_char(str, c);
  }
  hk_vm_push_string(vm, str);
}

static void set_option_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 4);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  int level = (int) hk_as_number(args[2]);
  int option = (int) hk_as_number(args[3]);
  int value = (int) hk_as_number(args[4]);
  Socket sock = udata->sock;
#ifdef _WIN32
  int result = setsockopt(sock, level, option, (const char *) &value, sizeof(value));
#else
  int result = setsockopt(sock, level, option, &value, sizeof(value));
#endif
  if (result == SOCKET_ERROR)
  {
    hk_vm_runtime_error(vm, "cannot set socket option");
    return;
  }
  hk_vm_push_nil(vm);
}

static void get_option_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 3);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  int level = (int) hk_as_number(args[2]);
  int option = (int) hk_as_number(args[3]);
  int value;
  Socket sock = udata->sock;
  socklen_t size = sizeof(value);
#ifdef _WIN32
  int result = getsockopt(sock, level, option, (char *) &value, &size);
#else
  int result = getsockopt(sock, level, option, &value, &size);
#endif
  if (result == SOCKET_ERROR)
  {
    hk_vm_runtime_error(vm, "cannot get socket option");
    return;
  }
  hk_vm_push_number(vm, value);
}

static void set_block_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  Socket sock = udata->sock;
#ifdef _WIN32
  unsigned long mode = 1;
  int result = ioctlsocket(sock, FIONBIO, &mode);
  if (result)
  {
    hk_vm_runtime_error(vm, "cannot set socket to blocking mode");
    return;
  }
#else
  int flags = fcntl(sock, F_GETFL, 0);
  int result = fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
  if (result == -1)
  {
    hk_vm_runtime_error(vm, "cannot set socket to blocking mode");
    return;
  }
#endif
  hk_vm_push_nil(vm);
}

static void set_nonblock_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[1]);
  Socket sock = udata->sock;
#ifdef _WIN32
  unsigned long mode = 0;
  int result = ioctlsocket(sock, FIONBIO, &mode);
  if (result)
  {
    hk_vm_runtime_error(vm, "cannot set socket to non-blocking mode");
    return;
  }
#else
  int flags = fcntl(sock, F_GETFL, 0);
  int result = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  if (result == -1)
  {
    hk_vm_runtime_error(vm, "cannot set socket to non-blocking mode");
    return;
  }
#endif
  hk_vm_push_nil(vm);
}

HK_LOAD_MODULE_HANDLER(socket)
{
  hk_vm_push_string_from_chars(vm, -1, "socket");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "AF_INET");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, AF_INET);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "AF_INET6");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, AF_INET6);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SOCK_STREAM");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SOCK_STREAM);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SOCK_DGRAM");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SOCK_DGRAM);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IPPROTO_TCP");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, IPPROTO_TCP);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "IPPROTO_UDP");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, IPPROTO_UDP);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SOL_SOCKET");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SOL_SOCKET);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SO_REUSEADDR");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SO_REUSEADDR);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new", 3, new_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "close");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "close", 1, close_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "connect");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "connect", 3, connect_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "accept");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "accept", 1, accept_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "bind");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "bind", 3, bind_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "listen");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "listen", 2, listen_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "send");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "send", 3, send_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "recv");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "recv", 3, recv_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "writeln");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "writeln", 2, writeln_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "readln");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "readln", 1, readln_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "set_option");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "set_option", 4, set_option_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "get_option");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "get_option", 3, get_option_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "set_block");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "set_block", 1, set_block_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "set_nonblock");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "set_nonblock", 1, set_nonblock_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 22);
}
