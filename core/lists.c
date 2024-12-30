//
// lists.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "lists.h"
#include <stdlib.h>

typedef struct LinkedListNode
{
  struct LinkedListNode *next;
  struct LinkedListNode *prev;
  HkValue               elem;
} LinkedListNode;

typedef struct
{
  HK_USERDATA_HEADER
  int            length;
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
static void new_linked_list_call(HkVM *vm, HkValue *args);
static void len_call(HkVM *vm, HkValue *args);
static void is_empty_call(HkVM *vm, HkValue *args);
static void push_front_call(HkVM *vm, HkValue *args);
static void push_back_call(HkVM *vm, HkValue *args);
static void pop_front_call(HkVM *vm, HkValue *args);
static void pop_back_call(HkVM *vm, HkValue *args);
static void front_call(HkVM *vm, HkValue *args);
static void back_call(HkVM *vm, HkValue *args);

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
  hk_free(node);
}

static inline LinkedList *linked_list_new(void)
{
  LinkedList *list = (LinkedList *) hk_allocate(sizeof(*list));
  hk_userdata_init((HkUserdata *) list, linked_list_deinit);
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

static void new_linked_list_call(HkVM *vm, HkValue *args)
{
  (void) args;
  hk_vm_push_userdata(vm, (HkUserdata *) linked_list_new());
}

static void len_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  hk_vm_push_number(vm, list->length);
}

static void is_empty_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  hk_vm_push_bool(vm, !list->length);
}

static void push_front_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  HkValue elem = args[2];
  LinkedList *result = linked_list_copy(list);
  linked_list_inplace_push_front(result, elem);
  hk_vm_push_userdata(vm, (HkUserdata *) result);
}

static void push_back_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  HkValue elem = args[2];
  LinkedList *result = linked_list_copy(list);
  linked_list_inplace_push_back(result, elem);
  hk_vm_push_userdata(vm, (HkUserdata *) result);
}

static void pop_front_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
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
  hk_vm_push_userdata(vm, (HkUserdata *) result);
}

static void pop_back_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
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
  hk_vm_push_userdata(vm, (HkUserdata *) result);
}

static void front_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  HkValue elem = list->head ? list->head->elem : HK_NIL_VALUE;
  hk_vm_push(vm, elem);
}

static void back_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  LinkedList *list = (LinkedList *) hk_as_userdata(args[1]);
  HkValue elem = list->tail ? list->tail->elem : HK_NIL_VALUE;
  hk_vm_push(vm, elem);
}

HK_LOAD_MODULE_HANDLER(lists)
{
  hk_vm_push_string_from_chars(vm, -1, "lists");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new_linked_list");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new_linked_list", 0, new_linked_list_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "len");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "len", 1, len_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "is_empty");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "is_empty", 1, is_empty_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "push_front");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "push_front", 2, push_front_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "push_back");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "push_back", 2, push_back_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "pop_front");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "pop_front", 1, pop_front_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "pop_back");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "pop_back", 1, pop_back_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "front");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "front", 1, front_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "back");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "back", 1, back_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 9);
}
