//
// Hook Programming Language
// h_userdata.h
//

#ifndef H_USERDATA_H
#define H_USERDATA_H

#include "h_value.h"

#define USERDATA_HEADER OBJECT_HEADER \
                        void (*deinit)(struct userdata *);

typedef struct userdata
{
  USERDATA_HEADER
} userdata_t;

void userdata_init(userdata_t *udata, void (*deinit)(struct userdata *));
void userdata_free(userdata_t *udata);

#endif // H_USERDATA_H
