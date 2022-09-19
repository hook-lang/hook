//
// The Hook Programming Language
// hk_dump.c
//

#include "hk_dump.h"

void hk_dump(hk_function_t *fn, FILE *stream)
{
  hk_string_t *name = fn->name;
  hk_string_t *file = fn->file;
  char *name_chars = name ? name->chars : "<anonymous>";
  char *file_chars = file ? file->chars : "<srdin>";
  fprintf(stream, "%s in %s at %p\n", name_chars, file_chars, (void *) fn);
  fprintf(stream, "%d parameter(s), %d non-local(s), %d constant(s), %d function(s)\n", fn->arity,
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
      fprintf(stream, "  [%05d] Nil\n", j);
      break;
    case HK_OP_FALSE:
      fprintf(stream, "  [%05d] False\n", j);
      break;
    case HK_OP_TRUE:
      fprintf(stream, "  [%05d] True\n", j);
      break;
    case HK_OP_INT:
      {
        int32_t data = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "  [%05d] Int                   %d\n", j, data);
      }
      break;
    case HK_OP_CONSTANT:
      fprintf(stream, "  [%05d] Constant              %d\n", j, code[i++]);
      break;
    case HK_OP_RANGE:
      fprintf(stream, "  [%05d] Range\n", j);
      break;
    case HK_OP_ARRAY:
      fprintf(stream, "  [%05d] Array                 %d\n", j, code[i++]);
      break;
    case HK_OP_STRUCT:
      fprintf(stream, "  [%05d] Struct                %d\n", j, code[i++]);
      break;
    case HK_OP_INSTANCE:
      fprintf(stream, "  [%05d] Instance              %d\n", j, code[i++]);
      break;
    case HK_OP_CONSTRUCT:
      fprintf(stream, "  [%05d] Construct             %d\n", j, code[i++]);
      break;
    case HK_OP_CLOSURE:
      fprintf(stream, "  [%05d] Closure               %d\n", j, code[i++]);
      break;
    case HK_OP_UNPACK:
      fprintf(stream, "  [%05d] Unpack                %d\n", j, code[i++]);
      break;
    case HK_OP_DESTRUCT:
      fprintf(stream, "  [%05d] Destruct              %d\n", j, code[i++]);
      break;
    case HK_OP_POP:
      fprintf(stream, "  [%05d] Pop\n", j);
      break;
    case HK_OP_GLOBAL:
      fprintf(stream, "  [%05d] Global                %d\n", j, code[i++]);
      break;
    case HK_OP_NONLOCAL:
      fprintf(stream, "  [%05d] NonLocal              %d\n", j, code[i++]);
      break;
    case HK_OP_LOAD:
      fprintf(stream, "  [%05d] Load                  %d\n", j, code[i++]);
      break;
    case HK_OP_STORE:
      fprintf(stream, "  [%05d] Store                 %d\n", j, code[i++]);
      break;
    case HK_OP_ADD_ELEMENT:
      fprintf(stream, "  [%05d] AddElement\n", j);
      break;
    case HK_OP_GET_ELEMENT:
      fprintf(stream, "  [%05d] GetElement\n", j);
      break;
    case HK_OP_FETCH_ELEMENT:
      fprintf(stream, "  [%05d] FetchElement\n", j);
      break;
    case HK_OP_SET_ELEMENT:
      fprintf(stream, "  [%05d] SetElement\n", j);
      break;
    case HK_OP_PUT_ELEMENT:
      fprintf(stream, "  [%05d] PutElement\n", j);
      break;
    case HK_OP_DELETE_ELEMENT:
      fprintf(stream, "  [%05d] DeleteElement\n", j);
      break;
    case HK_OP_INPLACE_ADD_ELEMENT:
      fprintf(stream, "  [%05d] InplaceAddElement\n", j);
      break;
    case HK_OP_INPLACE_PUT_ELEMENT:
      fprintf(stream, "  [%05d] InplacePutElement\n", j);
      break;
    case HK_OP_INPLACE_DELETE_ELEMENT:
      fprintf(stream, "  [%05d] InplaceDeleteElement\n", j);
      break;
    case HK_OP_GET_FIELD:
      fprintf(stream, "  [%05d] GetField              %d\n", j, code[i++]);
      break;
    case HK_OP_FETCH_FIELD:
      fprintf(stream, "  [%05d] FetchField            %d\n", j, code[i++]);
      break;
    case HK_OP_SET_FIELD:
      fprintf(stream, "  [%05d] SetField\n", j);
      break;
    case HK_OP_PUT_FIELD:
      fprintf(stream, "  [%05d] PutField              %d\n", j, code[i++]);
      break;
    case HK_OP_INPLACE_PUT_FIELD:
      fprintf(stream, "  [%05d] InplacePutField       %d\n", j, code[i++]);
      break;
    case HK_OP_JUMP:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "  [%05d] Jump                  %d\n", j, offset);
      }
      break;
    case HK_OP_JUMP_IF_FALSE:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "  [%05d] JumpIfFalse           %d\n", j, offset);
      }
      break;
    case HK_OP_JUMP_IF_TRUE:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "  [%05d] JumpIfTrue            %d\n", j, offset);
      }
      break;
    case HK_OP_JUMP_IF_TRUE_OR_POP:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "  [%05d] JumpIfTrueOrPop       %d\n", j, offset);
      }
      break;
    case HK_OP_JUMP_IF_FALSE_OR_POP:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "  [%05d] JumpIfFalseOrPop      %d\n", j, offset);
      }
      break;
    case HK_OP_JUMP_IF_NOT_EQUAL:
      {
        int32_t offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "  [%05d] JumpIfNotEqual        %d\n", j, offset);
      }
      break;
    case HK_OP_EQUAL:
      fprintf(stream, "  [%05d] Equal\n", j);
      break;
    case HK_OP_GREATER:
      fprintf(stream, "  [%05d] Greater\n", j);
      break;
    case HK_OP_LESS:
      fprintf(stream, "  [%05d] Less\n", j);
      break;
    case HK_OP_NOT_EQUAL:
      fprintf(stream, "  [%05d] NotEqual\n", j);
      break;
    case HK_OP_NOT_GREATER:
      fprintf(stream, "  [%05d] NotGreater\n", j);
      break;
    case HK_OP_NOT_LESS:
      fprintf(stream, "  [%05d] NotLess\n", j);
      break;
    case HK_OP_BITWISE_OR:
      fprintf(stream, "  [%05d] BitwiseOr\n", j);
      break;
    case HK_OP_BITWISE_XOR:
      fprintf(stream, "  [%05d] BitwiseXor\n", j);
      break;
    case HK_OP_BITWISE_AND:
      fprintf(stream, "  [%05d] BitwiseAnd\n", j);
      break;
    case HK_OP_LEFT_SHIFT:
      fprintf(stream, "  [%05d] LeftShift\n", j);
      break;
    case HK_OP_RIGHT_SHIFT:
      fprintf(stream, "  [%05d] RightShift\n", j);
      break;
    case HK_OP_ADD:
      fprintf(stream, "  [%05d] Add\n", j);
      break;
    case HK_OP_SUBTRACT:
      fprintf(stream, "  [%05d] Subtract\n", j);
      break;
    case HK_OP_MULTIPLY:
      fprintf(stream, "  [%05d] Multiply\n", j);
      break;
    case HK_OP_DIVIDE:
      fprintf(stream, "  [%05d] Divide\n", j);
      break;
    case HK_OP_QUOTIENT:
      fprintf(stream, "  [%05d] Quotient\n", j);
      break;
    case HK_OP_REMAINDER:
      fprintf(stream, "  [%05d] Remainder\n", j);
      break;
    case HK_OP_NEGATE:
      fprintf(stream, "  [%05d] Negate\n", j);
      break;
    case HK_OP_NOT:
      fprintf(stream, "  [%05d] Not\n", j);
      break;
    case HK_OP_BITWISE_NOT:
      fprintf(stream, "  [%05d] BitwiseNot\n", j);
      break;
    case HK_OP_INCR:
      fprintf(stream, "  [%05d] Incr\n", j);
      break;
    case HK_OP_DECR:
      fprintf(stream, "  [%05d] Decr\n", j);
      break;
    case HK_OP_CALL:
      fprintf(stream, "  [%05d] Call                  %d\n", j, code[i++]);
      break;
    case HK_OP_LOAD_MODULE:
      fprintf(stream, "  [%05d] LoadModule\n", j);
      break;
    case HK_OP_RETURN:
      fprintf(stream, "  [%05d] Return\n", j);
      break;
    case HK_OP_RETURN_NIL:
      fprintf(stream, "  [%05d] ReturnNil\n", j);
      break;
    }
  }
  fprintf(stream, "%d instruction(s)\n\n", n);
  for (int32_t i = 0; i < fn->functions_length; ++i)
    hk_dump(fn->functions[i], stream);
}
