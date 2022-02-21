//
// Hook Programming Language
// h_hash.c
//

#include "h_hash.h"

uint32_t hash(int length, char *chars)
{
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++)
  {
    hash ^= chars[i];
    hash *= 16777619;
  }
  return hash;
}
