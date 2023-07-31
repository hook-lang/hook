//
// The Hook Programming Language
// lists.c
//

#include "lists.h"
#include <stdlib.h>
#include <hook/memory.h>
#include <hook/check.h>
#include <hook/status.h>

typedef struct LinkedListNode
{
  struct LinkedListNode *next;
  struct LinkedListNode *prev;
  HkValue elem;
} LinkedListNode;

typedef struct
{
  HK_USERDATA_HEADER
  int length;
  LinkedListNode *head;
  LinkedListNode *tail;
} LinkedList;

static inline LinkedListNode *LinkedListNode_new(HkValue elem);
static inline void LinkedListNode_free(LinkedListNode *node);
static inline LinkedList *linked_list_new(void);
static inline LinkedList *linked_list_copy(LinkedList *list);
static inline void linked_list_inplace_push_front(LinkedList *list, HkValue elem);
static inline void linked_list_inplace_push_back(LinkedList *list, HkValue elem);
static void linked_list_deinit(HkUserdata *udata);
static int new_linked_list_call(HkState *state, HkValue *args);
static int len_call(HkState *state, HkValue *args);
static int is_empty_call(HkState *state, HkValue *args);
static int push_front_call(HkState *state, HkValue *args);
static int push_back_call(HkState *state, HkValue *args);
static int pop_front_call(HkState *state, HkValue *args);
static int pop_back_call(HkState *state, HkValue *args);
static int front_call(HkState *state, HkValue *args);
static int back_call(HkState *state, HkValue *args);

static inline LinkedListNode *LinkedListNode_new(HkValue elem)
{
  LinkedListNode *node = (LinkedListNode *) hk_allocate(sizeof(*node));
  hk_value_incr_ref(elem);
  node->next = NULL;
  node->prev = NULL;
  node->elem = elem;
  return node;
}

static inline void LinkedListNode_free(LinkedListNode *node)
{
  hk_value_release(node->elem);
  free(node);
}

static inline LinkedList *linked_list_new(void)
{
  LinkedList *list = (LinkedList *) hk_allocate(sizeof(*list));
  hk_userdata_init((HkUserdata *) list, &linked_list_deinit);
  list->length = 0;
  list->head = NULL;
  list->tail = NULL;
  return list;
}

static inline LinkedList *linked_list_copy(LinkedList *list)
{
  LinkedList *result = linked_list_new();
  LinkedListNode *node = list->head;
  while (node)
  {
    linked_list_inplace_push_back(result, node->elem);
    node = node->next;
  }
  return result;
}

static inline void linked_list_inplace_push_front(LinkedList *list, HkValue elem)
{
  LinkedListNode *node = LinkedListNode_new(elem);
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

static inline void linked_list_inplace_push_back(LinkedList *list, HkValue elem)
{
  LinkedListNode *node = LinkedListNode_new(elem);
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

static void linked_list_deinit(HkUserdata *udata)
{
  LinkedList *list = (LinkedList *) udata;
  LinkedListNode *node = list->head;
  while (node)
  {
    LinkedListNode *next = node->next;
    LinkedListNode_free(node);
    node = next;
  }
}

static int new_linked_list_call(HkState *state, HkValue *args)
{
  (void) args;
  return hk_state_push_userdata(state, (HkUserdata *) linked_list_new());
}

static int len_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  return hk_state_push_number(state, list->length);
}

static int is_empty_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  return hk_state_push_bool(state, !list->length);
}

static int push_front_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  HkValue elem = args[2];
  LinkedList *result = linked_list_copy(list);
  linked_list_inplace_push_front(result, elem);
  return hk_state_push_userdata(state, (HkUserdata *) result);
}

static int push_back_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  HkValue elem = args[2];
  LinkedList *result = linked_list_copy(list);
  linked_list_inplace_push_back(result, elem);
  return hk_state_push_userdata(state, (HkUserdata *) result);
}

static int pop_front_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  LinkedList *result = linked_list_new();
  if (!list->head)
    goto end;
  LinkedListNode *node = list->head->next;
  while (node)
  {
    linked_list_inplace_push_back(result, node->elem);
    node = node->next;
  }
end:
  return hk_state_push_userdata(state, (HkUserdata *) result);
}

static int pop_back_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  LinkedList *result = linked_list_new();
  if (!list->tail)
    goto end;
  LinkedListNode *node = list->tail->prev;
  while (node)
  {
    linked_list_inplace_push_front(result, node->elem);
    node = node->prev;
  }
end:
  return hk_state_push_userdata(state, (HkUserdata *) result);
}

static int front_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  HkValue elem = list->head ? list->head->elem : HK_NIL_VALUE;
  return hk_state_push(state, elem);
}

static int back_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  HkValue elem = list->tail ? list->tail->elem : HK_NIL_VALUE;
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
