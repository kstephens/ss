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
} ss_s_c_type;

typedef struct ss_s_c_func_type {
    ss name;
    size_t c_size;    /* C sizeof() */
    ffi_type *f_type;

    ss_s_c_type *rtn_type;
    int nparam;
    ss_s_c_type **param_types;

    ffi_cif f_cif;
    short f_cif_inited;
    ffi_type *f_rtn_type;
    ffi_type **f_param_types;
    size_t c_args_size;
} ss_s_c_func_type;

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
            arg_p += ss_ffi_unbox(ft->param_types[i], argv[i], arg_p);
        }
    }

    ffi_call(&ft->f_cif, cfunc, rtn_space, f_args);
   
    return ss_ffi_box(ft->rtn_type, rtn_space);
}

ss identity(ss x) { return x; }

int main()
{
    ss_s_c_type ct_rtn;
    ss_s_c_type ct_arg0;
    ss_s_c_type *ct_args[1];
    ss_s_c_func_type ft;
    ss rtn, arg0;

    ct_rtn.name = "void*";
    ct_rtn.c_size = sizeof(void*);
    ct_rtn.f_type = &ffi_type_pointer;

    ct_arg0 = ct_rtn;
    ct_args[0] = &ct_arg0;

    memset(&ft, 0, sizeof(ft));
    ft.rtn_type = &ct_rtn;
    ft.nparam = 1;
    ft.param_types = ct_args;

    arg0 = (ss) 0x1234;
    rtn = ss_ffi_call(&ft, identity, 1, &arg0);
    printf("%p\n", rtn);

    return 0;
}
