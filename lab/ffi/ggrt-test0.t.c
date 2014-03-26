#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef void *GGRT_V;
#define GGRT_V GGRT_V
#define ggrt_malloc  malloc
#define ggrt_realloc realloc
#define ggrt_free    free
#define ggrt_strdup  strdup
#include "ggrt.h"
#define GGRT_V_type ggrt_type_pointer

size_t ggrt_ffi_unbox(ggrt_type *ct, GGRT_V *valp, void *dst)
{
  memset(dst, 0, ct->c_sizeof);
  memcpy(dst, valp, sizeof(*valp)); // dummy
  return ct->c_sizeof;
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

struct align_struct_dummy {
  char c[23];
  void *p;
};

#define SELEMS(E)     \
  E(c,char,char)      \
  E(s,short,short)    \
  E(i,int,int)        \
  E(l,long,long)      \
  E(f,float,float)    \
  E(d,double,double)  \
  E(ld,long double, long_double) \
  E(p,void *, voidP)

typedef struct test_struct {
#define E(N,T,TN) T N;
  SELEMS(E)
#undef E
} test_struct;

#define my_offsetof(T,E) ((size_t) &(((T*)0)->E))

static void test_struct_def()
{
  ggrt_type *st = ggrt_m_struct_type("struct", "test_struct");
  test_struct v;
  struct align_struct_dummy v2;
#define E(N,C,T) ggrt_m_struct_elem(st, #N, ggrt_type_##T);
  SELEMS(E)
#undef E
  ggrt_m_struct_type_end(st);

  assert(ggrt_type_sizeof(st) == sizeof(v));

#define E(N,T,TN) assert(ggrt_struct_elem(st, #N)->offset == my_offsetof(test_struct, N));
  SELEMS(E)
#undef E

}

static GGRT_V identity(GGRT_V x) { return x; }

static void test_func_call()
{
  ggrt_type *ct_rtn  = GGRT_V_type;
  ggrt_type *ct_params[1] = { GGRT_V_type };
  ggrt_type *ft = ggrt_m_func_type(ct_rtn, 1, ct_params);

  GGRT_V rtn, args[10];

  ggrt_symbol *sym = ggrt_global("identity", &identity, ft);

  args[0] = (GGRT_V) 0x1234;
  ggrt_ffi_call(ft, &rtn, ggrt_global_get("identity", 0)->addr, 1, args);
  printf("%p\n", rtn);
  assert(rtn == args[0]);
}

int main()
{
  ggrt_init();

  test_struct_def();
  test_func_call();

  return 0;
}

