//
// The Hook Programming Language
// compiler.h
//

#ifndef HK_COMPILER_H
#define HK_COMPILER_H

#include "callable.h"

#define HK_COMPILER_FLAG_NONE    0x00
#define HK_COMPILER_FLAG_ANALYZE 0x01

HkClosure *hk_compile(HkString *file, HkString *source, int flags);

#endif // HK_COMPILER_H
