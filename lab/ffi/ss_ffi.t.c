#include <stdio.h>
#include <stdlib.h>
#include <ffi.h>
#include <string.h>
#include <alloca.h>

typedef void *ss;

#define ss_malloc(x) malloc(x)

typedef struct ss_s_c_type {
    ss name;
    size_t c_size;    /* C sizeof() */
    ffi_type *f_type;
    struct ss_s_c_type *param_type; /* as an function parameter. */

    /* func type */
    struct ss_s_c_type *rtn_type;
    int nparam;
    struct ss_s_c_type **param_types;

    /* func type: enerated */
    ffi_cif f_cif;
    short f_cif_inited;
    ffi_type *f_rtn_type;
    ffi_type **f_param_types;
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

typedef ss_s_c_type ss_s_c_func_type;

ss_s_c_func_type *ss_m_c_func_type(void *rtn_type, int nparam, ss_s_c_type **param_types)
{
    ss_s_c_func_type *ct = ss_m_c_type(0, 0, 0);
    ct->rtn_type = rtn_type;
    ct->nparam = nparam;
    ct->param_types = param_types;
    ct->param_type = (ss_s_c_type *) ct; // void*
    return ct;
}

ss_s_c_func_type *ss_ffi_prepare(ss_s_c_func_type *ft)
{
    if ( ! ft->f_cif_inited ) {
        ft->f_rtn_type = ft->rtn_type->f_type;
        if ( ! ft->f_param_types ) {
            int i;
            ft->f_param_types = ss_malloc(sizeof(ft->f_param_types) * ft->nparam);
            ft->c_args_size = 0;
            for ( i = 0; i < ft->nparam; ++ i ) {
                ft->f_param_types[i] = ft->param_types[i]->f_type;
                ft->c_args_size += ft->param_types[i]->c_size;
            }
        }
        ffi_prep_cif(&ft->f_cif, FFI_DEFAULT_ABI, ft->nparam, ft->f_rtn_type, ft->f_param_types);
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
    void **f_args   = alloca(sizeof(*f_args) * ss_ffi_prepare(ft)->nparam);
    void *arg_space = alloca(ft->c_args_size);
    void *rtn_space = alloca(ft->rtn_type->c_size);

    memset(arg_space, 0, ft->c_args_size);
    {
        void *arg_p = arg_space;
        int i;
        for ( i = 0; i < argc; ++ i ) {
            f_args[i] = arg_p;
            arg_p += ss_ffi_unbox_arg(ft->param_types[i], argv[i], arg_p);
        }
    }

    ffi_call(&ft->f_cif, cfunc, rtn_space, f_args);
   
    return ss_ffi_box(ft->rtn_type, rtn_space);
}

#define TYPE(N,T) ss_s_c_type *ss_c_type_##N;
#include "type.def"

void ss_init_c_type()
{
#define TYPE(N,T) ss_c_type_##N = ss_m_c_type(#T, sizeof(T), &ffi_type_##N);
#include "type.def"
    /* Coerce these args to int? */
#define ITYPE(N,T) if ( sizeof(T) < sizeof(int) ) ss_c_type_##N->param_type = ss_c_type_sint;
#define TYPE(N,T)
#include "type.def"
}

ss identity(ss x) { return x; }

int main()
{
    ss_init_c_type();

    ss_s_c_type *ct_ss   = ss_c_type_pointer;
    ss_s_c_type *ct_rtn  = ct_ss;
    ss_s_c_type *ct_arg0 = ct_ss;
    ss_s_c_type *ct_args[1] = { ct_arg0 };
    ss_s_c_func_type *ft = ss_m_c_func_type(ct_rtn, 1, ct_args);
    ss rtn, arg0;

    arg0 = (ss) 0x1234;
    rtn = ss_ffi_call(ft, identity, 1, &arg0);
    printf("%p\n", rtn);

    return 0;
}
