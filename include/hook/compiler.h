//
// The Hook Programming Language
// compiler.h
//

#ifndef HK_COMPILER_H
#define HK_COMPILER_H

#include <hook/callable.h>

HkClosure *hk_compile(HkString *file, HkString *source);

#endif // HK_COMPILER_H
