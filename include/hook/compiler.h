//
// compiler.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_COMPILER_H
#define HK_COMPILER_H

#include "callable.h"

#define HK_COMPILER_FLAG_NONE    0x00
#define HK_COMPILER_FLAG_ANALYZE 0x01

HkClosure *hk_compile(HkString *file, HkString *source, int flags);

#endif // HK_COMPILER_H
