//
// The Hook Programming Language
// http.c
//

#include "http.h"
#include <assert.h>
#include <string.h>
#include "deps/picohttpparser.h"
#include "deps/socket.h"

#define BUFFER_SIZE (4096)
#define MAX_HEADERS (256)
#define BACKLOG     (511)

typedef enum
{
  // 1xx
  STATUS_CONTINUE                        = 100,
  STATUS_SWITCHING_PROTOCOLS             = 101,
  // 2xx
  STATUS_OK                              = 200,
  STATUS_CREATED                         = 201,
  STATUS_ACCEPTED                        = 202,
  STATUS_NON_AUTHORITATIVE_INFO          = 203,
  STATUS_NO_CONTENT                      = 204,
  STATUS_RESET_CONTENT                   = 205,
  STATUS_PARTIAL_CONTENT                 = 206,
  // 3xx
  STATUS_MULTIPLE_CHOICES                = 300,
  STATUS_MOVED_PERMANENTLY               = 301,
  STATUS_FOUND                           = 302,
  STATUS_SEE_OTHER                       = 303,
  STATUS_NOT_MODIFIED                    = 304,
  STATUS_USE_PROXY                       = 305,
  STATUS_TEMPORARY_REDIRECT              = 307,
  // 4xx
  STATUS_BAD_REQUEST                     = 400,
  STATUS_UNAUTHORIZED                    = 401,
  STATUS_PAYMENT_REQUIRED                = 402,
  STATUS_FORBIDDEN                       = 403,
  STATUS_NOT_FOUND                       = 404,
  STATUS_METHOD_NOT_ALLOWED              = 405,
  STATUS_NOT_ACCEPTABLE                  = 406,
  STATUS_PROXY_AUTH_REQUIRED             = 407,
  STATUS_REQUEST_TIMEOUT                 = 408,
  STATUS_CONFLICT                        = 409,
  STATUS_GONE                            = 410,
  STATUS_LENGTH_REQUIRED                 = 411,
  STATUS_PRECONDITION_FAILED             = 412,
  STATUS_REQUEST_ENTITY_TOO_LARGE        = 413,
  STATUS_URI_TOO_LONG                    = 414,
  STATUS_UNSUPPORTED_MEDIA_TYPE          = 415,
  STATUS_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
  STATUS_EXPECTATION_FAILED              = 417,
  // 5xx
  STATUS_INTERNAL_SERVER_ERROR           = 500,
  STATUS_NOT_IMPLEMENTED                 = 501,
  STATUS_BAD_GATEWAY                     = 502,
  STATUS_SERVICE_UNAVAILABLE             = 503,
  STATUS_GATEWAY_TIMEOUT                 = 504,
  STATUS_HTTP_VERSION_NOT_SUPPORTED      = 505
} Status;

typedef enum
{
  METHOD_OPTIONS,
  METHOD_GET,
  METHOD_HEAD,
  METHOD_POST,
  METHOD_PUT,
  METHOD_DELETE,
  METHOD_TRACE,
  METHOD_CONNECT,
  METHOD_PATCH
} Method;

typedef struct
{
  HK_USERDATA_HEADER
  Socket sock;
  SocketAddress addr;
} SocketWrapper;

typedef struct
{
  SocketWrapper wrap;
  HkClosure *requestHandler;
} ServerUserdata;

typedef struct
{
  SocketWrapper wrap;
} ClientUserdata;

static HkStruct *requestStruct = NULL;
static HkStruct *responseStruct = NULL;

static inline HkStruct *get_request_struct(void);
static inline HkStruct *get_response_struct(void);
static inline void socket_wrapper_init(SocketWrapper *wrap, void (*deinit)(HkUserdata *),
  Socket sock);
static inline void socket_wrapper_deinit(SocketWrapper *wrap);
static inline ServerUserdata *server_userdata_new(void);
static inline ClientUserdata *client_userdata_new(void);
static inline const char *status_reason(Status status);
static inline const char *method_name(Method method);
static inline void loop(HkState *state, ServerUserdata *server, HkString *ip,
  int port);
static inline void server_listen(HkState *state, ServerUserdata *server, HkString *ip,
  int port);
static inline void handle_request(HkState *state, ServerUserdata *server,
  Socket clientSock);
static inline HkInstance *parse_request(HkState *state, Socket sock);
static inline HkInstance *new_default_response(void);
static inline void add_content_length_header(HkArray *headers, int contentLength);
static inline void write_response(HkState *state, Socket sock, HkInstance *res);
static inline void write_status_line(Socket sock, int status);
static inline void write_headers(HkState *state, Socket sock, HkArray *headers);
static inline void write_body(Socket sock, HkString *body);
static void server_userdata_deinit(HkUserdata *udata);
static void client_userdata_deinit(HkUserdata *udata);
static void new_server_call(HkState *state, HkValue *args);
static void new_client_call(HkState *state, HkValue *args);
static void close_call(HkState *state, HkValue *args);
static void set_request_handler_call(HkState *state, HkValue *args);
static void listen_call(HkState *state, HkValue *args);
static void send_request_call(HkState *state, HkValue *args);

static inline HkStruct *get_request_struct(void)
{
  if (!requestStruct)
  {
    requestStruct = hk_struct_new(hk_string_from_chars(-1, "Request"));
    (void) hk_struct_define_field(requestStruct, hk_string_from_chars(-1, "method"));
    (void) hk_struct_define_field(requestStruct, hk_string_from_chars(-1, "path"));
    (void) hk_struct_define_field(requestStruct, hk_string_from_chars(-1, "headers"));
    (void) hk_struct_define_field(requestStruct, hk_string_from_chars(-1, "body"));
  }
  return requestStruct;
}

static inline HkStruct *get_response_struct(void)
{
  if (!responseStruct)
  {
    responseStruct = hk_struct_new(hk_string_from_chars(-1, "Response"));
    (void) hk_struct_define_field(responseStruct, hk_string_from_chars(-1, "status"));
    (void) hk_struct_define_field(responseStruct, hk_string_from_chars(-1, "headers"));
    (void) hk_struct_define_field(responseStruct, hk_string_from_chars(-1, "body"));
  }
  return responseStruct;
}

static inline void socket_wrapper_init(SocketWrapper *wrap, void (*deinit)(HkUserdata *),
  Socket sock)
{
  hk_userdata_init((HkUserdata *) wrap, deinit);
  wrap->sock = sock;
  wrap->addr = (SocketAddress) { 0 };
}

static inline void socket_wrapper_deinit(SocketWrapper *wrap)
{
  if (wrap->sock == INVALID_SOCKET)
    return;
  socket_close(wrap->sock);
}

static inline ServerUserdata *server_userdata_new(void)
{
  Socket sock = socket_create(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET)
    return NULL;
  ServerUserdata *server = (ServerUserdata *) hk_allocate(sizeof(*server));
  socket_wrapper_init(&server->wrap, server_userdata_deinit, sock);
  server->requestHandler = NULL;
  return server;
}

static inline ClientUserdata *client_userdata_new(void)
{
  Socket sock = socket_create(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET)
    return NULL;
  ClientUserdata *client = (ClientUserdata *) hk_allocate(sizeof(*client));
  socket_wrapper_init(&client->wrap, client_userdata_deinit, sock);
  return client;
}

static inline const char *status_reason(Status status)
{
  switch (status)
  {
  case STATUS_CONTINUE:
    return "Continue";
  case STATUS_SWITCHING_PROTOCOLS:
    return "Switching Protocols";
  case STATUS_OK:
    return "OK";
  case STATUS_CREATED:
    return "Created";
  case STATUS_ACCEPTED:
    return "Accepted";
  case STATUS_NON_AUTHORITATIVE_INFO:
    return "Non-Authoritative Information";
  case STATUS_NO_CONTENT:
    return "No Content";
  case STATUS_RESET_CONTENT:
    return "Reset Content";
  case STATUS_PARTIAL_CONTENT:
    return "Partial Content";
  case STATUS_MULTIPLE_CHOICES:
    return "Multiple Choices";
  case STATUS_MOVED_PERMANENTLY:
    return "Moved Permanently";
  case STATUS_FOUND:
    return "Found";
  case STATUS_SEE_OTHER:
    return "See Other";
  case STATUS_NOT_MODIFIED:
    return "Not Modified";
  case STATUS_USE_PROXY:
    return "Use Proxy";
  case STATUS_TEMPORARY_REDIRECT:
    return "Temporary Redirect";
  case STATUS_BAD_REQUEST:
    return "Bad Request";
  case STATUS_UNAUTHORIZED:
    return "Unauthorized";
  case STATUS_PAYMENT_REQUIRED:
    return "Payment Required";
  case STATUS_FORBIDDEN:
    return "Forbidden";
  case STATUS_NOT_FOUND:
    return "Not Found";
  case STATUS_METHOD_NOT_ALLOWED:
    return "Method Not Allowed";
  case STATUS_NOT_ACCEPTABLE:
    return "Not Acceptable";
  case STATUS_PROXY_AUTH_REQUIRED:
    return "Proxy Authentication Required";
  case STATUS_REQUEST_TIMEOUT:
    return "Request Time-out";
  case STATUS_CONFLICT:
    return "Conflict";
  case STATUS_GONE:
    return "Gone";
  case STATUS_LENGTH_REQUIRED:
    return "Length Required";
  case STATUS_PRECONDITION_FAILED:
    return "Precondition Failed";
  case STATUS_REQUEST_ENTITY_TOO_LARGE:
    return "Request Entity Too Large";
  case STATUS_URI_TOO_LONG:
    return "Request-URI Too Large";
  case STATUS_UNSUPPORTED_MEDIA_TYPE:
    return "Unsupported Media Type";
  case STATUS_REQUESTED_RANGE_NOT_SATISFIABLE:
    return "Requested range not satisfiable";
  case STATUS_EXPECTATION_FAILED:
    return "Expectation Failed";
  case STATUS_INTERNAL_SERVER_ERROR:
    return "Internal Server Error";
  case STATUS_NOT_IMPLEMENTED:
    return "Not Implemented";
  case STATUS_BAD_GATEWAY:
    return "Bad Gateway";
  case STATUS_SERVICE_UNAVAILABLE:
    return "Service Unavailable";
  case STATUS_GATEWAY_TIMEOUT:
    return "Gateway Time-out";
  case STATUS_HTTP_VERSION_NOT_SUPPORTED:
    return "HTTP Version not supported";
  }
  return NULL;
}

static inline const char *method_name(Method method)
{
  switch (method)
  {
  case METHOD_OPTIONS: return "OPTIONS";
  case METHOD_GET:     return "GET";
  case METHOD_HEAD:    return "HEAD";
  case METHOD_POST:    return "POST";
  case METHOD_PUT:     return "PUT";
  case METHOD_DELETE:  return "DELETE";
  case METHOD_TRACE:   return "TRACE";
  case METHOD_CONNECT: return "CONNECT";
  case METHOD_PATCH:   return "PATCH";
  }
  return NULL;
}

static inline void loop(HkState *state, ServerUserdata *server, HkString *ip,
  int port)
{
  server_listen(state, server, ip, port);
  hk_return_if_not_ok(state);
  for (;;)
  {
    SocketAddress clientAddr;
    Socket clientSock = socket_accept(server->wrap.sock, &clientAddr);
    if (clientSock == INVALID_SOCKET)
    {
      hk_state_runtime_error(state, "Unable to accept incoming connection");
      return;
    }
    handle_request(state, server, clientSock);
    hk_return_if_not_ok(state);
    socket_close(clientSock);
  }
}

static inline void server_listen(HkState *state, ServerUserdata *server, HkString *ip,
  int port)
{
  SocketWrapper *wrap = &server->wrap;
  Socket sock = wrap->sock;
  int val = 1;
  int rc = socket_set_option(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
  const char *err = "Unable to listen on the given address";
  if (rc == SOCKET_ERROR)
  {
    hk_state_runtime_error(state, err);
    socket_close(sock);
    return;
  }
  SocketAddress *addr = &wrap->addr;
  socket_address_init(addr, AF_INET, ip->chars, port);
  if (socket_bind(sock, addr) == SOCKET_ERROR)
  {
    hk_state_runtime_error(state, err);
    socket_close(sock);
    return;
  }
  if (socket_listen(sock, BACKLOG) == SOCKET_ERROR)
  {
    hk_state_runtime_error(state, err);
    socket_close(sock);
  }
}

static inline void handle_request(HkState *state, ServerUserdata *server,
  Socket clientSock)
{
  HkClosure *requestHandler = server->requestHandler;
  HkInstance *req = parse_request(state, clientSock);
  hk_return_if_not_ok(state);
  if (!req)
    return;
  HkInstance *res = new_default_response();
  hk_state_push_closure(state, requestHandler);
  hk_return_if_not_ok(state);
  hk_state_push_instance(state, req);
  hk_return_if_not_ok(state);
  hk_state_push_instance(state, res);
  hk_return_if_not_ok(state);
  hk_state_call(state, 2);
  HkValue result = state->stackSlots[state->stackTop];
  const char *err = "Invalid response";
  if (result.type != HK_TYPE_INSTANCE)
  {
    hk_state_runtime_error(state, err);
    return;
  }
  res = hk_as_instance(result);
  // TODO: Use duck typing instead of comparing pointers.
  if (res->ztruct != get_response_struct())
  {
    hk_state_runtime_error(state, err);
    return;
  }
  write_response(state, clientSock, res);
  hk_state_pop(state);
}

static inline HkInstance *parse_request(HkState *state, Socket sock)
{
  char buf[BUFFER_SIZE];
  int bufLen = 0;
  const char *method;
  size_t methodLen;
  const char *path;
  size_t pathLen;
  int minorVersion;
  struct phr_header headers[MAX_HEADERS];
  size_t numHeaders;
  size_t prevBufLen = 0;
  int n;
  for (;;)
  {
    n = socket_recv(sock, &buf[bufLen], sizeof(buf) - bufLen, 0);
    if (!n)
      return NULL;
    if (n == SOCKET_ERROR)
    {
      hk_state_runtime_error(state, "Unable to receive data from the client");
      return NULL;
    }
    prevBufLen = bufLen;
    bufLen += n;
    numHeaders = sizeof(headers) / sizeof(headers[0]);
    n = phr_parse_request(buf, bufLen, &method, &methodLen,
      &path, &pathLen, &minorVersion, headers, &numHeaders, prevBufLen);
    if (n > 0)
      break; // Request parsed
    if (n == -1)
    {
      hk_state_runtime_error(state, "Unable to parse the request");
      return NULL;
    }
    // Incomplete request
    assert(n == -2);
    if (bufLen == BUFFER_SIZE)
    {
      hk_state_runtime_error(state, "Request too large");
      return NULL;
    }
  }
  HkString *methodStr = hk_string_from_chars(methodLen, method);
  HkString *pathStr = hk_string_from_chars(pathLen, path);
  HkArray *headersArr = hk_array_new_with_capacity(numHeaders);
  for (size_t i = 0; i < numHeaders; ++i)
  {
    struct phr_header *header = &headers[i];
    HkString *headerStr = hk_string_from_chars(header->name_len, header->name);
    hk_string_inplace_concat_chars(headerStr, 2, ": ");
    hk_string_inplace_concat_chars(headerStr, header->value_len, header->value);
    hk_array_inplace_add_element(headersArr, hk_string_value(headerStr));
  }
  // TODO: Use Content-Length header to read the body.
  int length = bufLen - n;
  char *chars = &buf[n];
  HkString *bodyStr = hk_string_from_chars(length, chars);
  HkInstance *req = hk_instance_new(get_request_struct());
  hk_incr_ref(methodStr);
  req->values[0] = hk_string_value(methodStr);
  hk_incr_ref(pathStr);
  req->values[1] = hk_string_value(pathStr);
  hk_incr_ref(headersArr);
  req->values[2] = hk_array_value(headersArr);
  hk_incr_ref(bodyStr);
  req->values[3] = hk_string_value(bodyStr);
  return req;
}

static inline HkInstance *new_default_response(void)
{
  HkArray *headers = hk_array_new_with_capacity(1);
  // TODO: Add default headers.
  //HkString *header = hk_string_from_chars(-1, "Content-Type: text/plain");
  //hk_array_inplace_add_element(headers, hk_string_value(header));
  HkString *body = hk_string_new();
  HkInstance *res = hk_instance_new(get_response_struct());
  res->values[0] = hk_number_value(STATUS_OK);
  hk_incr_ref(headers);
  res->values[1] = hk_array_value(headers);
  hk_incr_ref(body);
  res->values[2] = hk_string_value(body);
  return res;
}

static inline void add_content_length_header(HkArray *headers, int contentLength)
{
  HkString *header = hk_string_from_chars(-1, "Content-Length: ");
  char buf[16];
  int len = snprintf(buf, sizeof(buf), "%d", contentLength);
  hk_string_inplace_concat_chars(header, len, buf);
  hk_array_inplace_add_element(headers, hk_string_value(header));
}

static inline void write_response(HkState *state, Socket sock, HkInstance *res)
{
  int status = (int) hk_as_number(res->values[0]);
  HkArray *headers = hk_as_array(res->values[1]);
  HkString *body = hk_as_string(res->values[2]);
  add_content_length_header(headers, body->length);
  write_status_line(sock, status);
  write_headers(state, sock, headers);
  hk_return_if_not_ok(state);
  write_body(sock, body);
}

static inline void write_status_line(Socket sock, int status)
{
  (void) socket_send_all(sock, "HTTP/1.1 ", 9, 0);
  char buf[16];
  int len = snprintf(buf, sizeof(buf), "%d ", status);
  (void) socket_send_all(sock, buf, len, 0);
  const char *reason = status_reason((Status) status);
  (void) socket_send_all(sock, reason, strlen(reason), 0);
  (void) socket_send_all(sock, "\r\n", 2, 0);
}

static inline void write_headers(HkState *state, Socket sock, HkArray *headers)
{
  int n = headers->length;
  for (int i = 0; i < n; ++i)
  {
    HkValue elem = hk_array_get_element(headers, i);
    if (elem.type != HK_TYPE_STRING)
    {
      hk_state_runtime_error(state, "Invalid header value");
      return;
    }
    HkString *header = hk_as_string(elem);
    (void) socket_send_all(sock, header->chars, header->length, 0);
    (void) socket_send_all(sock, "\r\n", 2, 0);
  }
  (void) socket_send_all(sock, "\r\n", 2, 0);
}

static inline void write_body(Socket sock, HkString *body)
{
  if (!body->length)
    return;
  (void) socket_send_all(sock, body->chars, body->length, 0);
}

static void server_userdata_deinit(HkUserdata *udata)
{
  SocketWrapper *wrap = (SocketWrapper *) udata;
  socket_wrapper_deinit(wrap);
  ServerUserdata *server = (ServerUserdata *) wrap;
  hk_closure_release(server->requestHandler);
}

static void client_userdata_deinit(HkUserdata *udata)
{
  socket_wrapper_deinit((SocketWrapper *) udata);
}

static void new_server_call(HkState *state, HkValue *args)
{
  (void) args;
  ServerUserdata *server = server_userdata_new();
  if (!server)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) &server->wrap);
}

static void new_client_call(HkState *state, HkValue *args)
{
  (void) args;
  ClientUserdata *client = client_userdata_new();
  if (!client)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) &client->wrap);
}

static void close_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  SocketWrapper *wrap = (SocketWrapper *) hk_as_userdata(args[1]);
  if (wrap->sock != INVALID_SOCKET)
  {
    socket_close(wrap->sock);
    wrap->sock = INVALID_SOCKET;
  }
  hk_state_push_nil(state);
}

static void set_request_handler_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_callable(state, args, 2);
  hk_return_if_not_ok(state);
  ServerUserdata *server = ((ServerUserdata *) hk_as_userdata(args[1]));
  HkClosure *cl = hk_as_closure(args[2]);
  hk_incr_ref(cl);
  server->requestHandler = cl;
  hk_state_push_nil(state);
}

static void listen_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  ServerUserdata *server = (ServerUserdata *) hk_as_userdata(args[1]);
  HkString *ip = hk_as_string(args[2]);
  int port = (int) hk_as_number(args[3]);
  loop(state, server, ip, port);
  hk_return_if_not_ok(state);
  // TODO: return error as a string.
  hk_state_push_nil(state);
}

static void send_request_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 3);
  hk_return_if_not_ok(state);
  HkType headersTypes[] = { HK_TYPE_NIL, HK_TYPE_ARRAY };
  hk_state_check_argument_types(state, args, 4, 2, headersTypes);
  hk_return_if_not_ok(state);
  HkType bodyTypes[] = { HK_TYPE_NIL, HK_TYPE_STRING };
  hk_state_check_argument_types(state, args, 5, 2, bodyTypes);
  hk_return_if_not_ok(state);
  Socket sock = ((ClientUserdata *) hk_as_userdata(args[1]))->wrap.sock;
  int method = (int) hk_as_number(args[2]);
  HkString *path = hk_as_string(args[3]);
  HkArray *headers = hk_as_array(args[4]);
  HkString *body = hk_as_string(args[5]);
  // TODO: Implement this function.
  (void) sock;
  (void) method;
  (void) path;
  (void) headers;
  (void) body;
  (void) method_name;
  HkArray *result = hk_array_new_with_capacity(2);
  HkString *err = hk_string_from_chars(-1, "Not implemented yet");
  hk_array_inplace_add_element(result, HK_NIL_VALUE);
  hk_array_inplace_add_element(result, hk_string_value(err));
  hk_state_push_array(state, result);
}

HK_LOAD_MODULE_HANDLER(http)
{
  hk_state_push_string_from_chars(state, -1, "http");
  hk_return_if_not_ok(state);
  // Status constants
  hk_state_push_string_from_chars(state, -1, "STATUS_CONTINUE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_CONTINUE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_SWITCHING_PROTOCOLS");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_SWITCHING_PROTOCOLS);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_OK");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_OK);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_CREATED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_CREATED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_ACCEPTED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_ACCEPTED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_NON_AUTHORITATIVE_INFO");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_NON_AUTHORITATIVE_INFO);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_NO_CONTENT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_NO_CONTENT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_RESET_CONTENT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_RESET_CONTENT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_PARTIAL_CONTENT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_PARTIAL_CONTENT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_MULTIPLE_CHOICES");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_MULTIPLE_CHOICES);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_MOVED_PERMANENTLY");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_MOVED_PERMANENTLY);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_FOUND");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_FOUND);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_SEE_OTHER");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_SEE_OTHER);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_NOT_MODIFIED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_NOT_MODIFIED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_USE_PROXY");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_USE_PROXY);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_TEMPORARY_REDIRECT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_TEMPORARY_REDIRECT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_BAD_REQUEST");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_BAD_REQUEST);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_UNAUTHORIZED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_UNAUTHORIZED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_PAYMENT_REQUIRED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_PAYMENT_REQUIRED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_FORBIDDEN");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_FORBIDDEN);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_NOT_FOUND");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_NOT_FOUND);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_METHOD_NOT_ALLOWED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_METHOD_NOT_ALLOWED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_NOT_ACCEPTABLE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_NOT_ACCEPTABLE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_PROXY_AUTH_REQUIRED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_PROXY_AUTH_REQUIRED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_REQUEST_TIMEOUT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_REQUEST_TIMEOUT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_CONFLICT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_CONFLICT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_GONE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_GONE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_LENGTH_REQUIRED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_LENGTH_REQUIRED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_PRECONDITION_FAILED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_PRECONDITION_FAILED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_REQUEST_ENTITY_TOO_LARGE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_REQUEST_ENTITY_TOO_LARGE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_URI_TOO_LONG");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_URI_TOO_LONG);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_UNSUPPORTED_MEDIA_TYPE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_UNSUPPORTED_MEDIA_TYPE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_REQUESTED_RANGE_NOT_SATISFIABLE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_REQUESTED_RANGE_NOT_SATISFIABLE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_EXPECTATION_FAILED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_EXPECTATION_FAILED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_INTERNAL_SERVER_ERROR");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_INTERNAL_SERVER_ERROR);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_NOT_IMPLEMENTED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_NOT_IMPLEMENTED);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_BAD_GATEWAY");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_BAD_GATEWAY);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_SERVICE_UNAVAILABLE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_SERVICE_UNAVAILABLE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_GATEWAY_TIMEOUT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_GATEWAY_TIMEOUT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "STATUS_HTTP_VERSION_NOT_SUPPORTED");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, STATUS_HTTP_VERSION_NOT_SUPPORTED);
  hk_return_if_not_ok(state);
  // Method constants
  hk_state_push_string_from_chars(state, -1, "METHOD_OPTIONS");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_OPTIONS);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "METHOD_GET");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_GET);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "METHOD_HEAD");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_HEAD);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "METHOD_POST");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_POST);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "METHOD_PUT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_PUT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "METHOD_DELETE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_DELETE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "METHOD_TRACE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_TRACE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "METHOD_CONNECT");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_CONNECT);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "METHOD_PATCH");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, METHOD_PATCH);
  hk_return_if_not_ok(state);
  // Structs
  hk_state_push_string_from_chars(state, -1, "Request");
  hk_return_if_not_ok(state);
  hk_state_push_struct(state, get_request_struct());
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "Response");
  hk_return_if_not_ok(state);
  hk_state_push_struct(state, get_response_struct());
  hk_return_if_not_ok(state);
  // Functions
  hk_state_push_string_from_chars(state, -1, "new_server");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_server", 0, new_server_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new_client");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_client", 0, new_client_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "close");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "close", 1, close_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "set_request_handler");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "set_request_handler", 2, set_request_handler_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "listen");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "listen", 3, listen_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "send_request");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "send_request", 5, send_request_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 57);
}
