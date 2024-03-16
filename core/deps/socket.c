//
// The Hook Programming Language
// socket.c
//

#include "socket.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
  #include <ws2tcpip.h>
#else
  #include <arpa/inet.h>
  #include <errno.h>
  #include <fcntl.h>
  #include <unistd.h>
#endif

#ifdef _WIN32
  #define socklen_t int
#endif

#ifdef _WIN32
  static int initialized = 0;
#endif

void socket_startup(void)
{
#ifdef _WIN32
  if (!initialized)
  {
    WSADATA wsa;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
    assert(!rc);
    (void) rc;
  }
  ++initialized;
#endif
}

void socket_cleanup(void)
{
#ifdef _WIN32
  --initialized;
  if (initialized)
    return;
  int rc = WSACleanup();
  assert(!rc);
  (void) rc;
#endif
}

Socket socket_create(int domain, int type, int protocol)
{
  return socket(domain, type, protocol);
}

void socket_close(Socket sock)
{
  int rc;
#ifdef _WIN32
  rc = closesocket(sock);
  assert(!rc);
#else
  rc = close(sock);
  assert(!rc);
#endif
  (void) rc;
}

int socket_set_option(Socket sock, int level, int opt, const void *val, int len)
{
  return setsockopt(sock, level, opt, val, len);
}

void socket_address_init(SocketAddress *addr, int family, const char *ip, int port)
{
  memset(addr, 0, sizeof(*addr));
#ifdef _WIN32
  addr->sin_family = (ADDRESS_FAMILY) family;
#else
  addr->sin_family = family;
#endif
  addr->sin_port = htons((uint16_t) port);
  addr->sin_addr.s_addr = INADDR_ANY;
  if (!ip)
    return;
  int rc = inet_pton(family, ip, &addr->sin_addr);
  assert(rc == 1);
  (void) rc;
}

int socket_bind(Socket sock, SocketAddress *addr)
{
  return bind(sock, (struct sockaddr *) addr, sizeof(*addr));
}

int socket_listen(Socket sock, int backlog)
{
  return listen(sock, backlog);
}

Socket socket_accept(Socket sock, SocketAddress *addr)
{
  Socket newSock;
  socklen_t len = sizeof(*addr);
  for (;;)
  {
    newSock = accept(sock, (struct sockaddr *) addr, &len);
    if (newSock != INVALID_SOCKET)
      break;
  #ifdef _WIN32
    if (WSAGetLastError() == WSAEINTR)
      continue;
  #else
    if (errno == EINTR)
      continue;
  #endif
    break;
  }
  return newSock;
}

int socket_connect(Socket sock, SocketAddress *addr)
{
  return connect(sock, (struct sockaddr *) addr, sizeof(*addr));
}

int socket_send(Socket sock, const void *buf, int len, int flags)
{
  return (int) send(sock, buf, len, flags);
}

int socket_recv(Socket sock, void *buf, int len, int flags)
{
  return (int) recv(sock, buf, len, flags);
}

int socket_send_all(Socket sock, const void *buf, int len, int flags)
{
  char *ptr = (char *) buf;
  int count = 0;
  while (count < len)
  {
    int n;
  #ifdef _WIN32
    n = send(sock, ptr, len - count, flags);
  #else
    n = (int) send(sock, ptr, len - count, flags);
  #endif
    if (!n)
      return count;
    if (n == SOCKET_ERROR)
      return SOCKET_ERROR;
    count += n;
    ptr += n;
  }
  return count;
}

int socket_recv_all(Socket sock, void *buf, int len, int flags)
{
  char *ptr = (char *) buf;
  int count = 0;
  while (count < len)
  {
    int n;
  #ifdef _WIN32
    n = recv(sock, ptr, len - count, flags);
  #else
    n = (int) recv(sock, ptr, len - count, flags);
  #endif
    if (!n)
      return count;
    if (n == SOCKET_ERROR)
      return SOCKET_ERROR;
    count += n;
    ptr += n;
  }
  return count;
}

void socket_set_blocking(Socket sock)
{
#ifdef _WIN32
  unsigned long arg = 0;
  ioctlsocket(sock, FIONBIO, &arg);
#else
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
#endif
}

void socket_set_nonblocking(Socket sock)
{
#ifdef _WIN32
  unsigned long arg = 1;
  ioctlsocket(sock, FIONBIO, &arg);
#else
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

int socket_poll(PollFd *fds, int nfds, int timeout)
{
#ifdef _WIN32
  return WSAPoll(fds, nfds, timeout);
#else
  return poll(fds, nfds, timeout);
#endif
}
