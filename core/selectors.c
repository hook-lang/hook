//
// The Hook Programming Language
// selectors.c
//

#include "selectors.h"

#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <poll.h>
#endif

#ifdef _WIN32
  #define Socket SOCKET
  #define PollFd WSAPOLLFD
#else
  #define Socket int
  #define PollFd struct pollfd
#endif

#define MAX_FDS (4096)

typedef struct
{
  HK_USERDATA_HEADER
  int    fld0;
  int    fld1;
  int    fld2;
  Socket sock;
} SocketUserdata;

typedef struct
{
  HK_USERDATA_HEADER
  int            count;
  PollFd         fds[MAX_FDS];
  SocketUserdata *udatas[MAX_FDS];
} PollSelector;

#ifdef _WIN32
  static int initialized = 0;
#endif

#ifdef _WIN32
  static inline void startup(void);
  static inline void cleanup(void);
#endif

static inline int socket_poll(PollFd *fds, int nfds, int timeout);
static inline PollSelector *poll_selector_new(void);
static inline bool poll_selector_register(PollSelector *selector,
  SocketUserdata *udata, int events);
static inline bool poll_selector_unregister(PollSelector *selector,
  SocketUserdata *udata);
static inline bool poll_selector_modify(PollSelector *selector,
  SocketUserdata *udata, int events);
static inline HkArray *poll_selector_poll(PollSelector *selector, int timeout);
static void poll_selector_deinit(HkUserdata *udata);
static void new_poll_selector_call(HkState *state, HkValue *args);
static void register_call(HkState *state, HkValue *args);
static void unregister_call(HkState *state, HkValue *args);
static void modify_call(HkState *state, HkValue *args);
static void poll_call(HkState *state, HkValue *args);

#ifdef _WIN32
  static inline void startup(void)
  {
    if (!initialized)
    {
      WSADATA wsa;
      (void) WSAStartup(MAKEWORD(2, 2), &wsa);
    }
    ++initialized;
  }

  static inline void cleanup(void)
  {
    --initialized;
    if (initialized) return;
    (void) WSACleanup();
  }
#endif

static inline int socket_poll(PollFd *fds, int nfds, int timeout)
{
#ifdef _WIN32
  return WSAPoll(fds, nfds, timeout);
#else
  return poll(fds, nfds, timeout);
#endif
}

static inline PollSelector *poll_selector_new(void)
{
#ifdef _WIN32
  startup();
#endif
  PollSelector *selector = (PollSelector *) hk_allocate(sizeof(*selector));
  hk_userdata_init((HkUserdata *) selector, poll_selector_deinit);
  selector->count = 0;
  return selector;
}

static inline bool poll_selector_register(PollSelector *selector,
  SocketUserdata *udata, int events)
{
  if (selector->count == MAX_FDS) return false;
  PollFd fd = {
    .fd = (int) udata->sock,
    .events = events,
    .revents = 0
  };
  int index = selector->count;
  selector->fds[index] = fd;
  selector->udatas[index] = udata;
  ++selector->count;
  return true;
}

static inline bool poll_selector_unregister(PollSelector *selector,
  SocketUserdata *udata)
{
  int i = 1;
  int n = selector->count;
  for (; i < n; ++i)
  {
    PollFd *fd = &selector->fds[i];
    if (fd->fd == (int) udata->sock) break;
  }
  if (i == n) return false;
  for (; i < n - 1; ++i)
  {
    selector->fds[i] = selector->fds[i + 1];
    selector->udatas[i] = selector->udatas[i + 1];
  }
  --selector->count;
  return true;
}

static inline bool poll_selector_modify(PollSelector *selector,
  SocketUserdata *udata, int events)
{
  int i = 1;
  int n = selector->count;
  for (; i < n; ++i)
  {
    PollFd *fd = &selector->fds[i];
    if (fd->fd == (int) udata->sock) break;
  }
  if (i == n) return false;
  selector->fds[i].events = events;
  return true;
}

static inline HkArray *poll_selector_poll(PollSelector *selector, int timeout)
{
  HkArray *arr = hk_array_new();
  int n = selector->count;
  int rc = socket_poll(selector->fds, n, timeout);
  if (rc == -1)
  {
    hk_array_free(arr);
    return NULL;
  }
  if (!rc) goto end;
  int j = 0;
  for (int i = 0; i < n && j < rc; ++i)
  {
    PollFd *fd = &selector->fds[i];
    int revents = fd->revents;
    if (!revents) continue;
    HkUserdata *udata = (HkUserdata *) selector->udatas[i];
    HkArray *event = hk_array_new_with_capacity(2);
    hk_array_inplace_add_element(event, hk_userdata_value(udata));
    hk_array_inplace_add_element(event, hk_number_value(revents));
    hk_array_inplace_add_element(arr, hk_array_value(event));
    ++j;
  }
end:
  return arr;
}

static void poll_selector_deinit(HkUserdata *udata)
{
  (void) udata;
#ifdef _WIN32
  cleanup();
#endif
}

static void new_poll_selector_call(HkState *state, HkValue *args)
{
  (void) args;
  hk_state_push_userdata(state, (HkUserdata *) poll_selector_new());
}

static void register_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  PollSelector *selector = (PollSelector *) hk_as_userdata(args[1]);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[2]);
  int events = (int) hk_as_number(args[3]);
  if (!poll_selector_register(selector, udata, events))
  {
    hk_state_runtime_error(state, "too many file descriptors");
    return;
  }
  hk_state_push_nil(state);
}

static void unregister_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  PollSelector *selector = (PollSelector *) hk_as_userdata(args[1]);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[2]);
  if (!poll_selector_unregister(selector, udata))
  {
    hk_state_runtime_error(state, "file descriptor not found");
    return;
  }
  hk_state_push_nil(state);
}

static void modify_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  PollSelector *selector = (PollSelector *) hk_as_userdata(args[1]);
  SocketUserdata *udata = (SocketUserdata *) hk_as_userdata(args[2]);
  int events = (int) hk_as_number(args[3]);
  if (!poll_selector_modify(selector, udata, events))
  {
    hk_state_runtime_error(state, "file descriptor not found");
    return;
  }
  hk_state_push_nil(state);
}

static void poll_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_number(state, args, 2);
  hk_return_if_not_ok(state);
  PollSelector *selector = (PollSelector *) hk_as_userdata(args[1]);
  int timeout = (int) hk_as_number(args[2]);
  HkArray *arr = poll_selector_poll(selector, timeout);
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

HK_LOAD_MODULE_HANDLER(selectors)
{
  hk_state_push_string_from_chars(state, -1, "selectors");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "POLLIN");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, POLLIN);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "POLLOUT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, POLLOUT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "POLLERR");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, POLLERR);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "POLLHUP");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, POLLHUP);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "POLLNVAL");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, POLLNVAL);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "POLLPRI");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, POLLPRI);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new_poll_selector");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_poll_selector", 0, new_poll_selector_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "register");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "register", 3, register_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "unregister");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "unregister", 2, unregister_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "modify");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "modify", 3, modify_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "poll");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "poll", 2, poll_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 11);
}
