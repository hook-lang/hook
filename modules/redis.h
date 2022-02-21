//
// Hook Programming Language
// redis.h
//

#ifndef REDIS_H
#define REDIS_H

#include "h_vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_redis(vm_t *vm);
#else
int load_redis(vm_t *vm);
#endif

#endif // REDIS_H
