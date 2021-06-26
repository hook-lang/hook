//
// Hook Programming Language
// disasm.c
//

#include "disasm.h"
#include <stdio.h>

void dump(chunk_t *chunk)
{
  printf("<chunk at %p>\n", chunk);
  uint8_t *bytes = chunk->bytes;
  int i = 0;
  int n = 0;
  while (i < chunk->length)
  {
    opcode_t op = (opcode_t) bytes[i];
    int j = i++;
    ++n;
    switch (op)
    {
    case OP_NULL:
      printf("[%05d] Null\n", j);
      break;
    case OP_INT:
      {
        int num = *((uint16_t*) &bytes[i]);
        i += 2;
        printf("[%05d] Int      %d\n", j, num);
      }
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
    case OP_PRINT:
      printf("[%05d] Print\n", j);
      break;
    case OP_RETURN:
      printf("[%05d] Return\n", j);
      break;
    }
  }
  printf("%d instruction(s)\n\n", n);
}
