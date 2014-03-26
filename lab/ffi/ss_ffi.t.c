#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

typedef void *GGHCRT_V;
#define GGHCRT_V GGHCRT_V
#include "ggrt.c"

size_t gghcrt_ffi_unbox(gghcrt_s_c_type *ct, GGHCRT_V *valp, void *dst)
{
  memset(dst, 0, ct->c_size);
  memcpy(dst, valp, sizeof(*valp)); // dummy
  return ct->c_size;
}

size_t gghcrt_ffi_unbox_arg(gghcrt_s_c_type *ct, GGHCRT_V *valp, void *dst)
{
  return gghcrt_ffi_unbox(ct, valp, dst);
}

void gghcrt_ffi_box(gghcrt_s_c_type *ct, void *src, GGHCRT_V *dstp)
{
  memcpy(dstp, src, sizeof(*dstp)); // dummy
}

static GGHCRT_V identity(GGHCRT_V x) { return x; }

int main()
{
  gghcrt_init();

  gghcrt_s_c_type *ct_GGHCRT_V   = gghcrt_c_type_pointer;
  gghcrt_s_c_type *ct_rtn  = ct_GGHCRT_V;
  gghcrt_s_c_type *ct_params[1] = { ct_GGHCRT_V };
  gghcrt_s_c_func_type *ft = gghcrt_m_c_func_type(ct_rtn, 1, ct_params);

  GGHCRT_V rtn, args[10];

  args[0] = (GGHCRT_V) 0x1234;
  gghcrt_ffi_call(ft, &rtn, identity, 1, args);
  printf("%p\n", rtn);

  return 0;
}

