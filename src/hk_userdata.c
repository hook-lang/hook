//
// The Hook Programming Language
// hk_userdata.c
//

#include "hk_userdata.h"
#include <stdlib.h>

void hk_userdata_init(hk_userdata_t *udata, void (*deinit)(struct hk_userdata *))
{
  udata->ref_count = 0;
  udata->deinit = deinit;
}

void hk_userdata_free(hk_userdata_t *udata)
{
  if (udata->deinit)
    udata->deinit(udata);
  free(udata);
}
