#ifndef _ss_h_
#define _ss_h_

#if 0
#define __USE_SVID 1
#endif
#ifdef __linux__
#define _FINITE_MATH_ONLY__ 9
#endif
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h> /* ssize_t? */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <errno.h>
#include <string.h> /* memcpy(), strerror(), strcasecmp() */
#include <strings.h> /* strcasecmp() alt. */
#include <sys/time.h>
#include <dlfcn.h> /* dlopen() */
#include <assert.h>
#ifndef __APPLE__
#include <alloca.h>
#endif
#ifdef __APPLE__
#define _DONT_USE_CTYPE_INLINE_ 1
#define _ANSI_SOURCE 1
#endif
#ifdef __linux__
#undef __USE_XOPEN2K8
#endif
#include <ctype.h>
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

#define ss_INLINE static __inline__ __attribute__((always_inline))

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
  ss_te_LAST
} ss_te;

typedef struct ss_s_type {
  void *c_func;
  ss_PRIM_DECL((*prim));
  const char *name;
  ss_word_t e; // ss_te
  ss instance_size;
  ss supers, methods;
  ss c_sizeof, c_ptr_type, c_elem_type;
} ss_s_type;

#define ss_t_def(N) extern ss_s_type *ss_t_##N;
#include "t.def"
extern ss_s_type* ss_te_to_t[];
extern ss_s_type* ss_ALIGNED(ss_immediate_types[], 64);

ss_INLINE
ss ss_eqQ(ss a, ss b) { return a == b ? ss_t : ss_f; }

#define ss_box(T,X)ss_PASTE2(ss_box_,T)(X)
#define ss_BOX(T,X)ss_PASTE2(ss_BOX_,T)(X)
#define ss_unb(T,X)ss_PASTE2(ss_unb_,T)(X)
#define ss_UNB(T,X)ss_PASTE2(ss_UNB_,T)(X)

ss_INLINE
ss          ss_c(ss_fixnum_t c) { return ss_BOX_char(c); }
ss_INLINE
ss_fixnum_t ss_C(ss v)          { return ss_UNB_char(v); }
ss_INLINE
int ss_fixnumQ(ss x) { return (int) (((ss_word_t) x) & 1); }
ss_INLINE
ss ss_type(ss x)
{
  return               x  ==  0 ? ss_t_null :
          ((ss_word_t) x) &   1 ? ss_t_fixnum :
          ((ss_word_t) x) <= 32 ? ss_immediate_types[(ss_word_t) x] :
          x <= ss_BOX_char(255) ? ss_t_char :
                                  (((ss*) x)[-1]);
}
#define ss_type_(X) ((ss_s_type*) ss_type(X))
ss_INLINE
ss_te ss_type_te(ss x)
{ return ss_type_(x)->e; }

ss_INLINE
int ss_literalQ(ss X)
{
  return ss_type_te(X) >= ss_te_LITERAL_MIN;
}

ss ss_box_fixnum(ss_fixnum_t _v);
ss_fixnum_t ss_unb_fixnum(ss v);
ss_INLINE
ss ss_i(ss_fixnum_t x) { return ss_BOX_fixnum(x); }
ss_INLINE
ss_fixnum_t ss_I(ss x) { return ss_UNB_fixnum(x); }

typedef double ss_flonum_t;
typedef struct ss_s_flonum {
  ss_flonum_t value;
} ss_s_flonum;
#define ss_UNB_flonum(X)((ss_s_flonum*)(X))->value
ss_INLINE
ss ss_box_flonum(ss_flonum_t _v);
ss_INLINE
ss_flonum_t ss_unb_flonum(ss v);

ss_INLINE
ss ss_b(int x) { return ss_b(x); }
#define ss_b(x) ((x) ? ss_t : ss_f) 
ss_INLINE
int ss_B(ss x) { return ss_B(x); }
#define ss_B(x) ((x) != ss_f)
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
ss_INLINE
ss     *ss_vector_V(ss x)
{ return ((ss_s_vector*)(x))->v; }
ss_INLINE
size_t  ss_vector_L(ss x)
{ return ((ss_s_vector*)(x))->l; }
ss_INLINE
ss      ss_vector_R(ss x, ss i)
{ return ((ss_s_vector*)(x))->v[ss_I(i)]; }
ss_INLINE
ss      ss_vector_S(ss x, ss i, ss v)
{ return ((ss_s_vector*)(x))->v[ss_I(i)] = v; }
ss ss_vecn(size_t l);
ss ss_vecnv(size_t l, const ss *v);
ss ss_vec(int n, ...);
ss_INLINE
ss ss_vec1(ss a0)
{
  ss_s_vector *self = ss_vecn(1);
  self->v[0] = a0;
  return self;
}

ss_INLINE
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
ss_INLINE
char   *ss_string_V(ss x)
{ return ((ss_s_string*)(x))->v; }
ss_INLINE
size_t  ss_string_L(ss x)
{ return ((ss_s_string*)(x))->l; }
ss_INLINE
ss      ss_string_R(ss x, ss i)
{ return ss_c(((ss_s_string*)(x))->v[ss_I(i)]); }
ss_INLINE
ss      ss_string_S(ss x, ss i, ss v)
{ ((ss_s_string*)(x))->v[ss_I(i)] = ss_C(v); return v; }

ss ss_strn(size_t l);
ss ss_s(const char *p);

typedef struct ss_s_symbol {
  ss name;
  ss docstring;
  ss syntax;
  ss_fixnum_t is_const;
} ss_s_symbol;
ss ss_box_symbol(const char *name);
ss_INLINE
const char *ss_sym_charP(ss x) {
  ss_s_symbol *sym = x;
  return sym->name == ss_f ? 0 : ss_string_V(sym->name);
}

typedef struct ss_s_port {
  ss opaque0, opaque1, opaque2, opaque3;
  ss name, mode, next_char;
  ss read_char, peek_char, char_readyQ;
  ss read_chars;
  ss write_char, write_chars;
  ss close;
} ss_s_port;

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

extern ss_s_env *ss_top_level_env, *ss_current_env;
ss ss_applyv(ss_s_env *ss_env, ss func, ss args);
#define ss_apply(FUNC, NARGS, ARGS ...)         \
  ss_applyv(ss_env, FUNC, ss_vec(NARGS, ARGS))
#define _ss_apply_sym(ENV, SYM, NARGS, ARGS ...)                        \
  ({ ss_s_env *ss_env = ss_top_level_env; ss __sym = ss_sym(SYM);       \
    ss_applyv((ENV), ss_eval(__sym), ss_vec(NARGS, ARGS));              \
  })

#define ss_apply_sym(SYM, NARGS, ARGS ...)              \
  _ss_apply_sym(ss_top_level_env, SYM, NARGS, ARGS)

#include "ss/catch.h"

typedef struct ss_s_prim {
  void *c_func;
  ss_PRIM_DECL((*prim));
  const char *name;
  ss_fixnum_t min_args, max_args, no_side_effect;
  const char *docstring;
} ss_s_prim;

#define _ss_prim_arity_check(MINARGS,MAXARGS,DOCSTRING)                 \
  if ( MINARGS >= 0 && ss_argc < MINARGS )                              \
    _ss_min_args_error(ss_env, ss_prim, DOCSTRING, ss_argc, MINARGS);   \
  if ( MAXARGS >= 0 && ss_argc > MAXARGS )                              \
    _ss_max_args_error(ss_env, ss_prim, DOCSTRING, ss_argc, MAXARGS)

#ifdef _ss_prim
#define __ss_prim __ss_prim
#else
#define __ss_prim(NAME,MINARGS,MAXARGS,NO_SIDE_EFFECT,DOCSTRING)        \
  extern ss ss_sym(NAME);                                               \
  ss ss_a1_##NAME(ss _1) {                                              \
    return ss_apply_sym(NAME, 1, _1);                                   \
  }                                                                     \
  ss ss_ea1_##NAME(ss_s_env *ss_env, ss _1) {                           \
    return _ss_apply_sym(ss_env, NAME, 1, _1);                          \
  }                                                                     \
  ss ss_a2_##NAME(ss _1, ss _2) {                                       \
    return ss_apply_sym(NAME, 2, _1, _2);                               \
  }                                                                     \
  ss ss_ea2_##NAME(ss_s_env *ss_env, ss _1, ss _2) {                    \
    return _ss_apply_sym(ss_env, NAME, 2, _1, _2);                      \
  }                                                                     \
  ss ss_p_##NAME;                                                       \
  static ss_PRIM_DECL(ss_PASTE2(_ss_pf_,NAME));                         \
  ss_s_prim ss_PASTE2(_ss_p_,NAME) = { 0, ss_PASTE2(_ss_pf_,NAME), #NAME, MINARGS, MAXARGS, NO_SIDE_EFFECT, "(" DOCSTRING ")" } ; \
  static ss_PRIM_DECL(ss_PASTE2(_ss_pf_,NAME)) {                        \
  ss ss_rtn = ss_undef;                                                 \
  _ss_prim_arity_check(MINARGS,MAXARGS,DOCSTRING);                      \
{
#endif

#define ss_prim(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  __ss_prim(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)
#define ss_return(X) do { ss_rtn = (X); goto _ss_rtn; } while(0)
#define ss_end                                                       \
  }                                                                  \
_ss_rtn:                                                             \
 return(ss_rtn);                                                     \
 }

#ifdef _ss_syntax
#define ss_syntax ss_syntax
#else
#define ss_syntax(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  __ss_prim(ss_syn_##NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)
#endif

typedef struct ss_s_lambda {
  ss formals;
  ss params;
  ss body;
  ss rest;
  ss_fixnum_t rest_i;
} ss_s_lambda;

typedef struct ss_s_closure {
  void *c_func;
  ss_PRIM_DECL((*prim));
  ss_s_lambda *lambda;
  ss_s_env *env;
} ss_s_closure;

typedef struct ss_s_quote {
  ss value;
} ss_s_quote;

typedef struct ss_s_var {
  ss name;
  ss_fixnum_t up, over;
} ss_s_var;

typedef struct ss_s_var_set {
  ss var;
  ss expr;
} ss_s_var_set;

typedef struct ss_s_global {
  ss *ref;
  ss name;
} ss_s_global;

typedef struct ss_s_if {
  ss t, a, b;
} ss_s_if;

struct ss_s_repl;
typedef struct ss_s_repl {
  ss_s_env *env;
  ss input, output, prompt, trap_errors;
  ss echo_read, echo_rewrite;
} ss_s_repl;

#ifdef _ss_sym
#define ss_sym ss_sym
#else
#define ss_sym(X)ss_PASTE2(_ss_sym_,X)
#endif

#define ss_sym_def(X)extern ss ss_PASTE2(_ss_sym_,X);
#include "sym.def"

#endif
