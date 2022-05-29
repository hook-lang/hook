//
// Hook Programming Language
// hook_dump.c
//

#include "hook_dump.h"

void hk_dump(hk_function_t *fn)
{
  hk_string_t *name = fn->name;
  hk_string_t *file = fn->file;
  char *name_chars = name ? name->chars : "<anonymous>";
  char *file_chars = file ? file->chars : "<srdin>";
  printf("%s in %s at %p\n", name_chars, file_chars, fn);
  printf("%d parameter(s), %d non-local(s), %d constant(s), %d function(s)\n", fn->arity,
    fn->num_nonlocals, fn->chunk.consts->length, fn->functions_length);
  uint8_t *code = fn->chunk.code;
  int32_t i = 0;
  int32_t n = 0;
  while (i < fn->chunk.code_length)
  {
    int32_t op = code[i];
    int32_t j = i++;
    ++n;
    switch (op)
    {
    case HK_OP_NIL:
      printf("  [%05d] Nil\n", j);
      break;
    case HK_OP_FALSE:
      printf("  [%05d] False\n", j);
      break;
    case HK_OP_TRUE:
      printf("  [%05d] True\n", j);
      break;
    case HK_OP_INT:
      {
        int32_t data = *((uint16_t*) &code[i]);
        i += 2;
        printf("  [%05d] Int                   %d\n", j, data);
      }
      break;
    case HK_OP_CONSTANT:
      printf("  [%05d] Constant              %d\n", j, code[i++]);
      break;
    case HK_OP_RANGE:
      printf("  [%05d] Range\n", j);
      break;
    case HK_OP_ARRAY:
      printf("  [%05d] Array                 %d\n", j, code[i++]);
      break;
    case HK_OP_STRUCT:
      printf("  [%05d] Struct                %d\n", j, code[i++]);
      break;
    case HK_OP_INSTANCE:
      printf("  [%05d] Instance              %d\n", j, code[i++]);
      break;
    case HK_OP_CONSTRUCT:
      printf("  [%05d] Construct             %d\n", j, code[i++]);
      break;
    case HK_OP_CLOSURE:
      printf("  [%05d] Closure               %d\n", j, code[i++]);
      break;
    case HK_OP_UNPACK:
      printf("  [%05d] Unpack                %d\n", j, code[i++]);
      break;
    case HK_OP_DESTRUCT:
      printf("  [%05d] Destruct              %d\n", j, code[i++]);
      break;
    case HK_OP_POP:
      printf("  [%05d] Pop\n", j);
      break;
    case HK_OP_GLOBAL:
      printf("  [%05d] Global                %d\n", j, code[i++]);
      break;
    case HK_OP_NONLOCAL:
      printf("  [%05d] NonLocal              %d\n", j, code[i++]);
      break;
    case HK_OP_LOAD:
      printf("  [%05d] Load                  %d\n", j, code[i++]);
      break;
    case HK_OP_STORE:
      printf("  [%05d] Store                 %d\n", j, code[i++]);
      break;
    case HK_OP_ADD_ELEMENT:
      printf("  [%05d] AddElement\n", j);
      break;
    case HK_OP_GET_ELEMENT:
      printf("  [%05d] GetElement\n", j);
      break;
    case HK_OP_FETCH_ELEMENT:
      printf("  [%05d] FetchElement\n", j);
      break;
    case HK_OP_SET_ELEMENT:
      printf("  [%05d] SetElement\n", j);
      break;
    case HK_OP_PUT_ELEMENT:
      printf("  [%05d] PutElement\n", j);
      break;
    case HK_OP_DELETE_ELEMENT:
      printf("  [%05d] DeleteElement\n", j);
      break;
    case HK_OP_INPLACE_ADD_ELEMENT:
      printf("  [%05d] InplaceAddElement\n", j);
      break;
    case HK_OP_INPLACE_PUT_ELEMENT:
      printf("  [%05d] InplacePutElement\n", j);
      break;
    case HK_OP_INPLACE_DELETE_ELEMENT:
      printf("  [%05d] InplaceDeleteElement\n", j);
      break;
    case HK_OP_GET_FIELD:
      printf("  [%05d] GetField              %d\n", j, code[i++]);
      break;
    case HK_OP_FETCH_FIELD:
      printf("  [%05d] FetchField            %d\n", j, code[i++]);
      break;
    case HK_OP_SET_FIELD:
      printf("  [%05d] SetField\n", j);
      break;
    case HK_OP_PUT_FIELD:
      printf("  [%05d] PutField              %d\n", j, code[i++]);
      break;
    case HK_OP_INPLACE_PUT_FIELD:
      printf("  [%05d] InplacePutField       %d\n", j, code[i++]);
      break;
    case HK_OP_JUMP:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        printf("  [%05d] Jump                  %d\n", j, offset);
      }
      break;
    case HK_OP_JUMP_IF_FALSE:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        printf("  [%05d] JumpIfFalse           %d\n", j, offset);
      }
      break;
    case HK_OP_JUMP_IF_TRUE:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        printf("  [%05d] JumpIfTrue            %d\n", j, offset);
      }
      break;
    case HK_OP_OR:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        printf("  [%05d] Or                    %d\n", j, offset);
      }
      break;
    case HK_OP_AND:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        printf("  [%05d] And                   %d\n", j, offset);
      }
      break;
    case HK_OP_MATCH:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        printf("  [%05d] Match                 %d\n", j, offset);
      }
      break;
    case HK_OP_EQUAL:
      printf("  [%05d] Equal\n", j);
      break;
    case HK_OP_GREATER:
      printf("  [%05d] Greater\n", j);
      break;
    case HK_OP_LESS:
      printf("  [%05d] Less\n", j);
      break;
    case HK_OP_NOT_EQUAL:
      printf("  [%05d] NotEqual\n", j);
      break;
    case HK_OP_NOT_GREATER:
      printf("  [%05d] NotGreater\n", j);
      break;
    case HK_OP_NOT_LESS:
      printf("  [%05d] NotLess\n", j);
      break;
    case HK_OP_ADD:
      printf("  [%05d] Add\n", j);
      break;
    case HK_OP_SUBTRACT:
      printf("  [%05d] Subtract\n", j);
      break;
    case HK_OP_MULTIPLY:
      printf("  [%05d] Multiply\n", j);
      break;
    case HK_OP_DIVIDE:
      printf("  [%05d] Divide\n", j);
      break;
    case HK_OP_QUOTIENT:
      printf("  [%05d] Quotient\n", j);
      break;
    case HK_OP_REMAINDER:
      printf("  [%05d] Remainder\n", j);
      break;
    case HK_OP_NEGATE:
      printf("  [%05d] Negate\n", j);
      break;
    case HK_OP_NOT:
      printf("  [%05d] Not\n", j);
      break;
    case HK_OP_INCR:
      printf("  [%05d] Incr\n", j);
      break;
    case HK_OP_DECR:
      printf("  [%05d] Decr\n", j);
      break;
    case HK_OP_CALL:
      printf("  [%05d] Call                  %d\n", j, code[i++]);
      break;
    case HK_OP_LOAD_MODULE:
      printf("  [%05d] LoadModule\n", j);
      break;
    case HK_OP_RETURN:
      printf("  [%05d] Return\n", j);
      break;
    case HK_OP_RETURN_NIL:
      printf("  [%05d] ReturnNil\n", j);
      break;
    }
  }
  printf("%d instruction(s)\n\n", n);
  for (int32_t i = 0; i < fn->functions_length; ++i)
    hk_dump(fn->functions[i]);  
}
