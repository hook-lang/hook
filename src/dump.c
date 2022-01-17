//
// Hook Programming Language
// dump.c
//

#include "dump.h"

void dump(function_t *fn)
{
  string_t *name = fn->name;
  string_t *file = fn->file;
  char *name_chars = name ? name->chars : "<anonymous>";
  char *file_chars = file ? file->chars : "<srdin>";
  printf("%s in %s at %p\n", name_chars, file_chars, fn);
  printf("%d parameter(s), %d non-local(s), %d constant(s), %d function(s)\n", fn->arity,
    fn->num_nonlocals, fn->consts->length, fn->num_functions);
  uint8_t *bytes = fn->chunk.bytes;
  int i = 0;
  int n = 0;
  while (i < fn->chunk.length)
  {
    opcode_t op = (opcode_t) bytes[i];
    int j = i++;
    ++n;
    switch (op)
    {
    case OP_NIL:
      printf("  [%05d] Nil\n", j);
      break;
    case OP_FALSE:
      printf("  [%05d] False\n", j);
      break;
    case OP_TRUE:
      printf("  [%05d] True\n", j);
      break;
    case OP_INT:
      {
        int data = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("  [%05d] Int                   %d\n", j, data);
      }
      break;
    case OP_CONSTANT:
      printf("  [%05d] Constant              %d\n", j, bytes[i++]);
      break;
    case OP_ARRAY:
      printf("  [%05d] Array                 %d\n", j, bytes[i++]);
      break;
    case OP_STRUCT:
      printf("  [%05d] Struct                %d\n", j, bytes[i++]);
      break;
    case OP_INSTANCE:
      printf("  [%05d] Instance              %d\n", j, bytes[i++]);
      break;
    case OP_CONSTRUCT:
      printf("  [%05d] Construct             %d\n", j, bytes[i++]);
      break;
    case OP_CLOSURE:
      printf("  [%05d] Closure               %d\n", j, bytes[i++]);
      break;
    case OP_UNPACK:
      printf("  [%05d] Unpack                %d\n", j, bytes[i++]);
      break;
    case OP_DESTRUCT:
      printf("  [%05d] Destruct              %d\n", j, bytes[i++]);
      break;
    case OP_POP:
      printf("  [%05d] Pop\n", j);
      break;
    case OP_GLOBAL:
      printf("  [%05d] Global                %d\n", j, bytes[i++]);
      break;
    case OP_NONLOCAL:
      printf("  [%05d] NonLocal              %d\n", j, bytes[i++]);
      break;
    case OP_GET_LOCAL:
      printf("  [%05d] GetLocal              %d\n", j, bytes[i++]);
      break;
    case OP_SET_LOCAL:
      printf("  [%05d] SetLocal              %d\n", j, bytes[i++]);
      break;
    case OP_ADD_ELEMENT:
      printf("  [%05d] AddElement\n", j);
      break;
    case OP_GET_ELEMENT:
      printf("  [%05d] GetElement\n", j);
      break;
    case OP_FETCH_ELEMENT:
      printf("  [%05d] FetchElement\n", j);
      break;
    case OP_SET_ELEMENT:
      printf("  [%05d] SetElement\n", j);
      break;
    case OP_PUT_ELEMENT:
      printf("  [%05d] PutElement\n", j);
      break;
    case OP_DELETE_ELEMENT:
      printf("  [%05d] DeleteElement\n", j);
      break;
    case OP_INPLACE_ADD_ELEMENT:
      printf("  [%05d] InplaceAddElement\n", j);
      break;
    case OP_INPLACE_PUT_ELEMENT:
      printf("  [%05d] InplacePutElement\n", j);
      break;
    case OP_INPLACE_DELETE_ELEMENT:
      printf("  [%05d] InplaceDeleteElement\n", j);
      break;
    case OP_GET_FIELD:
      printf("  [%05d] GetField              %d\n", j, bytes[i++]);
      break;
    case OP_FETCH_FIELD:
      printf("  [%05d] FetchField            %d\n", j, bytes[i++]);
      break;
    case OP_SET_FIELD:
      printf("  [%05d] SetField\n", j);
      break;
    case OP_PUT_FIELD:
      printf("  [%05d] PutField              %d\n", j, bytes[i++]);
      break;
    case OP_INPLACE_PUT_FIELD:
      printf("  [%05d] InplacePutField       %d\n", j, bytes[i++]);
      break;
    case OP_JUMP:
      {
        int offset = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("  [%05d] Jump                  %d\n", j, offset);
      }
      break;
    case OP_JUMP_IF_FALSE:
      {
        int offset = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("  [%05d] JumpIfFalse           %d\n", j, offset);
      }
      break;
    case OP_OR:
      {
        int offset = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("  [%05d] Or                    %d\n", j, offset);
      }
      break;
    case OP_AND:
      {
        int offset = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("  [%05d] And                   %d\n", j, offset);
      }
      break;
    case OP_MATCH:
      {
        int offset = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("  [%05d] Match                 %d\n", j, offset);
      }
      break;
    case OP_EQUAL:
      printf("  [%05d] Equal\n", j);
      break;
    case OP_GREATER:
      printf("  [%05d] Greater\n", j);
      break;
    case OP_LESS:
      printf("  [%05d] Less\n", j);
      break;
    case OP_NOT_EQUAL:
      printf("  [%05d] NotEqual\n", j);
      break;
    case OP_NOT_GREATER:
      printf("  [%05d] NotGreater\n", j);
      break;
    case OP_NOT_LESS:
      printf("  [%05d] NotLess\n", j);
      break;
    case OP_ADD:
      printf("  [%05d] Add\n", j);
      break;
    case OP_SUBTRACT:
      printf("  [%05d] Subtract\n", j);
      break;
    case OP_MULTIPLY:
      printf("  [%05d] Multiply\n", j);
      break;
    case OP_DIVIDE:
      printf("  [%05d] Divide\n", j);
      break;
    case OP_MODULO:
      printf("  [%05d] Modulo\n", j);
      break;
    case OP_NEGATE:
      printf("  [%05d] Negate\n", j);
      break;
    case OP_NOT:
      printf("  [%05d] Not\n", j);
      break;
    case OP_INCR:
      printf("  [%05d] Incr\n", j);
      break;
    case OP_DECR:
      printf("  [%05d] Decr\n", j);
      break;
    case OP_CALL:
      printf("  [%05d] Call                  %d\n", j, bytes[i++]);
      break;
    case OP_IMPORT_LIBRARY:
      printf("  [%05d] ImportLibrary\n", j);
      break;
    case OP_RETURN:
      printf("  [%05d] Return\n", j);
      break;
    case OP_RETURN_NIL:
      printf("  [%05d] ReturnNil\n", j);
      break;
    }
  }
  printf("%d instruction(s)\n\n", n);
  for (int i = 0; i < fn->num_functions; ++i)
    dump(fn->functions[i]);  
}
