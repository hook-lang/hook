//
// The Hook Programming Language
// socket.h
//

#ifndef SOCKET_H
#define SOCKET_H

#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <netinet/in.h>
  #include <poll.h>
#endif

#ifdef _WIN32
  #define Socket SOCKET
  #define PollFd WSAPOLLFD
#else
  #define Socket int
  #define PollFd struct pollfd
#endif

#ifndef INVALID_SOCKET
  #define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
  #define SOCKET_ERROR (-1)
#endif

typedef struct sockaddr_in SocketAddress;

void socket_startup(void);
void socket_cleanup(void);
Socket socket_create(int domain, int type, int protocol);
void socket_close(Socket sock);
int socket_set_option(Socket sock, int level, int opt, const void *val, int len);
void socket_address_init(SocketAddress *addr, int family, const char *ip, int port);
int socket_bind(Socket sock, SocketAddress *addr);
int socket_listen(Socket sock, int backlog);
Socket socket_accept(Socket sock, SocketAddress *addr);
int socket_connect(Socket sock, SocketAddress *addr);
int socket_send(Socket sock, const void *buf, int len, int flags);
int socket_recv(Socket sock, void *buf, int len, int flags);
int socket_send_all(Socket sock, const void *buf, int len, int flags);
int socket_recv_all(Socket sock, void *buf, int len, int flags);
void socket_set_blocking(Socket sock);
void socket_set_nonblocking(Socket sock);
int socket_poll(PollFd *fds, int nfds, int timeout);

#endif // SOCKET_H
