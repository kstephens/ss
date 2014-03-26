#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <string.h> /* memset, memcpy */

#include "ggrt.h"

ggrt_type *ggrt_m_type(const char *name, size_t c_size, void *f_type)
{
  ggrt_type *ct = ggrt_malloc(sizeof(*ct));
  memset(ct, 0, sizeof(*ct));
  ct->name = name ? ggrt_strdup(name) : name;
  ct->c_sizeof = c_size;
  ct->c_alignof = c_size; /* guess. */
  ct->c_vararg_size = c_size; /* can be overridden. */
  ct->f_type = f_type;
  ct->param_type = ct;
  return ct;
}

/* intrinsic types. */
#define TYPE(N,T,AN) ggrt_type *ggrt_type_##AN;
#include "type.def"
#define TYPE(N,T,AN) ggrt_type *ggrt_type_##N;
#include "type.def"

void ggrt_init()
{
#define TYPE(N,T,AN) ggrt_type_##N = ggrt_m_type(#T, sizeof(T), &ffi_type_##N); ggrt_type_##N->c_alignof = __alignof__(T);
#include "type.def"

  /* Coerce args to int */
#define ITYPE(N,T,AN) if ( sizeof(T) < sizeof(int) ) ggrt_type_##N->param_type = ggrt_type_sint;
#define TYPE(N,T,AN)
#include "type.def"

  /* Aliased types */
#define A_TYPE(N,T,AN) ggrt_type_##->alias_of = ggrt_type_##AN;
}

enum ggrt_enum {
  x, y, z
};

ggrt_elem *ggrt_m_elem(const char *name, ggrt_type *t)
{
  ggrt_elem *e = ggrt_malloc(sizeof(*e));
  memset(e, 0, sizeof(*e));
  e->name = name ? ggrt_strdup(name) : name;
  e->type = t;
  return e;
}

ggrt_type *ggrt_m_enum_type(const char *name, int nelems, const char **names, long *values)
{
  ggrt_type *ct = ggrt_m_type(name, sizeof(enum ggrt_enum), &ffi_type_sint);
  ct->nelems = nelems;
  ct->elems = ggrt_malloc(sizeof(ct->elems[0]) * ct->nelems);
  {
    int i;
    for ( i = 0; i < ct->nelems; ++ i ) {
      ggrt_elem *e = ct->elems[i] = ggrt_m_elem(names[i], ct);
      e->parent = ct;
      e->parent_i = i;
      e->enum_val = values ? values[i] : 0;
    }
  }
  return ct;
}

static ggrt_type *current_st; /* NOT THREAD SAFE! */
ggrt_type *ggrt_m_struct_type(const char *s_or_u, const char *name)
{
  ggrt_type *st = ggrt_m_type(name, 0, 0);
  st->struct_scope = current_st;

  st->elems = ggrt_malloc(sizeof(st->elems[0]) * st->nelems);

  current_st = st;
  return st;
}

int ggrt_m_struct_elem(ggrt_type *st, const char *name, ggrt_type *t)
{
  int i;
  ggrt_elem *e;
  if ( ! st )
    st = current_st;

  i = st->nelems ++;
  st->elems = ggrt_realloc(st->elems, sizeof(st->elems[0]) * st->nelems);

  e = st->elems[i] = ggrt_m_elem(name, t);
  e->parent = st;
  e->parent_i = i;

  return st->nelems;
}

size_t ggrt_type_sizeof(ggrt_type *st)
{
  if ( st->c_sizeof )
    return st->c_sizeof;
  if ( st->nelems ) {
    size_t offset = 0;
    int i;
    size_t adjust_alignof;
    ggrt_elem *e;
    for ( i = 0; i < st->nelems; ++ i ) {
      e = st->elems[i];
      if ( (adjust_alignof = offset % ggrt_type_alignof(e->type)) )
        offset += ggrt_type_alignof(e->type) - adjust_alignof;
      e->offset = offset;
      offset += ggrt_type_sizeof(e->type);
    }
    e = st->elems[0];
    if ( (adjust_alignof = offset % ggrt_type_alignof(e->type)) )
      offset += ggrt_type_alignof(e->type) - adjust_alignof;

    st->c_sizeof = offset;
    st->c_alignof = ggrt_type_alignof(e->type);
    st->c_vararg_size = st->c_sizeof; // ???
  }
  return st->c_sizeof;
}

size_t ggrt_type_alignof(ggrt_type *st)
{
  if ( st->c_alignof )
    return st->c_alignof;
  ggrt_type_sizeof(st);
  return st->c_alignof;
}

ggrt_type *ggrt_m_struct_type_end(ggrt_type *st)
{
  if ( ! st )
    st = current_st;

  current_st = st->struct_scope;
  return st;
}

ggrt_type *ggrt_m_func_type(void *rtn_type, int nelems, ggrt_type **param_types)
{
  ggrt_type *ct = ggrt_m_type(0, 0, 0);
  ct->param_type = ggrt_type_pointer;
  ct->rtn_type = rtn_type;
  ct->nelems = nelems;
  ct->elems = ggrt_malloc(sizeof(ct->elems[0]) * ct->nelems);
  {
    int i;
    for ( i = 0; i < nelems; ++ i ) {
      ggrt_type *pt = param_types[i];
      ggrt_elem *e = ct->elems[i] = ggrt_m_elem(0, pt);
      e->parent = ct;
      e->parent_i = i;
    }
  }
  return ct;
}

ggrt_type *ggrt_ffi_prepare(ggrt_type *ft)
{
  if ( ! ft->f_cif_inited ) {
    ft->f_rtn_type = ft->rtn_type->f_type;
    if ( ! ft->f_elem_types ) {
      int i;
      size_t offset = 0;
      ft->f_elem_types = ggrt_malloc(sizeof(ft->f_elem_types) * ft->nelems);
      for ( i = 0; i < ft->nelems; ++ i ) {
        ggrt_elem *e = ft->elems[i];
        ft->f_elem_types[i] = e->type->f_type;
        e->offset = offset;
        offset += ggrt_type_sizeof(e->type);
        ft->c_args_size = offset;
      }
      ft->c_args_size = offset;
    }
    if ( ffi_prep_cif(&ft->f_cif, FFI_DEFAULT_ABI, ft->nelems, ft->f_rtn_type, ft->f_elem_types) != FFI_OK )
      abort();
    ft->f_cif_inited = 1;
  }
  return ft;
}

#ifndef ggrt_BOX_DEFINED

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

#endif

void ggrt_ffi_call(ggrt_type *ft, GGRT_V *rtn_valp, void *cfunc, int argc, GGRT_V *argv)
{
  void **f_args   = alloca(sizeof(*f_args) * ggrt_ffi_prepare(ft)->nelems);
  void *arg_space = alloca(ft->c_args_size);
  void *rtn_space = alloca(ggrt_type_sizeof(ft->rtn_type));
  GGRT_V rtn_val;

  memset(arg_space, 0, ft->c_args_size);
  {
    void *arg_p = arg_space;
    int i;
    for ( i = 0; i < argc; ++ i ) {
      f_args[i] = arg_p;
      arg_p += ggrt_ffi_unbox_arg(ft->elems[i]->type, &argv[i], arg_p);
    }
  }

  ffi_call(&ft->f_cif, cfunc, rtn_space, f_args);
   
  ggrt_ffi_box(ft->rtn_type, rtn_space, rtn_valp);
}

