//
// Hook Programming Language
// disasm.c
//

#include "disasm.h"
#include <stdio.h>

static inline void dump_prototype(prototype_t *proto);

static inline void dump_prototype(prototype_t *proto)
{
  string_t *name = proto->name;
  if (name)
    printf("<function %.*s>\n", name->length, name->chars);
  else
    printf("<function>\n");
  uint8_t *bytes = proto->chunk.bytes;
  int i = 0;
  int n = 0;
  while (i < proto->chunk.length)
  {
    opcode_t op = (opcode_t) bytes[i];
    int j = i++;
    ++n;
    switch (op)
    {
    case OP_NULL:
      printf("[%05d] Null\n", j);
      break;
    case OP_FALSE:
      printf("[%05d] False\n", j);
      break;
    case OP_TRUE:
      printf("[%05d] True\n", j);
      break;
    case OP_INT:
      {
        int data = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("[%05d] Int               %d\n", j, data);
      }
      break;
    case OP_CONSTANT:
      printf("[%05d] Constant          %d\n", j, bytes[i++]);
      break;
    case OP_ARRAY:
      printf("[%05d] Array             %d\n", j, bytes[i++]);
      break;
    case OP_INSTANCE:
      printf("[%05d] Instance\n", j);
      break;
    case OP_INITILIZE:
      printf("[%05d] Initialize        %d\n", j, bytes[i++]);
      break;
    case OP_FUNCTION:
      printf("[%05d] Function          %d\n", j, bytes[i++]);
      break;
    case OP_UNPACK:
      printf("[%05d] Unpack            %d\n", j, bytes[i++]);
      break;
    case OP_DESTRUCT:
      printf("[%05d] Destruct          %d\n", j, bytes[i++]);
      break;
    case OP_POP:
      printf("[%05d] Pop\n", j);
      break;
    case OP_GLOBAL:
      printf("[%05d] Global            %d\n", j, bytes[i++]);
      break;
    case OP_NONLOCAL:
      printf("[%05d] NonLocal          %d\n", j, bytes[i++]);
      break;
    case OP_GET_LOCAL:
      printf("[%05d] GetLocal          %d\n", j, bytes[i++]);
      break;
    case OP_SET_LOCAL:
      printf("[%05d] SetLocal          %d\n", j, bytes[i++]);
      break;
    case OP_APPEND:
      printf("[%05d] Append\n", j);
      break;
    case OP_GET_ELEMENT:
      printf("[%05d] GetElement\n", j);
      break;
    case OP_FETCH_ELEMENT:
      printf("[%05d] FetchElement\n", j);
      break;
    case OP_SET_ELEMENT:
      printf("[%05d] SetElement\n", j);
      break;
    case OP_PUT_ELEMENT:
      printf("[%05d] PutElement\n", j);
      break;
    case OP_DELETE:
      printf("[%05d] Delete\n", j);
      break;
    case OP_INPLACE_APPEND:
      printf("[%05d] InplaceAppend\n", j);
      break;
    case OP_INPLACE_PUT_ELEMENT:
      printf("[%05d] InplacePutElement\n", j);
      break;
    case OP_INPLACE_DELETE:
      printf("[%05d] InplaceDelete\n", j);
      break;
    case OP_GET_FIELD:
      printf("[%05d] GetField\n", j);
      break;
    case OP_JUMP:
      {
        int offset = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("[%05d] Jump              %d\n", j, offset);
      }
      break;
    case OP_JUMP_IF_FALSE:
      {
        int offset = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("[%05d] JumpIfFalse       %d\n", j, offset);
      }
      break;
    case OP_JUMP_IF_TRUE:
      {
        int offset = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("[%05d] JumpIfTrue        %d\n", j, offset);
      }
      break;
    case OP_EQUAL:
      printf("[%05d] Equal\n", j);
      break;
    case OP_GREATER:
      printf("[%05d] Greater\n", j);
      break;
    case OP_LESS:
      printf("[%05d] Less\n", j);
      break;
    case OP_ADD:
      printf("[%05d] Add\n", j);
      break;
    case OP_SUBTRACT:
      printf("[%05d] Subtract\n", j);
      break;
    case OP_MULTIPLY:
      printf("[%05d] Multiply\n", j);
      break;
    case OP_DIVIDE:
      printf("[%05d] Divide\n", j);
      break;
    case OP_MODULO:
      printf("[%05d] Modulo\n", j);
      break;
    case OP_NEGATE:
      printf("[%05d] Negate\n", j);
      break;
    case OP_NOT:
      printf("[%05d] Not\n", j);
      break;
    case OP_CALL:
      printf("[%05d] Call              %d\n", j, bytes[i++]);
      break;
    case OP_RETURN:
      printf("[%05d] Return\n", j);
      break;
    }
  }
  printf("%d instruction(s)\n\n", n);
  for (int i = 0; i < proto->num_protos; ++i)
    dump_prototype(proto->protos[i]);  
}

void dump(function_t *fn)
{
  prototype_t *proto = fn->proto;
  dump_prototype(proto);
}
