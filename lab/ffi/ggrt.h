#ifndef __gg_ggrt_h
#define __gg_ggrt_h

#include <ffi.h>

#ifndef GGRT_V
#warning "GGRT_V is not defined, defaulting to void*."
typedef void *GGRT_V;
#define GGRT_V GGRT_V
#endif

#ifndef ggrt_malloc
#warning "ggrt_malloc(size) is not defined, defaulting to malloc(size)."
#define ggrt_malloc(x) malloc(x)
#define ggrt_strdup(x) strdup(x)
#endif

typedef struct ggrt_type {
  const char *name;
  size_t c_size;    /* C sizeof() */
  size_t c_alignof; /* C __alignof__() */
  ffi_type *f_type;
  struct ggrt_type *param_type; /* as an function parameter. */
  struct ggrt_type *alias_of;

  /* struct, union, enum, func type */
  struct ggrt_type *rtn_type; /* AND pointer to type. */
  int nelem;
  char **elem_names;
  struct ggrt_type **elem_types;
  GGRT_V *elem_values; /* enum values. */

  /* func type: generated */
  ffi_cif f_cif;
  short f_cif_inited;
  ffi_type *f_rtn_type;
  ffi_type **f_elem_types;
  size_t c_args_size;
} ggrt_type;

/* intrinsic types. */
#define TYPE(N,T,AN) extern ggrt_type *ggrt_c_type_##AN;
#include "type.def"
#define TYPE(N,T,AN) extern ggrt_type *ggrt_c_type_##N;
#include "type.def"

/* Must call before use. */
void ggrt_init();

/* Define intrinsic type. */
ggrt_type *ggrt_m_type(const char *name, size_t c_size, void *f_type);

/* Define function type. */
ggrt_type *ggrt_m_func_type(void *rtn_type, int nelem, ggrt_type **elem_types);

/* Func call. */
void ggrt_ffi_call(ggrt_type *ft, GGRT_V *rtn_valp, void *cfunc, int argc, GGRT_V *argv);

/* Users must define these functions. */
size_t ggrt_ffi_unbox(ggrt_type *ct, GGRT_V *valp, void *dst);
size_t ggrt_ffi_unbox_arg(ggrt_type *ct, GGRT_V *valp, void *dst);
void   ggrt_ffi_box(ggrt_type *ct, void *src, GGRT_V *dstp);

/* Internal */
ggrt_type *ggrt_ffi_prepare(ggrt_type *ft);

#endif
