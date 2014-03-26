#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include "ggrt.h"

gghcrt_s_c_type *gghcrt_m_c_type(const char *name, size_t c_size, void *f_type)
{
  gghcrt_s_c_type *ct = gghcrt_malloc(sizeof(*ct));
  memset(ct, 0, sizeof(*ct));
  ct->name = name ? gghcrt_strdup(name) : name;
  ct->c_size = c_size;
  ct->f_type = f_type;
  ct->param_type = ct;
  return ct;
}

/* intrinsic types. */
#define TYPE(N,T,AN) gghcrt_s_c_type *gghcrt_c_type_##AN;
#include "type.def"
#define TYPE(N,T,AN) gghcrt_s_c_type *gghcrt_c_type_##N;
#include "type.def"

void gghcrt_init()
{
#define TYPE(N,T,AN) gghcrt_c_type_##N = gghcrt_m_c_type(#T, sizeof(T), &ffi_type_##N); gghcrt_c_type_##N->c_alignof = __alignof__(T);
#include "type.def"

  /* Coerce args to int */
#define ITYPE(N,T,AN) if ( sizeof(T) < sizeof(int) ) gghcrt_c_type_##N->param_type = gghcrt_c_type_sint;
#define TYPE(N,T,AN)
#include "type.def"

  /* Aliased types */
#define A_TYPE(N,T,AN) gghcrt_c_type_##->alias_of = gghcrt_c_type_##AN;
}

gghcrt_s_c_func_type *gghcrt_m_c_func_type(void *rtn_type, int nelem, gghcrt_s_c_type **elem_types)
{
  gghcrt_s_c_func_type *ct = gghcrt_m_c_type(0, 0, 0);
  ct->rtn_type = rtn_type;
  ct->nelem = nelem;
  ct->elem_types = elem_types;
  ct->param_type = gghcrt_c_type_pointer;
  return ct;
}

gghcrt_s_c_func_type *gghcrt_ffi_prepare(gghcrt_s_c_func_type *ft)
{
  if ( ! ft->f_cif_inited ) {
    ft->f_rtn_type = ft->rtn_type->f_type;
    if ( ! ft->f_elem_types ) {
      int i;
      ft->f_elem_types = gghcrt_malloc(sizeof(ft->f_elem_types) * ft->nelem);
      ft->c_args_size = 0;
      for ( i = 0; i < ft->nelem; ++ i ) {
        ft->f_elem_types[i] = ft->elem_types[i]->f_type;
        ft->c_args_size += ft->elem_types[i]->c_size;
      }
    }
    if ( ffi_prep_cif(&ft->f_cif, FFI_DEFAULT_ABI, ft->nelem, ft->f_rtn_type, ft->f_elem_types) != FFI_OK )
      abort();
    ft->f_cif_inited = 1;
  }
  return ft;
}

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

void gghcrt_ffi_call(gghcrt_s_c_func_type *ft, GGHCRT_V *rtn_valp, void *cfunc, int argc, GGHCRT_V *argv)
{
  void **f_args   = alloca(sizeof(*f_args) * gghcrt_ffi_prepare(ft)->nelem);
  void *arg_space = alloca(ft->c_args_size);
  void *rtn_space = alloca(ft->rtn_type->c_size);
  GGHCRT_V rtn_val;

  memset(arg_space, 0, ft->c_args_size);
  {
    void *arg_p = arg_space;
    int i;
    for ( i = 0; i < argc; ++ i ) {
      f_args[i] = arg_p;
      arg_p += gghcrt_ffi_unbox_arg(ft->elem_types[i], &argv[i], arg_p);
    }
  }

  ffi_call(&ft->f_cif, cfunc, rtn_space, f_args);
   
  gghcrt_ffi_box(ft->rtn_type, rtn_space, rtn_valp);
}

