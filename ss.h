#ifndef _ss_h_
#define _ss_h_

#include <stdlib.h>
#include <stdio.h>

#define _ss_PASTE2(A,B)A##B
#define ss_PASTE2(A,B)_ss_PASTE2(A,B)
#define _ss_STRINGTIZE(A)#A
#define ss_STRINGTIZE(A)_ss_STRINGTIZE(A)

#define ss_malloc(X) malloc(X)

typedef enum ss_type {
  ss_t_UNDEF,
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
  ss_t_quote,
  ss_t_LITERAL_MAX = ss_t_quote,
  
  ss_t_cons,
  ss_t_null,
  ss_t_vector,
  ss_t_symbol,

  ss_t_eos,
  ss_t_port,
  
  ss_t_closure,
  ss_t_environment,

  ss_t_LAST
} ss_type;

typedef ss_type *ss_value;
#define ss_EQ(X,Y)((X)==(Y))
#define ss_NE(X,Y)!ss_EQ(X,Y)

#define ss_type_(X) (*(ss_value)(X))
#define ss_type(X) (((ss_integer_t) (X)) & 1 ? ss_t_integer : (X) <= ss_BOX_char(255) ? ss_t_char : ss_type_(X))
#define ss_REF(X)((void*)(X))
#define ss_BOX_REF(X)((ss_value)(X))
#define ss_literalQ(X)(ss_t_LITERAL_MIN <= ss_type(X) && ss_type(X) <= ss_t_LITERAL_MAX)

#define ss_UNBOX_char(X)              (((ss_integer_t) (X)) >> 1)
#define ss_BOX_char(X)   ((ss_value) ((((ss_integer_t) (X)) & 0xff) << 1))
ss_value ss_box_char(int _v);
int      ss_unbox_char(ss_value v);

typedef ssize_t ss_integer_t;
#define ss_UNBOX_integer(X)              (((ss_integer_t)(X)) >> 1)
#define ss_BOX_integer(X)   ((ss_value) ((((ss_integer_t)(X)) << 1) | 1))
ss_value ss_box_integer(ss_integer_t _v);
ss_integer_t ss_unbox_integer(ss_value v);

typedef double ss_real_t;
typedef struct ss_s_real {
  ss_type _type;
  ss_real_t _v;
} ss_s_real;
#define ss_UNBOX_real(X)((ss_s_real*)ss_REF(X))->_v
ss_value ss_box_real(ss_real_t _v);
ss_real_t ss_unbox_real(ss_value v);

typedef struct ss_s_quote {
  ss_type _type;
  ss_value _value;
} ss_s_quote;
#define ss_UNBOX_quote(X)((ss_s_quote*)ss_REF(X))->_value

typedef struct ss_s_cons {
  ss_type _type;
  ss_value _car, _cdr;
} ss_s_cons;
ss_value ss_cons(ss_value _car, ss_value _cdr);
#define ss_UNBOX_cons(X)((ss_s_cons*)ss_REF(X))
#define ss_CAR(X)(ss_UNBOX_cons(X)->_car)
#define ss_CDR(X)(ss_UNBOX_cons(X)->_cdr)
ss_value* _ss_car(ss_value a);
ss_value* _ss_cdr(ss_value a);
#define ss_car(X)(*_ss_car(X))
#define ss_cdr(X)(*_ss_cdr(X))

typedef struct ss_s_vector {
  ss_type _type;
  size_t _l;
  ss_value *_v;
} ss_s_vector;
#define ss_UNBOX_vector(X)*((ss_s_vector*)ss_REF(X))
#define ss_vector_v(X)((ss_s_vector*)ss_REF(X))->_v
#define ss_vector_l(X)((ss_s_vector*)ss_REF(X))->_l
ss_value ss_vecn(size_t l);
ss_value ss_vec1(ss_value _0);
ss_value ss_vec2(ss_value _0, ss_value _1);
ss_value ss_vec3(ss_value _0, ss_value _1, ss_value _2);
ss_value ss_vec4(ss_value _0, ss_value _1, ss_value _2, ss_value _3);
ss_value ss_vec(int n, ...);

typedef char ss_string_t;
typedef struct ss_s_string {
  ss_type _type;
  size_t _l;
  ss_string_t *_v;
} ss_s_string;
#define ss_UNBOX_string(X)*((ss_s_string*)ss_REF(X))
#define ss_string_v(X)((ss_s_string*)ss_REF(X))->_v
#define ss_string_l(X)((ss_s_string*)ss_REF(X))->_l
ss_value ss_strn(size_t l);

typedef struct ss_s_symbol {
  ss_type _type;
  ss_value _str;
  ss_value *_value;
  short _const;
} ss_s_symbol;
#define ss_UNBOX_symbol(X)*((ss_s_symbol*)ss_REF(X))
#define ss_symbol_value(X)*(((ss_s_symbol*)ss_REF(X))->_value)
#define ss_symbol_const(X)(((ss_s_symbol*)ss_REF(X))->_const)

typedef struct ss_s_environment {
  ss_type _type;
  int constantExprQ;
} ss_s_environment;

#define ss_PROC_DECL(X) ss_value X (ss_s_environment *ss_env, unsigned int ss_argc, ss_value *ss_argv)
typedef struct ss_s_prim {
  ss_type _type;
  const char *_name;
  ss_PROC_DECL((*_func));
} ss_s_prim;
typedef ss_s_prim ss_s_syntax;
#define ss_UNBOX_prim(X)((ss_s_prim*)ss_REF(X))
#define ss_UNBOX_syntax(X)ss_UNBOX_prim(X)

#ifndef _ss_prim
#define _ss_prim(TYPE,NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)      \
  static ss_PROC_DECL(ss_PASTE2(_ss_f_prim_,NAME)); \
  ss_s_prim ss_PASTE2(_ss_prim_,NAME) = { TYPE, #NAME, ss_PASTE2(_ss_f_prim_,NAME) } ; \
static ss_PROC_DECL(ss_PASTE2(_ss_f_prim_,NAME)) { \
  if ( MINARGS >= 0 ) { \
    if ( ss_argc < MINARGS ) \
      _ss_min_args_error(DOCSTRING, ss_argc, MINARGS); \
    if ( MAXARGS >= 0 && ss_argc > MAXARGS ) \
      _ss_max_args_error(DOCSTRING, ss_argc, MAXARGS); \
  } \
  if ( EVALQ ) { \
    ss_value *nv = alloca(sizeof(nv[0]) * ss_argc); \
    unsigned int i; \
    for ( i = 0; i < ss_argc; i ++ ) \
      nv[i] = ss_exec(ss_argv[i]); \
    ss_argv = nv; \
  } {
#endif

#define ss_prim(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  _ss_prim(ss_t_prim,NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)
#define ss_syntax(NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  _ss_prim(ss_t_syntax,NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING)
#define ss_return(X) return(X)
#define ss_end }}

struct ss_s_closure {
  ss_type _type;
  ss_value formals;
  ss_value args;
  ss_value body;
  struct ss_s_closure *parent;
};
typedef struct ss_s_closure ss_s_closure;

#define ss_box(T,X)ss_PASTE2(ss_box_,T)(X)
#define ss_BOX(T,X)ss_PASTE2(ss_BOX_,T)(X)
#define ss_unbox(T,X)ss_PASTE2(ss_unbox_,T)(X)
#define ss_UNBOX(T,X)ss_PASTE2(ss_UNBOX_,T)(X)

extern ss_type ss_undef[], ss_unspec[], ss_nil[], ss_t[], ss_f[];

#ifndef ss_sym
#define ss_sym(X)ss_PASTE2(_ss_sym_,X)
#endif
extern ss_value _ss_syms
#define ss_sym_def(X),ss_PASTE2(_ss_sym_,X)
#include "sym.def"
;

#endif
