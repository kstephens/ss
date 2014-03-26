#include <stdio.h>
#include <string.h>

typedef void *GGRT_V;
#define GGRT_V GGRT_V
#define ggrt_malloc malloc
#define ggrt_strdup strdup
#include "ggrt.h"
#define GGRT_V_type ggrt_type_pointer

size_t ggrt_ffi_unbox(ggrt_type *ct, GGRT_V *valp, void *dst)
{
  memset(dst, 0, ct->c_size);
  memcpy(dst, valp, sizeof(*valp)); // dummy
  return ct->c_size;
}

size_t ggrt_ffi_unbox_arg(ggrt_type *ct, GGRT_V *valp, void *dst)
{
  return ggrt_ffi_unbox(ct, valp, dst);
}

void ggrt_ffi_box(ggrt_type *ct, void *src, GGRT_V *dstp)
{
  memcpy(dstp, src, sizeof(*dstp)); // dummy
}

#define ggrt_BOX_DEFINED 1
#include "ggrt.c"

static GGRT_V identity(GGRT_V x) { return x; }

int main()
{
  ggrt_init();

  ggrt_type *ct_rtn  = GGRT_V_type;
  ggrt_type *ct_params[1] = { GGRT_V_type };
  ggrt_type *ft = ggrt_m_func_type(ct_rtn, 1, ct_params);

  GGRT_V rtn, args[10];

  args[0] = (GGRT_V) 0x1234;
  ggrt_ffi_call(ft, &rtn, identity, 1, args);
  printf("%p\n", rtn);

  return 0;
}

