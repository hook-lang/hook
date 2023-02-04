//
// The Hook Programming Language
// hk_socket.c
//

#include "hk_socket.h"
#include <string.h>
#include "hk_memory.h"
#include "hk_status.h"
#include "hk_error.h"

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
  #define socket_t SOCKET
#endif

#ifndef _WIN32
  #define socket_t       int32_t
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR   -1
#endif

#define ADDRESS_MAX_LEN 15

typedef struct
{
  HK_USERDATA_HEADER
  int32_t domain;
  int32_t type;
  int32_t protocol;
  socket_t sock;
} socket_wrapper_t;

#ifdef _WIN32
  static int32_t socket_count = 0;
#endif

static inline void socket_startup(void);
static inline void socket_cleanup(void);
static inline void socket_close(socket_t sock);
static inline bool socket_resolve(int32_t domain, int32_t type, const char *host, char *address);
static inline socket_wrapper_t *socket_wrapper_new(socket_t sock, int32_t domain, int32_t type, int32_t protocol);
static void socket_wrapper_deinit(hk_userdata_t *udata);
static int32_t new_call(hk_vm_t *vm, hk_value_t *args);
static int32_t close_call(hk_vm_t *vm, hk_value_t *args);
static int32_t connect_call(hk_vm_t *vm, hk_value_t *args);
static int32_t accept_call(hk_vm_t *vm, hk_value_t *args);
static int32_t bind_call(hk_vm_t *vm, hk_value_t *args);
static int32_t listen_call(hk_vm_t *vm, hk_value_t *args);
static int32_t send_call(hk_vm_t *vm, hk_value_t *args);
static int32_t recv_call(hk_vm_t *vm, hk_value_t *args);
static int32_t set_option_call(hk_vm_t *vm, hk_value_t *args);
static int32_t get_option_call(hk_vm_t *vm, hk_value_t *args);
static int32_t set_block_call(hk_vm_t *vm, hk_value_t *args);
static int32_t set_nonblock_call(hk_vm_t *vm, hk_value_t *args);

static inline void socket_startup(void)
{
#ifdef _WIN32
  if (!socket_count)
  {
    WSADATA wsa;
    (void) WSAStartup(MAKEWORD(2, 2), &wsa);
  }
  ++socket_count;
#endif
}

static inline void socket_cleanup(void)
{
#ifdef _WIN32
  --socket_count;
  if (!socket_count)
    (void) WSACleanup();
#endif
}

static inline void socket_close(socket_t sock)
{
#ifdef _WIN32
  closesocket(sock);
  socket_cleanup();
#else
  close(sock);
#endif
}

static inline bool socket_resolve(int32_t domain, int32_t type, const char *host, char *address)
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

static inline socket_wrapper_t *socket_wrapper_new(socket_t sock, int32_t domain, int32_t type, int32_t protocol)
{
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &socket_wrapper_deinit);
  wrapper->domain = domain;
  wrapper->type = type;
  wrapper->protocol = protocol;
  wrapper->sock = sock;
  return wrapper;
}

static void socket_wrapper_deinit(hk_userdata_t *udata)
{
  socket_t sock = ((socket_wrapper_t *) udata)->sock;
  if (sock != INVALID_SOCKET)
    socket_close(sock);
}

static int32_t new_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int32_t domain = (int32_t) hk_as_number(args[1]);
  int32_t type = (int32_t) hk_as_number(args[2]);
  int32_t protocol = (int32_t) hk_as_number(args[3]);
  socket_startup();
  socket_t sock = socket(domain, type, protocol);
  if (sock == INVALID_SOCKET)
  {
    socket_cleanup();
    return hk_vm_push_nil(vm);
  }
  socket_wrapper_t *wrapper = socket_wrapper_new(sock, domain, type, protocol);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) wrapper);
}

static int32_t close_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  socket_t sock = wrapper->sock;
  if (sock != INVALID_SOCKET)
  {
    socket_close(sock);
    wrapper->sock = INVALID_SOCKET;
  }
  return hk_vm_push_nil(vm);
}

static int32_t connect_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  hk_string_t *host = hk_as_string(args[2]);
  int32_t port = (int32_t) hk_as_number(args[3]);
  char address[ADDRESS_MAX_LEN];
  if (!socket_resolve(wrapper->domain, wrapper->type, host->chars, address))
  {
    hk_runtime_error("cannot resolve host '%s'", host->chars);
    return HK_STATUS_ERROR;
  }
  struct sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons((uint16_t) port);
  if (inet_pton(AF_INET, address, &sock_addr.sin_addr) < 1)
    return hk_vm_push_nil(vm);
  socket_t sock = wrapper->sock;
  if (connect(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
  {
    hk_runtime_error("cannot connect to address '%s'", address);
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_nil(vm);
}

static int32_t accept_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  socket_t sock;
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
    return hk_vm_push_nil(vm);
  }
  socket_wrapper_t *result = socket_wrapper_new(sock, wrapper->domain, wrapper->type, wrapper->protocol);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) result);
}

static int32_t bind_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  hk_string_t *host = hk_as_string(args[2]);
  int32_t port = (int32_t) hk_as_number(args[3]);
  char address[ADDRESS_MAX_LEN];
  if (!socket_resolve(wrapper->domain, wrapper->type, host->chars, address))
  {
    hk_runtime_error("cannot resolve host '%s'", host->chars);
    return HK_STATUS_ERROR;
  }
  struct sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = wrapper->domain;
  sock_addr.sin_port = htons((uint16_t) port);
  if (inet_pton(AF_INET, address, &sock_addr.sin_addr) < 1)
    return hk_vm_push_nil(vm);
  if (bind(wrapper->sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
  {
    hk_runtime_error("cannot bind to address '%s'", address);
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_nil(vm);
}

static int32_t listen_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  int32_t backlog = (int32_t) hk_as_number(args[2]);
  if (listen(wrapper->sock, backlog) == SOCKET_ERROR)
  {
    hk_runtime_error("cannot listen on socket");
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_nil(vm);
}

static int32_t send_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  hk_string_t *str = hk_as_string(args[2]);
  int32_t flags = (int32_t) hk_as_number(args[3]);
  int32_t length = (int32_t) send(wrapper->sock, str->chars, str->length, flags);
  return hk_vm_push_number(vm, length);
}

static int32_t recv_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  int32_t size = (int32_t) hk_as_number(args[2]);
  int32_t flags = (int32_t) hk_as_number(args[3]);
  hk_string_t *str = hk_string_new_with_capacity(size);
  int32_t length = (int32_t) recv(wrapper->sock, str->chars, size, flags);
  if (!length)
    return hk_vm_push_nil(vm);
  str->length = length;
  return hk_vm_push_string(vm, str);
}

static int32_t set_option_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 4) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  int32_t level = (int32_t) hk_as_number(args[2]);
  int32_t option = (int32_t) hk_as_number(args[3]);
  int32_t value = (int32_t) hk_as_number(args[4]);
  socket_t sock = wrapper->sock;
#ifdef _WIN32
  int32_t result = setsockopt(sock, level, option, (const char *) &value, sizeof(value));
#else
  int32_t result = setsockopt(sock, level, option, &value, sizeof(value));
#endif
  if (result == SOCKET_ERROR)
  {
    hk_runtime_error("cannot set socket option");
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_nil(vm);
}

static int32_t get_option_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  int32_t level = (int32_t) hk_as_number(args[2]);
  int32_t option = (int32_t) hk_as_number(args[3]);
  int32_t value;
  socket_t sock = wrapper->sock;
  socklen_t size = sizeof(value);
#ifdef _WIN32
  int32_t result = getsockopt(sock, level, option, (char *) &value, &size);
#else
  int32_t result = getsockopt(sock, level, option, &value, &size);
#endif
  if (result == SOCKET_ERROR)
  {
    hk_runtime_error("cannot get socket option");
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_number(vm, value);
}

static int32_t set_block_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  socket_t sock = wrapper->sock;
#ifdef _WIN32
  unsigned long mode = 1;
  int32_t result = ioctlsocket(sock, FIONBIO, &mode);
  if (result)
  {
    hk_runtime_error("cannot set socket to blocking mode");
    return HK_STATUS_ERROR;
  }
#else
  int32_t flags = fcntl(sock, F_GETFL, 0);
  int32_t result = fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
  if (result == -1)
  {
    hk_runtime_error("cannot set socket to blocking mode");
    return HK_STATUS_ERROR;
  }
#endif
  return hk_vm_push_nil(vm);
}

static int32_t set_nonblock_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  socket_wrapper_t *wrapper = (socket_wrapper_t *) hk_as_userdata(args[1]);
  socket_t sock = wrapper->sock;
#ifdef _WIN32
  unsigned long mode = 0;
  int32_t result = ioctlsocket(sock, FIONBIO, &mode);
  if (result)
  {
    hk_runtime_error("cannot set socket to non-blocking mode");
    return HK_STATUS_ERROR;
  }
#else
  int32_t flags = fcntl(sock, F_GETFL, 0);
  int32_t result = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  if (result == -1)
  {
    hk_runtime_error("cannot set socket to non-blocking mode");
    return HK_STATUS_ERROR;
  }
#endif
  return hk_vm_push_nil(vm);
}

HK_LOAD_FN(socket)
{
  if (hk_vm_push_string_from_chars(vm, -1, "socket") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "AF_INET") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, AF_INET) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "AF_INET6") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, AF_INET6) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "SOCK_STREAM") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, SOCK_STREAM) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "SOCK_DGRAM") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, SOCK_DGRAM) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "IPPROTO_TCP") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, IPPROTO_TCP) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "IPPROTO_UDP") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, IPPROTO_UDP) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "SOL_SOCKET") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, SOL_SOCKET) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "SO_REUSEADDR") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, SO_REUSEADDR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "new") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "new", 3, &new_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "close") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "close", 1, &close_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "connect") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "connect", 3, &connect_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "accept") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "accept", 1, &accept_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "bind") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "bind", 3, &bind_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "listen") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "listen", 2, &listen_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "send") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "send", 3, &send_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "recv") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "recv", 3, &recv_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "set_option") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "set_option", 4, &set_option_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "get_option") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "get_option", 3, &get_option_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "set_block") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "set_block", 1, &set_block_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "set_nonblock") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "set_nonblock", 1, &set_nonblock_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 20);
}
