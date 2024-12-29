//
// builtin.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef BUILTIN_H
#define BUILTIN_H

#include <hook/vm.h>

void load_globals(HkVM *vm);
int num_globals(void);
int lookup_global(int length, char *chars);

#endif // BUILTIN_H
