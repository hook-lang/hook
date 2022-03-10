//
// Hook Programming Language
// hook_userdata.h
//

#ifndef HOOK_USERDATA_H
#define HOOK_USERDATA_H

#include "hook_value.h"

#define USERDATA_HEADER OBJECT_HEADER \
                        void (*deinit)(struct userdata *);

typedef struct userdata
{
  USERDATA_HEADER
} userdata_t;

void userdata_init(userdata_t *udata, void (*deinit)(struct userdata *));
void userdata_free(userdata_t *udata);

#endif // HOOK_USERDATA_H
