#ifndef _ss_h_
#define _ss_h_

#include "gc/gc.h"
#include <stdlib.h>
#include <stdio.h>

#define _ss_PASTE2(A,B)A##B
#define ss_PASTE2(A,B)_ss_PASTE2(A,B)
#define _ss_STRINGTIZE(A)#A
#define ss_STRINGTIZE(A)_ss_STRINGTIZE(A)

#define ss_malloc(X) GC_malloc(X)

typedef enum ss_e_type {
  ss_t_UNDEF = 0,
  ss_t_LITERAL_MIN,
  ss_t_undef = ss_t_LITERAL_MIN,
  ss_t_unspec,
  ss_t_integer,
  ss_t_real,
  ss_t_string,
  ss_t_char,
  ss_t_boolean,
  ss_t_syntax,
  ss_t_prim,
  ss_t_closure,
  ss_t_quote,
  ss_t_eos,
  ss_t_LITERAL_MAX = ss_t_eos,

  ss_t_pair,
  ss_t_null,
  ss_t_vector,
  ss_t_symbol,
  ss_t_var_ref,
  ss_t_if,

  ss_t_port,
  
  ss_t_environment,

  ss_t_LAST
} ss_e_type;

typedef void *ss;
typedef ssize_t ss_integer_t;

#define ss_EQ(X,Y)((X)==(Y))
#define ss_NE(X,Y)!ss_EQ(X,Y)

#define ss_box(T,X)ss_PASTE2(ss_box_,T)(X)
#define ss_BOX(T,X)ss_PASTE2(ss_BOX_,T)(X)
#define ss_unbox(T,X)ss_PASTE2(ss_unbox_,T)(X)
#define ss_UNBOX(T,X)ss_PASTE2(ss_UNBOX_,T)(X)

#define ss_UNBOX_char(X)        (((ss_integer_t) (X)) >> 1)
#define ss_BOX_char(X)   ((ss) ((((ss_integer_t) (X)) & 0xff) << 1))
ss  ss_box_char(int _v);
int ss_unbox_char(ss v);

static inline ss_e_type ss_type(ss x)
{
  return ((ss_integer_t) x) & 1 ? ss_t_integer : 
          x <= ss_BOX_char(255) ? ss_t_char :
                                  ((ss_integer_t*) x)[-1];
}
static inline int ss_literalQ(ss X)
{
  return ss_t_LITERAL_MIN <= ss_type(X) && ss_type(X) <= ss_t_LITERAL_MAX;
}

#define ss_UNBOX_integer(X)        (((ss_integer_t)(X)) >> 1)
#define ss_BOX_integer(X)   ((ss) ((((ss_integer_t)(X)) << 1) | 1))
ss ss_box_integer(ss_integer_t _v);
ss_integer_t ss_unbox_integer(ss v);

typedef double ss_real_t;
typedef struct ss_s_real {
  ss_real_t value;
} ss_s_real;
#define ss_UNBOX_real(X)((ss_s_real*)(X))->value
ss ss_box_real(ss_real_t _v);
ss_real_t ss_unbox_real(ss v);

#define ss_BOX_boolean(X) ((X) ? ss_t : ss_f)
#define ss_UNBOX_boolean(X) ((X) != ss_f)

typedef struct ss_s_quote {
  ss value;
} ss_s_quote;
#define ss_UNBOX_quote(X)((ss_s_quote*)(X))->value

typedef struct ss_s_var_ref {
  ss name;
  ss_integer_t up, over;
} ss_s_var_ref;
#define ss_UNBOX_var_ref(X) (*(ss_s_var_ref*)(X))

typedef struct ss_s_if {
  ss t, a, b;
} ss_s_if;

typedef struct ss_s_cons {
  ss a, d;
} ss_s_cons;
ss ss_cons(ss _car, ss _cdr);
#define ss_CAR(X) (((ss_s_cons*) (X))->a)
#define ss_CDR(X) (((ss_s_cons*) (X))->d)
ss* _ss_car(ss a);
ss* _ss_cdr(ss a);
#define ss_car(X) (*_ss_car(X))
#define ss_cdr(X) (*_ss_cdr(X))

typedef struct ss_s_vector {
  ss *v;
  size_t l;
} ss_s_vector;
#define ss_vector_v(X) ((ss_s_vector*)(X))->v
#define ss_vector_l(X) ((ss_s_vector*)(X))->l
ss ss_vecn(size_t l);
ss ss_vec(int n, ...);

typedef char ss_string_t;
typedef struct ss_s_string {
  ss_string_t *v;
  size_t l;
} ss_s_string;
#define ss_string_v(X) ((ss_s_string*)(X))->v
#define ss_string_l(X) ((ss_s_string*)(X))->l
ss ss_strn(size_t l);

typedef struct ss_s_symbol {
  ss name;
  const char *docstring;
  ss_integer_t is_const;
} ss_s_symbol;
#define ss_UNBOX_symbol(X) (*((ss_s_symbol*)(X)))

typedef struct ss_s_environment {
  ss_integer_t argc;
  ss *symv;
  ss *argv;
  struct ss_s_environment *parent, *top_level;
  ss_integer_t constantExprQ;
} ss_s_environment;

#define ss_PROC_DECL(X) ss X (ss_s_environment *ss_env, ss *_ss_expr, unsigned int ss_argc, ss *ss_argv)
typedef struct ss_s_prim {
  ss_PROC_DECL((*func));
  const char *name;
  ss_integer_t minargs, maxargs, evalq;
  const char *docstring;
  void *c_func;
} ss_s_prim;
typedef ss_s_prim ss_s_syntax;
#define ss_UNBOX_prim(X)((ss_s_prim*)(X))
#define ss_UNBOX_syntax(X)ss_UNBOX_prim(X)

#ifndef _ss_prim
#define _ss_prim(TYPE,NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)             \
  static ss_PROC_DECL(ss_PASTE2(_ss_pf_,NAME));                         \
  ss_s_prim ss_PASTE2(_ss_p_,NAME) = { ss_PASTE2(_ss_pf_,NAME), #NAME, MINARGS, MAXARGS, EVALQ, DOCSTRING } ; \
  static ss_PROC_DECL(ss_PASTE2(_ss_pf_,NAME)) {                        \
  ss ss_rtn = ss_undef;                                                 \
  int ss_constantFold = 0;                                              \
  int ss_constantExprQAll = 1;                                          \
  if ( MINARGS >= 0 ) {                                                 \
    if ( ss_argc < MINARGS )                                            \
      _ss_min_args_error(DOCSTRING, ss_argc, MINARGS);                  \
    if ( MAXARGS >= 0 && ss_argc > MAXARGS )                            \
      _ss_max_args_error(DOCSTRING, ss_argc, MAXARGS);                  \
  }                                                                     \
  if ( EVALQ ) {                                                        \
    ss *nv = alloca(sizeof(nv[0]) * ss_argc);                           \
    unsigned int i;                                                     \
    for ( i = 0; i < ss_argc; i ++ ) {                                  \
      nv[i] = ss_exec(ss_argv[i]);                                      \
      ss_constantExprQAll &= ss_constantExprQ;                          \
    }                                                                   \
    ss_argv = nv;                                                       \
  }                                                                     \
  ss_constantExprQ = 0;                                                 \
{
#endif

#define ss_return(X) do { ss_rtn = (X); goto _ss_rtn; } while(0)
#define ss_end                                                       \
  }                                                                  \
_ss_rtn:                                                             \
 if ( (ss_constantExprQ = ss_constantFold && ss_constantExprQAll) )  \
   ss_rewrite_expr(ss_box_quote(ss_rtn), "constant folding");        \
 return(ss_rtn);                                                     \
 }

#define ss_prim(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  _ss_prim(ss_t_prim,NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)
#define ss_syntax(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  _ss_prim(ss_t_syntax,NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)

typedef struct ss_s_closure {
  ss_PROC_DECL((*_func));
  ss formals;
  ss params;
  ss body;
  ss_s_environment *env;
  ss rest;
  ss_integer_t rest_i;
} ss_s_closure;
#define ss_UNBOX_closure(X) (*(ss_s_closure*)(X))

extern ss ss_undef, ss_unspec, ss_nil, ss_t, ss_f;

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
