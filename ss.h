#ifndef _ss_h_
#define _ss_h_

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <errno.h>
#include <string.h> /* memcpy(), strerror(), strcasecmp() */
#include <strings.h> /* strcasecmp() alt. */
#ifdef __APPLE__
#define _DONT_USE_CTYPE_INLINE_ 1
#define _ANSI_SOURCE 1
#endif
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h> /* ssize_t? */
#include <dlfcn.h> /* dlopen() */
#include <assert.h>
#ifndef __APPLE__
#include <alloca.h>
#endif
#include <jit/jit.h>

#ifdef NO_GC
#define GC_malloc(X) malloc(X)
#define GC_malloc_atomic(X) malloc(X)
#define GC_realloc(X,Y) realloc(X,Y)
#define GC_free(X) free(X)
#define GC_register_finalizer(_1,_2,_3,_4,_5) ((void) 0)
#define GC_INIT() ((void) 0)
#else
#include "gc/gc.h"
#endif

#define ss_ALIGNED(DECL,X) DECL __attribute__ ((aligned (X)))

#define ss_PASTE2(A,B)ss_PASTE2_(A,B)
#define ss_PASTE2_(A,B)A##B
#define ss_PASTE3(A,B,C)ss_PASTE3_(A,B,C)
#define ss_PASTE3_(A,B,C)A##B##C
#define ss_PASTE4(A,B,C,D)ss_PASTE4_(A,B,C,D)
#define ss_PASTE4_(A,B,C,D)A##B##C##D
#define ss_STRINGTIZE(A)ss_STRINGTIZE_(A)
#define ss_STRINGTIZE_(A)#A

#define ss_malloc(X) GC_malloc(X)

typedef void *ss;
typedef size_t  ss_word_t;
typedef ssize_t ss_fixnum_t;

struct ss_s_env;
struct ss_s_prim;

#define ss_PRIM_DECL(X) \
  ss X (struct ss_s_env *ss_env, ss *_ss_expr, struct ss_s_prim *ss_prim, unsigned int ss_argc, ss *ss_argv)

#define ss_BOX_fixnum(X)  ((ss)  ((((ss_fixnum_t)(X)) << 1) | 1))
#define ss_UNB_fixnum(X)  (       (((ss_fixnum_t)(X)) >> 1)     )

// ((EOF = -1) + 17) << 1 => 32 => ss_eos
#define ss_BOX_char(X)    ((ss) (((((ss_fixnum_t)(X)) % 256) + 17) << 1)     )
#define ss_UNB_char(X)    (       (((ss_fixnum_t)(X))              >> 1) - 17)

#define ss_nil    ((ss)0)
#define ss_undef  ((ss)2)
#define ss_unspec ((ss)4)
#define ss_t      ((ss)6)
#define ss_f      ((ss)8)
#define ss_eos    ((ss)32)

typedef enum ss_te {
  ss_te_UNDEF = 0,
#define ss_te_def(N) ss_te_##N,
#include "te.def"
  ss_te_LITERAL_MIN = ss_te_undef,
  ss_te_LITERAL_MAX = ss_te_throwable,
  ss_te_LAST,
} ss_te;

typedef struct ss_s_type {
  void *c_func;
  ss_PRIM_DECL((*prim));
  const char *name;
  ss_word_t e; // ss_te
  size_t instance_size;
  ss supers;
  ss methods;
} ss_s_type;

#define ss_t_def(N) extern ss_s_type *ss_t_##N;
#include "t.def"
extern ss_s_type* ss_te_to_t[];
extern ss_s_type* ss_ALIGNED(ss_immediate_types[], 64);

static inline
ss ss_eqQ(ss a, ss b) { return a == b ? ss_t : ss_f; }

#define ss_box(T,X)ss_PASTE2(ss_box_,T)(X)
#define ss_BOX(T,X)ss_PASTE2(ss_BOX_,T)(X)
#define ss_unb(T,X)ss_PASTE2(ss_unb_,T)(X)
#define ss_UNB(T,X)ss_PASTE2(ss_UNB_,T)(X)

static inline
ss          ss_c(ss_fixnum_t c) { return ss_BOX_char(c); }
ss_fixnum_t ss_C(ss v)          { return ss_UNB_char(v); }

static inline
ss ss_type(ss x)
{
  return               x  ==  0 ? ss_t_null :
          ((ss_word_t) x) &   1 ? ss_t_fixnum :
          ((ss_word_t) x) <= 32 ? ss_immediate_types[(ss_word_t) x] :
          x <= ss_BOX_char(255) ? ss_t_char :
                                  (((ss*) x)[-1]);
}
#define ss_type_(X) ((ss_s_type*) ss_type(X))
static inline
ss_te ss_type_te(ss x)
{ return ss_type_(x)->e; }

static inline
int ss_literalQ(ss X)
{
  return ss_te_LITERAL_MIN <= ss_type_te(X) && ss_type_te(X) <= ss_te_LITERAL_MAX;
}

ss ss_box_fixnum(ss_fixnum_t _v);
ss_fixnum_t ss_unb_fixnum(ss v);
static inline
ss ss_i(ss_fixnum_t x) { return ss_BOX_fixnum(x); }
static inline
ss_fixnum_t ss_I(ss x) { return ss_UNB_fixnum(x); }

typedef double ss_flonum_t;
typedef struct ss_s_flonum {
  ss_flonum_t value;
} ss_s_flonum;
#define ss_UNB_flonum(X)((ss_s_flonum*)(X))->value
static inline
ss ss_box_flonum(ss_flonum_t _v);
static inline
ss_flonum_t ss_unb_flonum(ss v);

static inline
ss ss_b(int x) { return x ? ss_t : ss_f; }
static inline
int ss_B(ss x) { return x != ss_f; }
#define ss_box_boolean(X) ss_b((int) (X)) 
#define ss_unb_boolean(X) ss_B(X)

typedef struct ss_s_cons {
  ss a, d;
} ss_s_cons;
#define ss_CAR(X) (((ss_s_cons*) (X))->a)
#define ss_CDR(X) (((ss_s_cons*) (X))->d)

typedef struct ss_s_vector {
  ss *v;
  size_t l;
} ss_s_vector;
static inline
ss    *ss_vector_V(ss x)
{ return ((ss_s_vector*)(x))->v; }
static inline
size_t ss_vector_L(ss x)
{ return ((ss_s_vector*)(x))->l; }
static inline
ss     ss_vector_R(ss x, ss i)
{ return ((ss_s_vector*)(x))->v[ss_I(i)]; }
static inline
ss     ss_vector_S(ss x, ss i, ss v)
{ ((ss_s_vector*)(x))->v[ss_I(i)] = v; return x; }
ss ss_vecn(size_t l);
ss ss_vecnv(size_t l, const ss *v);
ss ss_vec(int n, ...);
static inline
ss ss_vec1(ss a0)
{
  ss_s_vector *self = ss_vecn(1);
  self->v[0] = a0;
  return self;
}

static inline
ss ss_vec2(ss a0, ss a1)
{
  ss_s_vector *self = ss_vecn(2);
  self->v[0] = a0;
  self->v[1] = a1;
  return self;
}

typedef char ss_string_t;
typedef struct ss_s_string {
  ss_string_t *v;
  size_t l;
} ss_s_string;
static inline
char  *ss_string_V(ss x)
{ return ((ss_s_string*)(x))->v; }
static inline
size_t ss_string_L(ss x)
{ return ((ss_s_string*)(x))->l; }
static inline
ss     ss_string_R(ss x, ss i)
{ return ss_c(((ss_s_string*)(x))->v[ss_I(i)]); }
static inline
ss     ss_string_S(ss x, ss i, ss v)
{ ((ss_s_string*)(x))->v[ss_I(i)] = ss_C(v); return x; }

ss ss_strn(size_t l);
ss ss_s(const char *p);

typedef struct ss_s_symbol {
  ss name;
  ss docstring;
  ss syntax;
  ss_fixnum_t is_const;
} ss_s_symbol;
#define ss_UNB_symbol(X) (*((ss_s_symbol*)(X)))
ss ss_box_symbol(const char *name);

typedef struct ss_s_port {
  FILE *fp;
  ss name;
  ss mode;
} ss_s_port;
#define ss_UNB_port(X) (*(ss_s_port*)(X))

struct ss_s_catch;
typedef struct ss_s_env {
  ss_fixnum_t argc;
  ss *symv;
  ss *argv;
  struct ss_s_env *parent, *top_level;
  ss_fixnum_t constantExprQ, constantExprQAll;
  ss_fixnum_t level, depth;
  ss expr;
  struct ss_s_catch *catch, *error_catch;
} ss_s_env;

#define ss_constantExprQ    ss_env->constantExprQ
#define ss_constantExprQAll ss_env->constantExprQAll

ss ss_error_raise(ss_s_env *ss_env, ss val);
ss ss_error(ss_s_env *ss_env, const char *code, ss obj, const char *format, ...);

#include "ss/catch.h"

typedef struct ss_s_prim {
  void *c_func;
  ss_PRIM_DECL((*prim));
  const char *name;
  ss_fixnum_t min_args, max_args, no_side_effect;
  const char *docstring;
} ss_s_prim;
#define ss_UNB_prim(X)((ss_s_prim*)(X))

#ifndef _ss_prim
#define _ss_prim(NAME,MINARGS,MAXARGS,NO_SIDE_EFFECT,DOCSTRING)         \
  extern ss ss_sym(NAME);                                               \
  ss ss_p_##NAME;                                                       \
  static ss_PRIM_DECL(ss_PASTE2(_ss_pf_,NAME));                         \
  ss_s_prim ss_PASTE2(_ss_p_,NAME) = { 0, ss_PASTE2(_ss_pf_,NAME), #NAME, MINARGS, MAXARGS, NO_SIDE_EFFECT, DOCSTRING } ; \
  static ss_PRIM_DECL(ss_PASTE2(_ss_pf_,NAME)) {                        \
  ss ss_rtn = ss_undef;                                                 \
  (void) ss_sym(NAME);                                                  \
  if ( MINARGS >= 0 && ss_argc < MINARGS )                              \
    _ss_min_args_error(ss_env, ss_prim, DOCSTRING, ss_argc, MINARGS);   \
  if ( MAXARGS >= 0 && ss_argc > MAXARGS )                              \
    _ss_max_args_error(ss_env, ss_prim, DOCSTRING, ss_argc, MAXARGS);   \
{
#endif

#define ss_return(X) do { ss_rtn = (X); goto _ss_rtn; } while(0)

#ifndef ss_prim
#define ss_end                                                       \
  }                                                                  \
_ss_rtn:                                                             \
 return(ss_rtn);                                                     \
 }
#define ss_prim(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  _ss_prim(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)
#endif

#ifndef ss_syntax
#define ss_syntax(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  _ss_prim(ss_syn_##NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)
#endif

typedef struct ss_s_lambda {
  ss formals;
  ss params;
  ss body;
  ss rest;
  ss_fixnum_t rest_i;
} ss_s_lambda;
#define ss_UNB_lambda(X) (*(ss_s_lambda*)(X))

typedef struct ss_s_closure {
  void *c_func;
  ss_PRIM_DECL((*prim));
  ss_s_lambda *lambda;
  ss_s_env *env;
} ss_s_closure;
#define ss_UNB_closure(X) (*(ss_s_closure*)(X))

typedef struct ss_s_quote {
  ss value;
} ss_s_quote;
#define ss_UNB_quote(X) ((ss_s_quote*)(X))->value

typedef struct ss_s_var {
  ss name;
  ss_fixnum_t up, over;
} ss_s_var;
#define ss_UNB_var(X) (*(ss_s_var*)(X))

typedef struct ss_s_var_set {
  ss var;
  ss expr;
} ss_s_var_set;
#define ss_UNB_var_set(X) (*(ss_s_var_set*)(X))

typedef struct ss_s_global {
  ss *ref;
  ss name;
} ss_s_global;
#define ss_UNB_global(X) (*((ss_s_global*)(X))->ref)

typedef struct ss_s_if {
  ss t, a, b;
} ss_s_if;

#ifndef ss_sym
#define ss_sym(X)ss_PASTE2(_ss_sym_,X)
#endif

#ifndef _ss_prim
#define _ss_prim(TYPE,NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) ss_sym(NAME)
#include "prim.def"
#endif

extern ss _ss_syms
#define ss_sym_def(X),ss_PASTE2(_ss_sym_,X)
#include "sym.def"
;

#endif
