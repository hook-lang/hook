//
// The Hook Programming Language
// userdata.c
//

#include <hook/userdata.h>
#include <stdlib.h>

void hk_userdata_init(HkUserdata *udata, void (*deinit)(HkUserdata *))
{
  udata->refCount = 0;
  udata->deinit = deinit;
}

void hk_userdata_free(HkUserdata *udata)
{
  if (udata->deinit)
    udata->deinit(udata);
  free(udata);
}
