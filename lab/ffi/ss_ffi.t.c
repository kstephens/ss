#include <stdio.h>
#include <stdlib.h>
#include <ffi.h>
#include <string.h>
#include <alloca.h>

typedef void *ss;

#define ss_malloc(x) malloc(x)

typedef struct ss_s_c_type {
    const char *name;
    size_t c_size;    /* C sizeof() */
  size_t c_alignof;   /* C __alignof__() */
    ffi_type *f_type;
    struct ss_s_c_type *param_type; /* as an function parameter. */
  struct ss_s_c_type *alias_of;

    /* struct, union, func type */
    struct ss_s_c_type *rtn_type;
    int nelem;
    char **elem_names;
    struct ss_s_c_type **elem_types;

    /* func type: generated */
    ffi_cif f_cif;
    short f_cif_inited;
    ffi_type *f_rtn_type;
    ffi_type **f_elem_types;
    size_t c_args_size;
} ss_s_c_type;

ss_s_c_type *ss_m_c_type(const char *name, size_t c_size, void *f_type)
{
    ss_s_c_type *ct = ss_malloc(sizeof(*ct));
    memset(ct, 0, sizeof(*ct));
    ct->name = (ss) name;
    ct->c_size = c_size;
    ct->f_type = f_type;
    ct->param_type = ct;
    return ct;
}

#define TYPE(N,T,AN) ss_s_c_type *ss_c_type_##AN;
#include "type.def"

#define TYPE(N,T,AN) ss_s_c_type *ss_c_type_##N;
#include "type.def"

void ss_init_c_type()
{
#define TYPE(N,T,AN) ss_c_type_##N = ss_m_c_type(#T, sizeof(T), &ffi_type_##N); ss_c_type_##N->c_alignof = __alignof__(T);
#include "type.def"

    /* Coerce args to int */
#define ITYPE(N,T,AN) if ( sizeof(T) < sizeof(int) ) ss_c_type_##N->param_type = ss_c_type_sint;
#define TYPE(N,T,AN)
#include "type.def"

  /* Aliased types */
#define A_TYPE(N,T,AN) ss_c_type_##->alias_of = ss_c_type_##AN;
}

typedef ss_s_c_type ss_s_c_func_type;

ss_s_c_func_type *ss_m_c_func_type(void *rtn_type, int nelem, ss_s_c_type **elem_types)
{
    ss_s_c_func_type *ct = ss_m_c_type(0, 0, 0);
    ct->rtn_type = rtn_type;
    ct->nelem = nelem;
    ct->elem_types = elem_types;
    ct->param_type = ss_c_type_pointer;
    return ct;
}

ss_s_c_func_type *ss_ffi_prepare(ss_s_c_func_type *ft)
{
    if ( ! ft->f_cif_inited ) {
        ft->f_rtn_type = ft->rtn_type->f_type;
        if ( ! ft->f_elem_types ) {
            int i;
            ft->f_elem_types = ss_malloc(sizeof(ft->f_elem_types) * ft->nelem);
            ft->c_args_size = 0;
            for ( i = 0; i < ft->nelem; ++ i ) {
                ft->f_elem_types[i] = ft->elem_types[i]->f_type;
                ft->c_args_size += ft->elem_types[i]->c_size;
            }
        }
        ffi_prep_cif(&ft->f_cif, FFI_DEFAULT_ABI, ft->nelem, ft->f_rtn_type, ft->f_elem_types);
        ft->f_cif_inited = 1;
    }
    return ft;
}

size_t ss_ffi_unbox(ss_s_c_type *ct, ss val, void *dst)
{
    memset(dst, 0, ct->c_size);
    memcpy(dst, &val, sizeof(val)); // dummy
    return ct->c_size;
}

size_t ss_ffi_unbox_arg(ss_s_c_type *ct, ss val, void *dst)
{
    return ss_ffi_unbox(ct, val, dst);
}

ss ss_ffi_box(ss_s_c_type *ct, void *src)
{
    ss result = 0;
    memcpy(&result, src, sizeof(result)); // dummy
    return result;
}

ss ss_ffi_call(ss_s_c_func_type *ft, void *cfunc, int argc, ss *argv)
{
    void **f_args   = alloca(sizeof(*f_args) * ss_ffi_prepare(ft)->nelem);
    void *arg_space = alloca(ft->c_args_size);
    void *rtn_space = alloca(ft->rtn_type->c_size);

    memset(arg_space, 0, ft->c_args_size);
    {
        void *arg_p = arg_space;
        int i;
        for ( i = 0; i < argc; ++ i ) {
            f_args[i] = arg_p;
            arg_p += ss_ffi_unbox_arg(ft->elem_types[i], argv[i], arg_p);
        }
    }

    ffi_call(&ft->f_cif, cfunc, rtn_space, f_args);
   
    return ss_ffi_box(ft->rtn_type, rtn_space);
}

ss identity(ss x) { return x; }

int main()
{
    ss_init_c_type();

    ss_s_c_type *ct_ss   = ss_c_type_pointer;
    ss_s_c_type *ct_rtn  = ct_ss;
    ss_s_c_type *ct_params[1] = { ct_ss };
    ss_s_c_func_type *ft = ss_m_c_func_type(ct_rtn, 1, ct_params);

    ss rtn, args[10];

    args[0] = (ss) 0x1234;
    rtn = ss_ffi_call(ft, identity, 1, args);
    printf("%p\n", rtn);

    return 0;
}
