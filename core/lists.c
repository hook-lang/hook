//
// The Hook Programming Language
// lists.c
//

#include "lists.h"
#include <stdlib.h>
#include <hook/memory.h>
#include <hook/check.h>
#include <hook/status.h>

typedef struct linked_list_node
{
  struct linked_list_node *next;
  struct linked_list_node *prev;
  hk_value_t elem;
} linked_list_node_t;

typedef struct
{
  HK_USERDATA_HEADER
  int32_t length;
  linked_list_node_t *head;
  linked_list_node_t *tail;
} linked_list_t;

static inline linked_list_node_t *linked_list_node_new(hk_value_t elem);
static inline void linked_list_node_free(linked_list_node_t *node);
static inline linked_list_t *linked_list_new(void);
static inline linked_list_t *linked_list_copy(linked_list_t *list);
static inline void linked_list_inplace_push_front(linked_list_t *list, hk_value_t elem);
static inline void linked_list_inplace_push_back(linked_list_t *list, hk_value_t elem);
static void linked_list_deinit(hk_userdata_t *udata);
static int32_t new_linked_list_call(hk_state_t *state, hk_value_t *args);
static int32_t len_call(hk_state_t *state, hk_value_t *args);
static int32_t is_empty_call(hk_state_t *state, hk_value_t *args);
static int32_t push_front_call(hk_state_t *state, hk_value_t *args);
static int32_t push_back_call(hk_state_t *state, hk_value_t *args);
static int32_t pop_front_call(hk_state_t *state, hk_value_t *args);
static int32_t pop_back_call(hk_state_t *state, hk_value_t *args);
static int32_t front_call(hk_state_t *state, hk_value_t *args);
static int32_t back_call(hk_state_t *state, hk_value_t *args);

static inline linked_list_node_t *linked_list_node_new(hk_value_t elem)
{
  linked_list_node_t *node = (linked_list_node_t *) hk_allocate(sizeof(*node));
  hk_value_incr_ref(elem);
  node->next = NULL;
  node->prev = NULL;
  node->elem = elem;
  return node;
}

static inline void linked_list_node_free(linked_list_node_t *node)
{
  hk_value_release(node->elem);
  free(node);
}

static inline linked_list_t *linked_list_new(void)
{
  linked_list_t *list = (linked_list_t *) hk_allocate(sizeof(*list));
  hk_userdata_init((hk_userdata_t *) list, &linked_list_deinit);
  list->length = 0;
  list->head = NULL;
  list->tail = NULL;
  return list;
}

static inline linked_list_t *linked_list_copy(linked_list_t *list)
{
  linked_list_t *result = linked_list_new();
  linked_list_node_t *node = list->head;
  while (node)
  {
    linked_list_inplace_push_back(result, node->elem);
    node = node->next;
  }
  return result;
}

static inline void linked_list_inplace_push_front(linked_list_t *list, hk_value_t elem)
{
  linked_list_node_t *node = linked_list_node_new(elem);
  if (!list->head)
  {
    list->length = 1;
    list->head = node;
    list->tail = node;
    return;
  }
  ++list->length;
  list->head->prev = node;
  node->next = list->head;
  list->head = node;
}

static inline void linked_list_inplace_push_back(linked_list_t *list, hk_value_t elem)
{
  linked_list_node_t *node = linked_list_node_new(elem);
  if (!list->tail)
  {
    list->length = 1;
    list->head = node;
    list->tail = node;
    return;
  }
  ++list->length;
  list->tail->next = node;
  node->prev = list->tail;
  list->tail = node;
}

static void linked_list_deinit(hk_userdata_t *udata)
{
  linked_list_t *list = (linked_list_t *) udata;
  linked_list_node_t *node = list->head;
  while (node)
  {
    linked_list_node_t *next = node->next;
    linked_list_node_free(node);
    node = next;
  }
}

static int32_t new_linked_list_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  return hk_state_push_userdata(state, (hk_userdata_t *) linked_list_new());
}

static int32_t len_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  linked_list_t *list = (linked_list_t *) hk_as_userdata(args[1]);
  return hk_state_push_number(state, list->length);
}

static int32_t is_empty_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  linked_list_t *list = (linked_list_t *) hk_as_userdata(args[1]);
  return hk_state_push_bool(state, !list->length);
}

static int32_t push_front_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  linked_list_t *list = (linked_list_t *) hk_as_userdata(args[1]);
  hk_value_t elem = args[2];
  linked_list_t *result = linked_list_copy(list);
  linked_list_inplace_push_front(result, elem);
  return hk_state_push_userdata(state, (hk_userdata_t *) result);
}

static int32_t push_back_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  linked_list_t *list = (linked_list_t *) hk_as_userdata(args[1]);
  hk_value_t elem = args[2];
  linked_list_t *result = linked_list_copy(list);
  linked_list_inplace_push_back(result, elem);
  return hk_state_push_userdata(state, (hk_userdata_t *) result);
}

static int32_t pop_front_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  linked_list_t *list = (linked_list_t *) hk_as_userdata(args[1]);
  linked_list_t *result = linked_list_new();
  if (!list->head)
    goto end;
  linked_list_node_t *node = list->head->next;
  while (node)
  {
    linked_list_inplace_push_back(result, node->elem);
    node = node->next;
  }
end:
  return hk_state_push_userdata(state, (hk_userdata_t *) result);
}

static int32_t pop_back_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  linked_list_t *list = (linked_list_t *) hk_as_userdata(args[1]);
  linked_list_t *result = linked_list_new();
  if (!list->tail)
    goto end;
  linked_list_node_t *node = list->tail->prev;
  while (node)
  {
    linked_list_inplace_push_front(result, node->elem);
    node = node->prev;
  }
end:
  return hk_state_push_userdata(state, (hk_userdata_t *) result);
}

static int32_t front_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  linked_list_t *list = (linked_list_t *) hk_as_userdata(args[1]);
  hk_value_t elem = list->head ? list->head->elem : HK_NIL_VALUE;
  return hk_state_push(state, elem);
}

static int32_t back_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  linked_list_t *list = (linked_list_t *) hk_as_userdata(args[1]);
  hk_value_t elem = list->tail ? list->tail->elem : HK_NIL_VALUE;
  return hk_state_push(state, elem);
}

HK_LOAD_FN(lists)
{
  if (hk_state_push_string_from_chars(state, -1, "lists") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new_linked_list") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new_linked_list", 0, &new_linked_list_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "len") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "len", 1, &len_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "is_empty") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "is_empty", 1, &is_empty_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "push_front") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "push_front", 2, &push_front_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "push_back") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "push_back", 2, &push_back_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "pop_front") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "pop_front", 1, &pop_front_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "pop_back") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "pop_back", 1, &pop_back_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "front") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "front", 1, &front_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "back") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "back", 1, &back_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 9);
}
