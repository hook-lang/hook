//
// Hook Programming Language
// redis.h
//

#ifndef REDIS_H
#define REDIS_H

#include "hook.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_redis(hk_vm_t *vm);
#else
int32_t load_redis(hk_vm_t *vm);
#endif

#endif // REDIS_H
