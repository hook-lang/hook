//
// Hook Programming Language
// hook_hash.c
//

#include "hook_hash.h"

uint32_t hash_fnv1a(int32_t length, char *chars)
{
  uint32_t hash = 2166136261u;
  for (int32_t i = 0; i < length; i++)
  {
    hash ^= chars[i];
    hash *= 16777619;
  }
  return hash;
}
