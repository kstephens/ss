#if 0
#ifndef ss_cwrap_c

#include "ggrt/ggrt.h"

size_t ss_ffi_unbox(ggrt_ctx ctx, ggrt_type_t *ct, const void *boxp, void *dst)
{
  ss val = *(ss*) boxp;

  switch ( ct->te ) {
  case ggrt_te_void:
    break;
#define ITYPE(T,N) case ggrt_te_##N: *((T*) dst) = ss_fixnum_(val); break;
#define FTYPE(T,N) case ggrt_te_##N: *((T*) dst) = ss_flonum_(val); break;
#include "cintrinsics.def"
  default:
    abort();
  }
  
  return ct->c_sizeof;
}

size_t ss_ffi_unbox_arg(ggrt_ctx ctx, ggrt_type_t *ct, const void *boxp, void *dst)
{
  return ggrt_ffi_unbox(ctx, ct, boxp, dst);
}

void ss_ffi_box(ggrt_ctx ctx, ggrt_type_t *ct, const void *src, void *boxp)
{
#define return(X) *(ss*) boxp = (X); return
  switch ( ct->te ) {
  case ggrt_te_void: return(ss_unspec);
#define CTYPE(T,N) case ggrt_te_##N: return(ss_c(*(T*)src));
#define ITYPE(T,N) case ggrt_te_##N: return(ss_i(*(T*)src));
#define FTYPE(T,N) case ggrt_te_##N: return(ss_box_fixnum(*(T*)src));
#include "cintrinsics.def"
  default:
    abort();
  }
}

#endif
#endif
