//
// module.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef MODULE_H
#define MODULE_H

#include <hook/vm.h>

void module_cache_init(void);
void module_cache_deinit(void);
void module_load(HkVM *vm, HkString *currFile);

#endif // MODULE_H
