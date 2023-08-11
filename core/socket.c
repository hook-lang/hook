//
// The Hook Programming Language
// socket.c
//

#include "socket.h"
#include <string.h>
#include <hook/memory.h>

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
  int domain;
  int type;
  int protocol;
  Socket sock;
} SocketWrapper;

#ifdef _WIN32
  static int initialized = 0;
#endif

static inline void socket_startup(void);
static inline void socket_cleanup(void);
static inline void socket_close(Socket sock);
static inline bool socket_resolve(int domain, int type, const char *host, char *address);
static inline SocketWrapper *socket_wrapper_new(Socket sock, int domain, int type, int protocol);
static void socket_wrapper_deinit(HkUserdata *udata);
static void new_call(HkState *state, HkValue *args);
static void close_call(HkState *state, HkValue *args);
static void connect_call(HkState *state, HkValue *args);
static void accept_call(HkState *state, HkValue *args);
static void bind_call(HkState *state, HkValue *args);
static void listen_call(HkState *state, HkValue *args);
static void send_call(HkState *state, HkValue *args);
static void recv_call(HkState *state, HkValue *args);
static void set_option_call(HkState *state, HkValue *args);
static void get_option_call(HkState *state, HkValue *args);
static void set_block_call(HkState *state, HkValue *args);
static void set_nonblock_call(HkState *state, HkValue *args);

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

static inline SocketWrapper *socket_wrapper_new(Socket sock, int domain, int type, int protocol)
{
  SocketWrapper *wrapper = (SocketWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, socket_wrapper_deinit);
  wrapper->domain = domain;
  wrapper->type = type;
  wrapper->protocol = protocol;
  wrapper->sock = sock;
  return wrapper;
}

static void socket_wrapper_deinit(HkUserdata *udata)
{
  Socket sock = ((SocketWrapper *) udata)->sock;
  if (sock == INVALID_SOCKET)
    return;
  socket_close(sock);
}

static void new_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_int(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  int domain = (int) hk_as_number(args[1]);
  int type = (int) hk_as_number(args[2]);
  int protocol = (int) hk_as_number(args[3]);
  socket_startup();
  Socket sock = socket(domain, type, protocol);
  if (sock == INVALID_SOCKET)
  {
    socket_cleanup();
    hk_state_push_nil(state);
    return;
  }
  SocketWrapper *wrapper = socket_wrapper_new(sock, domain, type, protocol);
  hk_state_push_userdata(state, (HkUserdata *) wrapper);
}

static void close_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  Socket sock = wrapper->sock;
  if (sock != INVALID_SOCKET)
  {
    socket_close(sock);
    wrapper->sock = INVALID_SOCKET;
  }
  hk_state_push_nil(state);
}

static void connect_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  HkString *host = hk_as_string(args[2]);
  int port = (int) hk_as_number(args[3]);
  char address[ADDRESS_MAX_LEN];
  if (!socket_resolve(wrapper->domain, wrapper->type, host->chars, address))
  {
    hk_state_runtime_error(state, "cannot resolve host '%s'", host->chars);
    return;
  }
  struct sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons((uint16_t) port);
  if (inet_pton(AF_INET, address, &sock_addr.sin_addr) < 1)
  {
    hk_state_push_nil(state);
    return;
  }
  Socket sock = wrapper->sock;
  if (connect(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
  {
    hk_state_runtime_error(state, "cannot connect to address '%s'", address);
    return;
  }
  hk_state_push_nil(state);
}

static void accept_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  Socket sock;
  struct sockaddr_in sock_addr;
  socklen_t sock_addr_len = sizeof(sock_addr);
  for (;;)
  {
    sock = accept(wrapper->sock, (struct sockaddr *) &sock_addr, &sock_addr_len);
    if (sock != INVALID_SOCKET)
      break;
  #ifdef _WIN32
    if (WSAGetLastError() == WSAEINTR)
      continue;
  #else
    if (errno == EINTR)
      continue;
  #endif
    hk_state_push_nil(state);
    return;
  }
  SocketWrapper *result = socket_wrapper_new(sock, wrapper->domain, wrapper->type, wrapper->protocol);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void bind_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  HkString *host = hk_as_string(args[2]);
  int port = (int) hk_as_number(args[3]);
  char address[ADDRESS_MAX_LEN];
  if (!socket_resolve(wrapper->domain, wrapper->type, host->chars, address))
  {
    hk_state_runtime_error(state, "cannot resolve host '%s'", host->chars);
    return;
  }
  struct sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = wrapper->domain;
  sock_addr.sin_port = htons((uint16_t) port);
  if (inet_pton(AF_INET, address, &sock_addr.sin_addr) < 1)
  {
    hk_state_push_nil(state);
    return;
  }
  if (bind(wrapper->sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
  {
    hk_state_runtime_error(state, "cannot bind to address '%s'", address);
    return;
  }
  hk_state_push_nil(state);
}

static void listen_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  int backlog = (int) hk_as_number(args[2]);
  if (listen(wrapper->sock, backlog) == SOCKET_ERROR)
  {
    hk_state_runtime_error(state, "cannot listen on socket");
    return;
  }
  hk_state_push_nil(state);
}

static void send_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  HkString *str = hk_as_string(args[2]);
  int flags = (int) hk_as_number(args[3]);
  int length = (int) send(wrapper->sock, str->chars, str->length, flags);
  hk_state_push_number(state, length);
}

static void recv_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  int size = (int) hk_as_number(args[2]);
  int flags = (int) hk_as_number(args[3]);
  HkString *str = hk_string_new_with_capacity(size);
  int length = (int) recv(wrapper->sock, str->chars, size, flags);
  if (!length)
  {
    hk_state_push_nil(state);
    return;
  }
  str->length = length;
  hk_state_push_string(state, str);
}

static void set_option_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 4);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  int level = (int) hk_as_number(args[2]);
  int option = (int) hk_as_number(args[3]);
  int value = (int) hk_as_number(args[4]);
  Socket sock = wrapper->sock;
#ifdef _WIN32
  int result = setsockopt(sock, level, option, (const char *) &value, sizeof(value));
#else
  int result = setsockopt(sock, level, option, &value, sizeof(value));
#endif
  if (result == SOCKET_ERROR)
  {
    hk_state_runtime_error(state, "cannot set socket option");
    return;
  }
  hk_state_push_nil(state);
}

static void get_option_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  int level = (int) hk_as_number(args[2]);
  int option = (int) hk_as_number(args[3]);
  int value;
  Socket sock = wrapper->sock;
  socklen_t size = sizeof(value);
#ifdef _WIN32
  int result = getsockopt(sock, level, option, (char *) &value, &size);
#else
  int result = getsockopt(sock, level, option, &value, &size);
#endif
  if (result == SOCKET_ERROR)
  {
    hk_state_runtime_error(state, "cannot get socket option");
    return;
  }
  hk_state_push_number(state, value);
}

static void set_block_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  Socket sock = wrapper->sock;
#ifdef _WIN32
  unsigned long mode = 1;
  int result = ioctlsocket(sock, FIONBIO, &mode);
  if (result)
  {
    hk_state_runtime_error(state, "cannot set socket to blocking mode");
    return;
  }
#else
  int flags = fcntl(sock, F_GETFL, 0);
  int result = fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
  if (result == -1)
  {
    hk_state_runtime_error(state, "cannot set socket to blocking mode");
    return;
  }
#endif
  hk_state_push_nil(state);
}

static void set_nonblock_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  SocketWrapper *wrapper = (SocketWrapper *) hk_as_userdata(args[1]);
  Socket sock = wrapper->sock;
#ifdef _WIN32
  unsigned long mode = 0;
  int result = ioctlsocket(sock, FIONBIO, &mode);
  if (result)
  {
    hk_state_runtime_error(state, "cannot set socket to non-blocking mode");
    return;
  }
#else
  int flags = fcntl(sock, F_GETFL, 0);
  int result = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  if (result == -1)
  {
    hk_state_runtime_error(state, "cannot set socket to non-blocking mode");
    return;
  }
#endif
  hk_state_push_nil(state);
}

HK_LOAD_MODULE_HANDLER(socket)
{
  hk_state_push_string_from_chars(state, -1, "socket");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "AF_INET");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, AF_INET);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "AF_INET6");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, AF_INET6);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SOCK_STREAM");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SOCK_STREAM);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SOCK_DGRAM");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SOCK_DGRAM);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "IPPROTO_TCP");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, IPPROTO_TCP);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "IPPROTO_UDP");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, IPPROTO_UDP);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SOL_SOCKET");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SOL_SOCKET);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SO_REUSEADDR");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SO_REUSEADDR);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new", 3, new_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "close");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "close", 1, close_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "connect");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "connect", 3, connect_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "accept");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "accept", 1, accept_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "bind");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "bind", 3, bind_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "listen");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "listen", 2, listen_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "send");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "send", 3, send_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "recv");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "recv", 3, recv_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "set_option");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "set_option", 4, set_option_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "get_option");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "get_option", 3, get_option_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "set_block");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "set_block", 1, set_block_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "set_nonblock");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "set_nonblock", 1, set_nonblock_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 20);
}
