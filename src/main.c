//
// Hook Programming Language
// main.c
//

#include <stdlib.h>
#include "string.h"
#include "compiler.h"
#include "disasm.h"
#include "vm.h"

int main(void)
{
  string_t *str = string_from_stream(stdin);
  scanner_t scan;
  scanner_init(&scan, str->chars);
  chunk_t chunk;
  chunk_init(&chunk, 0);
  compile(&chunk, &scan);
  string_free(str);
  //dump(&chunk);
  stack_t stk;
  stack_init(&stk, 0);
  execute(&stk, &chunk);
  chunk_free(&chunk);
  stack_free(&stk);
  return EXIT_SUCCESS;
}
