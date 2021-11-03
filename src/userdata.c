//
// Hook Programming Language
// userdata.c
//

#include "userdata.h"
#include <stdlib.h>

void userdata_init(userdata_t *udata, void (*deinit)(struct userdata *))
{
  udata->ref_count = 0;
  udata->deinit = deinit;
}

void userdata_free(userdata_t *udata)
{
  if (udata->deinit)
    udata->deinit(udata);
  free(udata);
}
