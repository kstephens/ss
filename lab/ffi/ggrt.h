#ifndef __gg_ggrt_h
#define __gg_ggrt_h

#include <ffi.h>

#ifndef GGHCRT_V
#warning "GGHCRT_V is not defined, defaulting to void*."
typedef void *GGHCRT_V;
#define GGHCRT_V GGHCRT_V
#endif

#ifndef gghcrt_malloc
#warning "gghcrt_malloc(size) is not defined, defaulting to malloc(size)."
#define gghcrt_malloc(x) malloc(x)
#define gghcrt_strdup(x) strdup(x)
#endif

typedef struct gghcrt_s_c_type {
  const char *name;
  size_t c_size;    /* C sizeof() */
  size_t c_alignof; /* C __alignof__() */
  ffi_type *f_type;
  struct gghcrt_s_c_type *param_type; /* as an function parameter. */
  struct gghcrt_s_c_type *alias_of;

  /* struct, union, enum, func type */
  struct gghcrt_s_c_type *rtn_type; /* AND pointer to type. */
  int nelem;
  char **elem_names;
  struct gghcrt_s_c_type **elem_types;
  GGHCRT_V *elem_values; /* enum values. */

  /* func type: generated */
  ffi_cif f_cif;
  short f_cif_inited;
  ffi_type *f_rtn_type;
  ffi_type **f_elem_types;
  size_t c_args_size;
} gghcrt_s_c_type;

typedef gghcrt_s_c_type gghcrt_s_c_func_type;

/* intrinsic types. */
#define TYPE(N,T,AN) extern gghcrt_s_c_type *gghcrt_c_type_##AN;
#include "type.def"
#define TYPE(N,T,AN) extern gghcrt_s_c_type *gghcrt_c_type_##N;
#include "type.def"

/* Must call before use. */
void gghcrt_init();

/* Define intrinsic type. */
gghcrt_s_c_type *gghcrt_m_c_type(const char *name, size_t c_size, void *f_type);

/* Define function type. */
gghcrt_s_c_func_type *gghcrt_m_c_func_type(void *rtn_type, int nelem, gghcrt_s_c_type **elem_types);

/* Func call. */
void gghcrt_ffi_call(gghcrt_s_c_func_type *ft, GGHCRT_V *rtn_valp, void *cfunc, int argc, GGHCRT_V *argv);

/* Users must define these functions. */
size_t gghcrt_ffi_unbox(gghcrt_s_c_type *ct, GGHCRT_V *valp, void *dst);
size_t gghcrt_ffi_unbox_arg(gghcrt_s_c_type *ct, GGHCRT_V *valp, void *dst);
void   gghcrt_ffi_box(gghcrt_s_c_type *ct, void *src, GGHCRT_V *dstp);

/* Internal */
gghcrt_s_c_func_type *gghcrt_ffi_prepare(gghcrt_s_c_func_type *ft);

#endif
