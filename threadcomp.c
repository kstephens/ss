#include "ss.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h> /* memcpy() */

ss_type ss_undef[] = { ss_t_undef };
ss_type ss_unspec[] = { ss_t_unspec };
ss_type ss_nil[] = { ss_t_null };
ss_type ss_t[] = { ss_t_boolean };
ss_type ss_f[] = { ss_t_boolean };
ss_type ss_eos[] = { ss_t_eos };

#define ss_sym_def(X) ss_value ss_PASTE2(_ss_sym_,X);
#include "sym.def"

#define ss_typecheck(T,V)((void)0)

ss_value ss_box_integer(ss_integer_t v)
{
  return ss_BOX_integer(v);
}

ss_integer_t ss_unbox_integer(ss_value v)
{
  ss_typecheck(ss_t_integer,v);
  return ss_UNBOX_integer(v);
}

ss_value ss_box_real(ss_real_t v)
{
  ss_s_real *self = ss_malloc(sizeof(*self));
  ss_type_(self) = ss_t_real;
  self->_v = v;
  return ss_BOX_REF(self);
}

ss_real_t ss_unbox_real(ss_value v)
{
  ss_typecheck(ss_t_real,v);
  return ss_UNBOX_real(v);
}

ss_value ss_box_char(int _v)
{
  return ss_BOX_char(_v);
}
int      ss_unbox_char(ss_value v)
{
  ss_typecheck(ss_t_char, v);
  return ((int) ss_UNBOX_char(v)) & 0xff;
}

ss_value ss_strn(size_t l)
{
  ss_s_string *self = ss_malloc(sizeof(*self));
  ss_type_(self) = ss_t_string;
  self->_l = l;
  self->_v = ss_malloc(sizeof(self->_v[0]) * (l + 1));
  return ss_BOX_REF(self);
}

ss_value ss_strnv(size_t l, const char *v)
{
  ss_value self = ss_strn(l);
  memcpy(ss_string_v(self), v, sizeof(ss_string_v(self)[0]) * l);
  ss_string_v(self)[l] = 0;
  return self;
}

ss_value ss_string_TO_number(ss_value s, int radix)
{
  char *endp = 0;
  long long n = strtoll(ss_string_v(s), &endp, radix);
  return ss_box(integer, n);
}

static ss_value symbols = ss_nil;
ss_value ss_box_symbol(const char *name)
{
  ss_s_symbol *self;
  ss_value l;

  for ( l = symbols; l != ss_nil; l = ss_cdr(l) ) {
    ss_value e = ss_car(l);
    ss_value n = ss_car(e);
    if ( strcmp(name, ss_string_v(n)) == 0 )
      return ss_cdr(e);
  }

  self = ss_malloc(sizeof(*self));
  ss_type_(self) = ss_t_symbol;
  self->_str = ss_strnv(strlen(name), name);
  self->_value = 0;
  self->_const = 0;

  symbols = ss_cons(ss_cons(self->_str, ss_BOX_REF(self)), symbols);

  return ss_BOX_REF(self);
}

void ss_init_symbol()
{
#define ss_sym_def(X) ss_PASTE2(_ss_sym_, X) = ss_box_symbol(#X);
#include "sym.def"
  ss_sym(ADD) = ss_box_symbol("+");
  ss_sym(DOT) = ss_box_symbol(".");
  ss_sym(unquote_splicing) = ss_box_symbol("unquote-splicing");
}


ss_value ss_box_quote(ss_value v)
{
  if ( ss_literalQ(v) ) {
    return(v);
  } else {
    ss_s_quote *self = ss_malloc(sizeof(*self));
    ss_type_(self) = ss_t_quote;
    self->_value = v;
    return ss_BOX_REF(self);
  }
}

ss_value ss_unbox_quote(ss_value v)
{
  ss_typecheck(ss_t_quote,v);
  return ss_UNBOX_quote(v);
}

ss_value ss_cons(ss_value a, ss_value d)
{
  ss_s_cons *self = ss_malloc(sizeof(*self));
  ss_type_(self) = ss_t_cons;
  self->_car = a;
  self->_cdr = d;
  return ss_BOX_REF(self);
}
ss_value* _ss_car(ss_value a)
{
  ss_typecheck(ss_t_cons,a);
  return &ss_CAR(a);
}
ss_value* _ss_cdr(ss_value a)
{
  ss_typecheck(ss_t_cons,a);
  return &ss_CDR(a);
}

size_t ss_list_length(ss_value x)
{
  size_t l = 0;
  
  again:
  switch ( ss_type(x) ) {
    case ss_t_cons:
      x = ss_CDR(x);
      l ++;
      goto again;
    case ss_t_null:
      return l;
    case ss_t_vector:
      return l + ss_vector_l(x);
  default: abort();
  }
}

ss_value ss_list_to_vector(ss_value x)
{
  size_t l = 0;
  ss_value v = ss_vecn(ss_list_length(x));
  again:
  switch ( ss_type(x) ) {
    case ss_t_cons:
      ss_vector_v(v)[l ++] = ss_CAR(x);
      x = ss_CDR(x);
      goto again;
    case ss_t_null:
      return x;
    case ss_t_vector:
      memcpy(ss_vector_v(v) + l, ss_vector_v(x), sizeof(ss_vector_v(v)[0]) * ss_vector_l(x));
      return v;
  default: abort();
  }
  return v;
}

ss_value ss_vecn(size_t l)
{
  ss_s_vector *self = ss_malloc(sizeof(*self));
  ss_type_(self) = ss_t_vector;
  self->_l = l;
  self->_v = ss_malloc(sizeof(self->_v[0]) * l);
  return ss_BOX_REF(self);
}

ss_value ss_vecnv(size_t l, const ss_value *v)
{
  ss_value self = ss_vecn(l);
  memcpy(ss_vector_v(self), v, sizeof(ss_vector_v(self)[0]) * l);
  return self;
}

ss_value ss_vec1(ss_value _0)
{
  ss_value x = ss_vecn(1);
  ss_vector_v(x)[0] = _0;
  return x;
}
ss_value ss_vec2(ss_value _0, ss_value _1)
{
  ss_value x = ss_vecn(2);
  ss_vector_v(x)[0] = _0;
  ss_vector_v(x)[1] = _1;
  return x;
}
ss_value ss_vec3(ss_value _0, ss_value _1, ss_value _2)
{
  ss_value x = ss_vecn(3);
  ss_vector_v(x)[0] = _0;
  ss_vector_v(x)[1] = _1;
  ss_vector_v(x)[2] = _2;
  return x;
}
ss_value ss_vec4(ss_value _0, ss_value _1, ss_value _2, ss_value _3)
{
  ss_value x = ss_vecn(4);
  ss_vector_v(x)[0] = _0;
  ss_vector_v(x)[1] = _1;
  ss_vector_v(x)[2] = _2;
  ss_vector_v(x)[3] = _3;
  return x;
}
ss_value ss_vec(int n, ...)
{
  ss_value x = ss_vecn(n);
  int i;
  va_list vap;
  va_start(vap,n);
  
  for ( i = 0; i < n; i ++ ) {
    ss_vector_v(x)[i] = va_arg(vap, ss_value);
  }
  va_end(vap);
  return x;
}


ss_value ss_error(const char *format, ...)
{
  va_list vap;
  va_start(vap, format);
  fprintf(stderr, "error ");
  vfprintf(stderr, format, vap);
  fprintf(stderr, "\n");
  va_end(vap);
  abort();
  return 0;
}

void _ss_min_args_error(const char *DOCSTRING, int ss_argc, int MINARGS)
{
  ss_error("apply not-enough-args %s got %d expected %d", DOCSTRING, ss_argc, MINARGS);
}

void _ss_max_args_error(const char *DOCSTRING, int ss_argc, int MAXARGS)
{
  ss_error("apply too-many-args %s got %d expected %d", DOCSTRING, ss_argc, MAXARGS);
}

ss_value _ss_exec(ss_s_environment *ss_env, ss_value *_ss_expr);
#define ss_exec(X) _ss_exec(ss_env, &(X))
#define ss_constantExprQ ss_env->constantExprQ

ss_syntax(quote,1,1,0,"(quote <value>)")
  ss_constantExprQ = 1;
  ss_return(ss_box(quote,ss_argv[0]));
ss_end

ss_syntax(if,2,3,0,"(if <pred> <true> ?<false>?)")
  ss_value x;
  x = ss_exec(ss_argv[0]);
  
  if ( ss_constantExprQ ) {
    ss_return(ss_NE(x,ss_f) ? ss_exec(ss_argv[1]) : (ss_argc == 3 ? ss_exec(ss_argv[2]) : (ss_constantExprQ = 1, ss_undef)) );
  } else {
    ss_constantExprQ = 0;
    ss_return(ss_vec4(ss_sym(_if), x, ss_exec(ss_argv[1]), (ss_argc == 3 ? ss_exec(ss_argv[2]) : ss_undef)));
  }
ss_end

ss_prim(_if,-1,-1,0,"(if <pred> <true> ?<false>?)")
  ss_value x = ss_exec(ss_argv[0]);
  ss_return(ss_NE(x,ss_f) ? ss_exec(ss_argv[1]) : ss_exec(ss_argv[2]));
ss_end

ss_syntax(lambda,2,-1,0,"(lambda formals . body)")
  ss_return(ss_vec(3, ss_sym(_lambda), ss_box(quote, ss_argv[0]), ss_box(quote, ss_vecnv(ss_argc - 1, ss_argv + 1))));
ss_end

ss_syntax(car,1,1,1,"(car <pair>)")
  if ( ss_constantExprQ ) {
    ss_constantExprQ = 1;
    ss_return(ss_box(quote,ss_car(ss_argv[0])));
  } else {
    ss_constantExprQ = 0;
    ss_return(ss_vec2(ss_sym(_car), ss_argv[0]));
  }
ss_end

ss_prim(_car,-1,-1,1,"(car <pair>)")
  ss_typecheck(ss_t_cons,ss_argv[0]);
  ss_return(ss_CAR(ss_argv[0]));
ss_end

ss_syntax(cdr,2,2,1,"(cdr <pair>)")
  if ( ss_constantExprQ ) {
    ss_constantExprQ = 1;
    ss_return(ss_box(quote,ss_cdr(ss_argv[0])));
  } else {
    ss_constantExprQ = 0;
    ss_return(ss_vec2(ss_sym(_cdr), ss_argv[0]));
  }
ss_end

ss_prim(_cdr,-1,-1,1,"(cdr <pair>)")
  ss_typecheck(ss_t_cons,ss_argv[0]);
  ss_return(ss_CDR(ss_argv[0]));
ss_end

ss_syntax(ADD,0,-1,1,"(+ <z>...)")
  switch ( ss_argc ) {
  case 0:
    ss_return(ss_box(integer,0));
  case 1:
    ss_return(ss_argv[0]);
  case 2:
    ss_return(ss_vec3(ss_sym(_add), ss_argv[0], ss_argv[1]));
  default:
    ss_return(ss_vec3(ss_sym(_add), ss_argv[1], ss_cons(ss_sym(ADD), ss_vec(ss_argc - 1, ss_argv + 1))));
  }
ss_end

static
void ss_number_coerce_2(ss_value *argv)
{
  switch ( ss_type(argv[0]) ) {
  case ss_t_integer:
    switch ( ss_type(argv[1]) ) {
    case ss_t_real:
      argv[0] = ss_box(real, ss_UNBOX(integer, argv[0]));
      break;
    case ss_t_integer:
      break;
    default: abort();
    }
    break;
  case ss_t_real:
    switch ( ss_type(argv[1]) ) {
    case ss_t_integer:
      argv[1] = ss_box(real, ss_UNBOX(integer, argv[1]));
      break;
    case ss_t_real: break;
    default: abort();
    }
    break;
  default: abort();
  }
}

ss_prim(_add,-1,-1,1,"(+ <z>...)")
  ss_number_coerce_2(ss_argv);
  switch ( ss_type(ss_argv[0]) ) {
  case ss_t_integer:
    ss_return(ss_box(integer, ss_UNBOX(integer,ss_argv[0]) + ss_UNBOX(integer,ss_argv[1])));
  case ss_t_real:
    ss_return(ss_box(real, ss_UNBOX(real,ss_argv[0]) + ss_UNBOX(real,ss_argv[1])));
  default: abort();
  }
ss_end


ss_syntax(SUB,0,-1,1,"(- <z>...)")
  switch ( ss_argc ) {
  case 0:
    ss_return(ss_box(integer,0));
  case 1:
    ss_return(ss_vec2(ss_sym(_neg), ss_argv[0]));
  case 2:
    ss_return(ss_vec3(ss_sym(_sub), ss_argv[0], ss_argv[1]));
  default:
    ss_return(ss_vec3(ss_sym(_sub), ss_argv[1], ss_cons(ss_sym(ADD), ss_vec(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_prim(_sub,-1,-1,1,"(- <z>...)")
  ss_number_coerce_2(ss_argv);
  switch ( ss_type(ss_argv[0]) ) {
  case ss_t_integer:
    ss_return(ss_box(integer, ss_UNBOX(integer,ss_argv[0]) - ss_UNBOX(integer,ss_argv[1])));
  case ss_t_real:
    ss_return(ss_box(real, ss_UNBOX(real,ss_argv[0]) - ss_UNBOX(real,ss_argv[1])));
  default: abort();
  }
ss_end

ss_prim(_neg,-1,-1,1,"(- <z>)")
  switch ( ss_type(ss_argv[0]) ) {
  case ss_t_integer:
    ss_return(ss_box(integer, - ss_UNBOX(integer,ss_argv[0])));
  case ss_t_real:
    ss_return(ss_box(real, - ss_UNBOX(real,ss_argv[0])));
  default: abort();
  }
ss_end

#define ss_expr (*_ss_expr)

ss_value _ss_exec(ss_s_environment *ss_env, ss_value *_ss_expr)
{
  again:
  switch ( ss_type(ss_expr) ) {
  case ss_t_integer:
  case ss_t_real:
  case ss_t_undef:
  case ss_t_boolean:
  case ss_t_string:
  case ss_t_syntax:
  case ss_t_prim:
    ss_constantExprQ = 1;
    ss_return(ss_expr);
    
  case ss_t_quote:
    ss_constantExprQ = 1;
    ss_return(ss_UNBOX(quote,ss_expr));
  
  case ss_t_symbol: {
    ss_value v = ss_symbol_value(ss_expr);
    if ( (ss_constantExprQ = ss_symbol_const(ss_expr)) ) {
      ss_expr = ss_box(quote,v);
    }
    ss_return(v);
  }
    
  case ss_t_cons:
    ss_expr = ss_list_to_vector(ss_expr);
    /* FALL THROUGH */
    
  case ss_t_vector: {
    ss_value op = ss_exec(ss_vector_v(ss_expr)[0]);
    if ( ss_constantExprQ ) {
      ss_vector_v(ss_expr)[0] = op;
    }
    switch ( ss_type(op) ) {
    case ss_t_syntax:
      ss_expr = (ss_UNBOX(syntax,op)->_func)(ss_env, ss_vector_l(ss_expr) - 1, ss_vector_v(ss_expr) + 1);
      goto again;
      
    case ss_t_prim:
      ss_return((ss_UNBOX(prim,op)->_func)(ss_env, ss_vector_l(ss_expr) - 1, ss_vector_v(ss_expr) + 1));
      
    default:
      ss_error("apply cannot-apply ~S", op);
    }
  }
  default:
    ss_error("eval cannot-eval ~S", ss_expr);
  }
  ss_return(ss_expr);
}

int main(int argc, char **argv)
{
  ss_init_symbol();
  return 0;
}

ss_value ss_read(ss_value port);

ss_prim(_read,1,1,1,"(_read <port>)")
{
  ss_return(ss_read(ss_argv[0]));
}
ss_end

#define VALUE ss_value
#define READ_DECL ss_value ss_read(ss_value stream)
#define READ_CALL() ss_read(stream)
#define FP(stream) (*(FILE**)stream)
#define GETC(stream) getc(FP(stream))
#define UNGETC(stream,c) ungetc(c, FP(stream))
#define EQ(X,Y) ((X) == (Y))
#define NIL ss_nil
#define EOS ss_eos
#define CONS ss_cons
#define CAR ss_car
#define T ss_t
#define F ss_f
#define E ss_eos
#define BRACKET_LISTS 1
#define SET_CDR(C,R) ss_CDR(C) = (R)
#define MAKE_CHAR(I) ss_box(char, (I))
#define LIST_2_VECTOR(X) ss_list_to_vector(X)
#define STRING(S,L) ss_strnv((L), (S))
#define SYMBOL_DOT ss_sym(DOT)
#define SYMBOL(N) ss_sym(N)
#define STRING_2_NUMBER(s, radix) ss_string_TO_number(s, radix)
#define STRING_2_SYMBOL(s) ss_box(symbol, ss_string_v(s))
#define ERROR(msg,args...) ss_error("read: " msg, ##args)
#define RETURN(X) return X
#if 0
#define MALLOC(S) GC_malloc_atomic(S)
#define REALLOC(P,S) GC_realloc(P,S)
#define FREE(P) GC_free(P)
#endif
#include "lispread/lispread.c"

