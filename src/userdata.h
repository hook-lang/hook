//
// Hook Programming Language
// userdata.h
//

#ifndef USERDATA_H
#define USERDATA_H

#include "value.h"

#define USERDATA_HEADER OBJECT_HEADER \
                        void (*deinit)(struct userdata *);

typedef struct userdata
{
  USERDATA_HEADER
} userdata_t;

void userdata_init(userdata_t *udata, void (*deinit)(struct userdata *));
void userdata_free(userdata_t *udata);

#endif
