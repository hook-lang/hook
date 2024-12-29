//
// dump.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <hook/dump.h>

static inline int get_line(HkChunk *chunk, int offset);

static inline int get_line(HkChunk *chunk, int offset)
{
  int result = 1;
  HkLine *lines = chunk->lines;
  for (int i = 0; i < chunk->linesLength; ++i)
  {
    HkLine *line = &lines[i];
    if (line->offset > offset)
      break;
    result = line->no;
  }
  return result;
}

void hk_dump(HkFunction *fn, FILE *stream)
{
  HkString *name = fn->name;
  HkString *file = fn->file;
  char *nameChars = name ? name->chars : "; <anonymous>";
  char *fileChars = file ? file->chars : "; <srdin>";
  HkChunk *chunk = &fn->chunk;
  fprintf(stream, "; %s in %s at %p\n", nameChars, fileChars, (void *) fn);
  fprintf(stream, "; %d parameter(s), %d non-local(s), %d constant(s), %d function(s)\n", fn->arity,
    fn->numNonlocals, chunk->consts->length, fn->functionsLength);
  uint8_t *code = chunk->code;
  int i = 0;
  int n = 0;
  int last_line = -1;
  while (i < chunk->codeLength)
  {
    HkOpCode op = (HkOpCode) code[i];
    int j = i++;
    ++n;
    int line = get_line(chunk, j);
    if (line != last_line)
    {
      fprintf(stream, "  %-5d %5d ", line, j);
      last_line = line;
    }
    else
      fprintf(stream, "        %5d ", j);
    switch (op)
    {
    case HK_OP_NIL:
      fprintf(stream, "Nil\n");
      break;
    case HK_OP_FALSE:
      fprintf(stream, "False\n");
      break;
    case HK_OP_TRUE:
      fprintf(stream, "True\n");
      break;
    case HK_OP_INT:
      {
        int data = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "Int                   %5d\n", data);
      }
      break;
    case HK_OP_CONSTANT:
      fprintf(stream, "Constant              %5d\n", code[i++]);
      break;
    case HK_OP_RANGE:
      fprintf(stream, "Range\n");
      break;
    case HK_OP_ARRAY:
      fprintf(stream, "Array                 %5d\n", code[i++]);
      break;
    case HK_OP_STRUCT:
      fprintf(stream, "Struct                %5d\n", code[i++]);
      break;
    case HK_OP_INSTANCE:
      fprintf(stream, "Instance              %5d\n", code[i++]);
      break;
    case HK_OP_CONSTRUCT:
      fprintf(stream, "Construct             %5d\n", code[i++]);
      break;
    case HK_OP_ITERATOR:
      fprintf(stream, "Iterator\n");
      break;
    case HK_OP_CLOSURE:
      fprintf(stream, "Closure               %5d\n", code[i++]);
      break;
    case HK_OP_UNPACK_ARRAY:
      fprintf(stream, "UnpackArray           %5d\n", code[i++]);
      break;
    case HK_OP_UNPACK_STRUCT:
      fprintf(stream, "UnpackStruct          %5d\n", code[i++]);
      break;
    case HK_OP_POP:
      fprintf(stream, "Pop\n");
      break;
    case HK_OP_GLOBAL:
      fprintf(stream, "Global                %5d\n", code[i++]);
      break;
    case HK_OP_NONLOCAL:
      fprintf(stream, "NonLocal              %5d\n", code[i++]);
      break;
    case HK_OP_GET_LOCAL:
      fprintf(stream, "GetLocal              %5d\n", code[i++]);
      break;
    case HK_OP_SET_LOCAL:
      fprintf(stream, "SetLocal              %5d\n", code[i++]);
      break;
    case HK_OP_ADD_ELEMENT:
      fprintf(stream, "AddElement\n");
      break;
    case HK_OP_GET_ELEMENT:
      fprintf(stream, "GetElement\n");
      break;
    case HK_OP_FETCH_ELEMENT:
      fprintf(stream, "FetchElement\n");
      break;
    case HK_OP_SET_ELEMENT:
      fprintf(stream, "SetElement\n");
      break;
    case HK_OP_PUT_ELEMENT:
      fprintf(stream, "PutElement\n");
      break;
    case HK_OP_DELETE_ELEMENT:
      fprintf(stream, "DeleteElement\n");
      break;
    case HK_OP_INPLACE_ADD_ELEMENT:
      fprintf(stream, "InplaceAddElement\n");
      break;
    case HK_OP_INPLACE_PUT_ELEMENT:
      fprintf(stream, "InplacePutElement\n");
      break;
    case HK_OP_INPLACE_DELETE_ELEMENT:
      fprintf(stream, "InplaceDeleteElement\n");
      break;
    case HK_OP_GET_FIELD:
      fprintf(stream, "GetField              %5d\n", code[i++]);
      break;
    case HK_OP_FETCH_FIELD:
      fprintf(stream, "FetchField            %5d\n", code[i++]);
      break;
    case HK_OP_SET_FIELD:
      fprintf(stream, "SetField\n");
      break;
    case HK_OP_PUT_FIELD:
      fprintf(stream, "PutField              %5d\n", code[i++]);
      break;
    case HK_OP_INPLACE_PUT_FIELD:
      fprintf(stream, "InplacePutField       %5d\n", code[i++]);
      break;
    case HK_OP_CURRENT:
      fprintf(stream, "Current\n");
      break;
    case HK_OP_JUMP:
      {
        int offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "Jump                  %5d\n", offset);
      }
      break;
    case HK_OP_JUMP_IF_FALSE:
      {
        int offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "JumpIfFalse           %5d\n", offset);
      }
      break;
    case HK_OP_JUMP_IF_TRUE:
      {
        int offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "JumpIfTrue            %5d\n", offset);
      }
      break;
    case HK_OP_JUMP_IF_TRUE_OR_POP:
      {
        int offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "JumpIfTrueOrPop       %5d\n", offset);
      }
      break;
    case HK_OP_JUMP_IF_FALSE_OR_POP:
      {
        int offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "JumpIfFalseOrPop      %5d\n", offset);
      }
      break;
    case HK_OP_JUMP_IF_NOT_EQUAL:
      {
        int offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "JumpIfNotEqual        %5d\n", offset);
      }
      break;
    case HK_OP_JUMP_IF_NOT_VALID:
      {
        int offset = *((uint16_t*) &code[i]);
        i += 2;
        fprintf(stream, "JumpIfNotValid        %5d\n", offset);
      }
      break;
    case HK_OP_NEXT:
      fprintf(stream, "Next\n");
      break;
    case HK_OP_EQUAL:
      fprintf(stream, "Equal\n");
      break;
    case HK_OP_GREATER:
      fprintf(stream, "Greater\n");
      break;
    case HK_OP_LESS:
      fprintf(stream, "Less\n");
      break;
    case HK_OP_NOT_EQUAL:
      fprintf(stream, "NotEqual\n");
      break;
    case HK_OP_NOT_GREATER:
      fprintf(stream, "NotGreater\n");
      break;
    case HK_OP_NOT_LESS:
      fprintf(stream, "NotLess\n");
      break;
    case HK_OP_BITWISE_OR:
      fprintf(stream, "BitwiseOr\n");
      break;
    case HK_OP_BITWISE_XOR:
      fprintf(stream, "BitwiseXor\n");
      break;
    case HK_OP_BITWISE_AND:
      fprintf(stream, "BitwiseAnd\n");
      break;
    case HK_OP_LEFT_SHIFT:
      fprintf(stream, "LeftShift\n");
      break;
    case HK_OP_RIGHT_SHIFT:
      fprintf(stream, "RightShift\n");
      break;
    case HK_OP_ADD:
      fprintf(stream, "Add\n");
      break;
    case HK_OP_SUBTRACT:
      fprintf(stream, "Subtract\n");
      break;
    case HK_OP_MULTIPLY:
      fprintf(stream, "Multiply\n");
      break;
    case HK_OP_DIVIDE:
      fprintf(stream, "Divide\n");
      break;
    case HK_OP_QUOTIENT:
      fprintf(stream, "Quotient\n");
      break;
    case HK_OP_REMAINDER:
      fprintf(stream, "Remainder\n");
      break;
    case HK_OP_NEGATE:
      fprintf(stream, "Negate\n");
      break;
    case HK_OP_NOT:
      fprintf(stream, "Not\n");
      break;
    case HK_OP_BITWISE_NOT:
      fprintf(stream, "BitwiseNot\n");
      break;
    case HK_OP_INCREMENT:
      fprintf(stream, "Increment\n");
      break;
    case HK_OP_DECREMENT:
      fprintf(stream, "Decrement\n");
      break;
    case HK_OP_CALL:
      fprintf(stream, "Call                  %5d\n", code[i++]);
      break;
    case HK_OP_LOAD_MODULE:
      fprintf(stream, "LoadModule\n");
      break;
    case HK_OP_RETURN:
      fprintf(stream, "Return\n");
      break;
    case HK_OP_RETURN_NIL:
      fprintf(stream, "ReturnNil\n");
      break;
    }
  }
  fprintf(stream, "; %d instruction(s)\n\n", n);
  for (int j = 0; j < fn->functionsLength; ++j)
    hk_dump(fn->functions[j], stream);
}
