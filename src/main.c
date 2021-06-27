//
// Hook Programming Language
// main.c
//

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "compiler.h"
#include "disasm.h"
#include "vm.h"

static inline bool has_option(int argc, const char **argv, const char *option);

static inline bool has_option(int argc, const char **argv, const char *option)
{
  for (int i = 1; i < argc; ++i)
    if (!strcmp(argv[i], option))
      return true;
  return false;
}

int main(int argc, const char **argv)
{
  string_t *str = string_from_stream(stdin);
  scanner_t scan;
  scanner_init(&scan, str->chars);

  chunk_t chunk;
  chunk_init(&chunk, 0);
  array_t *consts = array_allocate(0);
  consts->length = 0;
  compile(&chunk, consts, &scan);
  string_free(str);

  if (has_option(argc, argv, "--disasm"))
  {
    dump(&chunk);
    chunk_free(&chunk);
    array_free(consts);
    return EXIT_SUCCESS;
  }

  vm_t vm;
  vm_init(&vm, 0);
  vm_execute(&vm, chunk.bytes, consts->elements);
  chunk_free(&chunk);
  array_free(consts);
  vm_free(&vm);

  return EXIT_SUCCESS;
}
