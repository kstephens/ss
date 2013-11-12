#include "ss.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h> /* memcpy() */
#include <assert.h>

FILE **ss_stdin = &stdin, **ss_stdout = &stdout, **ss_stderr = &stderr;
ss ss_write(ss obj, ss port);

size_t ss_malloc_bytes, ss_malloc_objects;
#undef ss_malloc
void* ss_malloc(size_t s)
{
  ss_malloc_bytes += s;
  ss_malloc_objects ++;
  return GC_malloc(s);
}

ss ss_eqQ(ss a, ss b)
{
  return a == b ? ss_t : ss_f;
}

ss ss_undef, ss_unspec, ss_nil, ss_t, ss_f, ss_eos;

ss _ss_exec(ss_s_environment *ss_env, ss *_ss_expr);
#define ss_expr (*_ss_expr)
#define ss_exec(X) _ss_exec(ss_env, &(X))
#define ss_constantExprQ ss_env->constantExprQ
#define ss_rewrite_verbose 0
#define ss_exec_verbose 0
static inline
void _ss_rewrite_expr(ss *_ss_expr, ss X, const char *REASON)
{
  if ( ss_rewrite_verbose ) {
    fprintf(*ss_stderr, ";; rewrite: ");
    ss_write(ss_expr, ss_stderr);
    fprintf(*ss_stderr, "\n;;  reason: %s\n", (REASON));
  }
  ss_expr = X;
  if ( ss_rewrite_verbose ) {
    fprintf(*ss_stderr, ";;      as: ");
    ss_write(ss_expr, ss_stderr);
    fprintf(*ss_stderr, "\n");
  }
}
#if ss_rewrite_verbose == 0
#define ss_rewrite_expr(X,REASON) (ss_expr = (X))
#else
#define ss_rewrite_expr(X,REASON) _ss_rewrite_expr(&ss_expr, X, REASON)
#endif

ss ss_alloc(ss_e_type type, size_t size)
{
  void *ptr = ss_malloc(sizeof(ss_integer_t) + size);
  *((ss_integer_t*) ptr) = type;
  ptr += sizeof(ss_integer_t);
  return ptr;
}

ss ss_alloc_copy(ss_e_type type, size_t size, void *ptr)
{
  void *self = ss_alloc(type, size);
  memcpy(self, ptr, size);
  return self;
}

#define FP(port) (*(FILE**) (port))
ss ss_error(const char *format, ss obj, ...)
{
  va_list vap;
  va_start(vap, obj);
  fprintf(FP(ss_stderr), "\n  ss: error ");
  vfprintf(FP(ss_stderr), format, vap);
  fprintf(FP(ss_stderr), ": ");
  ss_write(obj, ss_stderr);
  fprintf(FP(ss_stderr), "\n");
  va_end(vap);
  abort();
  return 0;
}

void ss_write_real(ss v, ss port)
{
  char buf[64];
  snprintf(buf, 63, "%.22g", ss_unbox(real, v));
  if ( ! (strchr(buf, 'e') || strchr(buf, '.')) ) {
    strcat(buf, ".0");
  }
  fprintf(FP(port), "%s", buf);
}

ss ss_write(ss v, ss port)
{
  FILE *out = FP(port);
  switch ( ss_type(v) ) {
  case ss_t_undef:   fprintf(out, "#<undef>"); break;
  case ss_t_integer: fprintf(out, "%lld",   (long long) ss_unbox(integer, v)); break;
  case ss_t_real:    ss_write_real(v, port); break;
  case ss_t_string:  fprintf(out, "\"%s\"", ss_string_v(v)); break;
  case ss_t_char:    fprintf(out, "#\\%c",  ss_unbox(char, v)); break;
  case ss_t_boolean: fprintf(out, "#%c",    v == ss_t ? 't' : 'f'); break;
  case ss_t_syntax:  fprintf(out, "#<syntax %s>", ss_UNBOX(syntax, v)->name); break;
  case ss_t_prim:    fprintf(out, "#<prim %s>",   ss_UNBOX(prim, v)->name); break;
  case ss_t_symbol:  fprintf(out, "%s",   ss_string_v(ss_UNBOX(symbol, v).name)); break;
  case ss_t_if:
    {
      ss_s_if *self = v;
      fprintf(out, "(if ");
      ss_write(self->t, port);
      fprintf(out, " ");
      ss_write(self->a, port);
      fprintf(out, " ");
      ss_write(self->b, port);
      fprintf(out, ")");
    }
    break;
  case ss_t_var:
    fprintf(out, "#<v ");
    ss_write(ss_UNBOX(var, v).name, port);
    fprintf(out, " %d %d>", (int) ss_UNBOX(var, v).up, (int) ss_UNBOX(var, v).over);
    break;
  case ss_t_global:
    fprintf(out, "#<g ");
    ss_write(((ss_s_global*) v)->name, port);
    fprintf(out, ">");
    break;
  case ss_t_quote:   fprintf(out, "'"); ss_write(ss_UNBOX(quote, v), port); break;
  case ss_t_eos:     fprintf(out, "#<eos>"); break;
  case ss_t_null:    fprintf(out, "()"); break;
  case ss_t_closure:
    fprintf(out, "#<closure ");
    ss_write(ss_UNBOX(closure, v).formals, port);
    fprintf(out, ">");
    break;
  case ss_t_port:
    fprintf(out, "#<port ");
    ss_write(ss_UNBOX(port, v).name, port);
    fprintf(out, " ");
    ss_write(ss_UNBOX(port, v).mode, port);
    fprintf(out, ">");
    break;
  default:           fprintf(out, "#<??? %d @%p>", ss_type(v), (void*) v); break;
  case ss_t_pair:
    fprintf(out, "(");
    while ( v != ss_nil ) {
      switch ( ss_type(v) ) {
      case ss_t_pair:
        ss_write(ss_car(v), port);
        v = ss_cdr(v);
        if ( v != ss_nil )
          fprintf(out, " ");
        break;
      default:
        fprintf(out, ". ");
        ss_write(v, port);
        v = ss_nil;
        break;
      }
    }
    fprintf(out, ")");
    break;
  case ss_t_vector:
    {
      size_t i = 0;
      fprintf(out, "#(");
      while ( i < ss_vector_l(v) ) {
        ss_write(ss_vector_v(v)[i], port);
        if ( ++ i < ss_vector_l(v) )
          fprintf(out, " ");
      }
      fprintf(out, ")");
    }
    break;
  }
  return ss_undef;
}

#define ss_sym_def(X) ss ss_PASTE2(_ss_sym_,X);
#include "sym.def"

#define ss_typecheck(T,V)((void)0)

ss ss_box_integer(ss_integer_t v)
{
  return ss_BOX_integer(v);
}
ss ss_i(ss_integer_t v) { return ss_box_integer(v); }

ss_integer_t ss_unbox_integer(ss v)
{
  ss_typecheck(ss_t_integer,v);
  return ss_UNBOX_integer(v);
}
ss_integer_t ss_I(ss v) { return ss_UNBOX_integer(v); }

ss ss_box_real(ss_real_t v)
{
  ss_s_real *self = ss_alloc(ss_t_real, sizeof(*self));
  self->value = v;
  return self;
}
ss ss_r(ss v)
{
  return ss_box_real(*(double*) &v);
}

ss_real_t ss_unbox_real(ss v)
{
  ss_typecheck(ss_t_real, v);
  return ss_UNBOX_real(v);
}
ss ss_R(ss v)
{
  ss rtn;
  *((double*) &rtn) = ss_unbox_real(v);
  return rtn;
}

ss ss_GI(ss o, ss i)
{
  return ((ss*) o)[ss_I(i)];
}
ss ss_SI(ss o, ss i, ss v)
{
  return ((ss*) o)[ss_I(i)] = v;
}

ss ss_box_char(int _v)
{
  return ss_BOX_char(_v);
}
int ss_unbox_char(ss v)
{
  ss_typecheck(ss_t_char, v);
  return ((int) ss_UNBOX_char(v)) & 0xff;
}

ss ss_strn(size_t l)
{
  ss_s_string *self = ss_alloc(ss_t_string, sizeof(*self));
  self->v = ss_malloc(sizeof(self->v[0]) * (l + 1));
  self->l = l;
  return self;
}

ss ss_strnv(size_t l, const char *v)
{
  ss self = ss_strn(l);
  memcpy(ss_string_v(self), v, sizeof(ss_string_v(self)[0]) * l);
  ss_string_v(self)[l] = 0;
  return self;
}

ss ss_s(const char *p)
{
  if ( ! p ) return ss_f;
  return ss_strnv(strlen(p), (void*) p);
}
ss ss_S(ss p)
{
  if ( p == ss_f || p == ss_nil ) return 0;
  return ss_string_v(p);
}

ss ss_string_TO_number(ss s, int radix)
{
  char *endp;
  double d;
  long long ll;

  ll = strtoll(ss_string_v(s), &endp, radix);
  if ( ! *endp )
    return ss_box(integer, ll);
  d = strtod(ss_string_v(s), &endp);
  if ( ! *endp )
    return ss_box(real, d);
  return ss_f;
}

static ss ss_symbols;
ss ss_box_symbol(const char *name)
{
  ss_s_symbol *sym;
  {
    ss l;
    for ( l = ss_symbols; l != ss_nil; l = ss_cdr(l) ) {
      sym = (ss_s_symbol*) ss_car(l);
      if ( strcmp(name, ss_string_v(sym->name)) == 0 )
        goto rtn;
    }
  }

  sym = ss_alloc(ss_t_symbol, sizeof(*sym));
  sym->name = ss_strnv(strlen(name), name);
  sym->is_const = 0;

  ss_symbols = ss_cons(sym, ss_symbols);

 rtn:
  // fprintf(stderr, "  symbol(%s) => %p\n", name, sym);
  return sym;
}

void ss_init_symbol(ss_s_environment *ss_env)
{
#define ss_sym_def(X) ss_PASTE2(_ss_sym_, X) = ss_box_symbol(#X);
#include "sym.def"
#define BOP(NAME,OP) ss_sym(NAME) = ss_box_symbol(#OP);
#define UOP(NAME,OP) BOP(NAME,OP)
#define ROP(NAME,OP) BOP(NAME,OP)
#define IBOP(NAME,OP) BOP(NAME,OP)
#define IUOP(NAME,OP) BOP(NAME,OP)
#include "cops.def"
  ss_sym(DOT) = ss_box_symbol(".");
  ss_sym(setE) = ss_box_symbol("set!");
  ss_sym(_rest) = ss_box_symbol("&rest");
  ss_sym(unquote_splicing) = ss_box_symbol("unquote-splicing");
}

ss ss_box_quote(ss v)
{
  if ( ss_literalQ(v) ) {
    return(v);
  } else {
    ss_s_quote *self = ss_alloc(ss_t_quote, sizeof(*self));
    self->value = v;
    return self;
  }
}

ss ss_cons(ss a, ss d)
{
  ss_s_cons *self = ss_alloc(ss_t_pair, sizeof(*self));
  self->a = a;
  self->d = d;
  return self;
}
ss* _ss_car(ss a)
{
  ss_typecheck(ss_t_pair,a);
  return &ss_CAR(a);
}
ss* _ss_cdr(ss a)
{
  ss_typecheck(ss_t_pair,a);
  return &ss_CDR(a);
}

ss ss_listnv(size_t n, const ss *v)
{
  ss l = ss_nil, *lp = &l;
  while ( n -- ) {
    *lp = ss_cons(*(v ++), ss_nil);
    lp = &ss_CDR(*lp);
  }
  return l;
}

size_t ss_list_length(ss x)
{
  size_t l = 0;
  
  again:
  switch ( ss_type(x) ) {
  case ss_t_pair:
    x = ss_CDR(x);
    l ++;
    goto again;
  case ss_t_null:
    return l;
  case ss_t_vector:
    return l + ss_vector_l(x);
  default:
    return l + 1;
  }
}

ss ss_list_to_vector(ss x)
{
  size_t l = 0;
  ss v = ss_vecn(ss_list_length(x));
  again:
  switch ( ss_type(x) ) {
  case ss_t_pair:
    ss_vector_v(v)[l ++] = ss_CAR(x);
    x = ss_CDR(x);
    goto again;
  case ss_t_null:
    break;
  case ss_t_vector:
    memcpy(ss_vector_v(v) + l, ss_vector_v(x), sizeof(ss_vector_v(v)[0]) * ss_vector_l(x));
    break;
  default:
    ss_vector_v(v)[l] = ss_cons(ss_sym(_rest), x);
    break;
  }
  return v;
}

ss ss_vecn(size_t l)
{
  ss_s_vector *self = ss_alloc(ss_t_vector, sizeof(*self));
  self->v = ss_malloc(sizeof(self->v[0]) * l);
  self->l = l;
  return self;
}

ss ss_vecnv(size_t l, const ss *v)
{
  ss_s_vector *self = ss_vecn(l);
  memcpy(self->v, v, sizeof(self->v[0]) * l);
  return self;
}

ss ss_vec(int n, ...)
{
  ss x = ss_vecn(n);
  int i = 0;
  va_list vap;
  va_start(vap,n);
  while ( i < n )
    ss_vector_v(x)[i ++] = va_arg(vap, ss);
  va_end(vap);
  return x;
}

ss ss_m_environment(ss_s_environment *parent)
{
  ss_s_environment *env = ss_alloc(ss_t_environment, sizeof(*env));
  env->constantExprQ = 0;
  env->argc = 0;
  env->symv = env->argv = 0;
  env->parent = parent;
  env->top_level = parent ? parent->top_level : env;
  return env;
}

ss ss_m_var(ss sym, int up, int over)
{
  ss_s_var *self = ss_alloc(ss_t_var, sizeof(*self));
  self->name = sym;
  self->up = up;
  self->over = over;
  return self;
}

ss ss_m_global(ss sym, ss ref)
{
  ss_s_global *self = ss_alloc(ss_t_global, sizeof(*self));
  self->name = sym;
  self->ref = ref;
  return self;
}

ss ss_define(ss_s_environment *env, ss sym, ss val)
{
  int i;
  for ( i = 0; i < env->argc; ++ i )
    if ( ss_EQ(sym, env->symv[i]) )
      return env->argv[i] = val;

  env->symv = memcpy(ss_malloc(sizeof(env->symv) * (env->argc + 1)), env->symv, sizeof(env->symv[0]) * env->argc);
  env->symv[env->argc] = sym;
  env->argv = memcpy(ss_malloc(sizeof(env->argv) * (env->argc + 1)), env->argv, sizeof(env->argv[0]) * env->argc);
  env->argv[env->argc] = val;
  ++ env->argc;

  return sym;
}

ss* ss_bind(ss *_ss_expr, ss_s_environment *env, ss var)
{
  int up, over;
  ss *ref;
  switch ( ss_type(var) ) {
  case ss_t_symbol:
    up = 0;
    while ( env ) {
      for ( over = 0; over < env->argc; ++ over ) {
        // fprintf(*ss_stderr, ";; bind "); ss_write(var, ss_stdout); fprintf(*ss_stderr, " = "); ss_write(env->symv[over], ss_stdout); fprintf(*ss_stderr, "\n");
        if ( ss_EQ(var, env->symv[over]) ) {
          ss_rewrite_expr(ss_m_var(var, up, over), "var binding is known");
          ref = &env->argv[over];
          goto rtn;
        }
      }
      ++ up;
      env = env->parent;
    }
    break;
  case ss_t_var:
    up   = ss_UNBOX(var, var).up;
    over = ss_UNBOX(var, var).over;
    while ( up -- > 0 ) env = env->parent;
    assert(env);
    ref = &env->argv[over];
    goto rtn;
  default: break;
  }
  return(ss_error("unbound", var));

 rtn:
  if ( ss_type(*ref) == ss_t_global ) {
    ss_rewrite_expr(*ref, "global binding is known");
    ref = &ss_UNBOX(global, *ref);
  }
  return ref;
}

ss ss_set(ss *_ss_expr, ss_s_environment *env, ss var, ss val)
{
  *ss_bind(_ss_expr, env, var) = val;
  return var;
}

ss ss_get(ss *_ss_expr, ss_s_environment *env, ss var)
{
  return *ss_bind(_ss_expr, env, var);
}

#define ss_symbol_value(X) ss_get(&ss_expr, ss_env, (X))

void _ss_min_args_error(ss op, const char *DOCSTRING, int ss_argc, int MINARGS)
{
  ss_error("apply not-enough-args (%s) got %d expected %d", op, DOCSTRING, ss_argc, MINARGS);
}

void _ss_max_args_error(ss op, const char *DOCSTRING, int ss_argc, int MAXARGS)
{
  ss_error("apply too-many-args (%s) got %d expected %d", op, DOCSTRING, ss_argc, MAXARGS);
}

ss_syntax(define,1,-1,0,"define name value") {
  ss name = ss_argv[0];
  if ( ss_type(name) == ss_t_pair ) {
    ss_return(ss_cons(ss_sym(define), ss_cons(ss_car(name), ss_cons(ss_cons(ss_sym(lambda), ss_cons(ss_cdr(name), ss_listnv(ss_argc - 1, ss_argv + 1))), ss_nil))));
  } else {
    ss_return(ss_cons(ss_sym(_define), ss_cons(name, ss_cons(ss_argv[1], ss_nil))));
  }
} ss_end

ss_prim(_define,2,2,0,"define name value") {
  ss_return(ss_define(ss_env->top_level, ss_argv[0], ss_exec(ss_argv[1])));
} ss_end

ss_prim(setE,2,2,0,"set! name value") {
  ss_return(ss_set(&ss_argv[0], ss_env, ss_argv[0], ss_exec(ss_argv[1])));
} ss_end

ss ss_read(ss port);
ss_prim(read,0,1,1,"_read port")
{
  ss_return(ss_read(ss_argc > 0 ? ss_argv[0] : ss_stdin));
}
ss_end

ss_prim(write,1,2,1,"write object")
  ss_write(ss_argv[0], ss_argc > 1 ? ss_argv[1] : ss_stdout);
ss_end

ss_prim(newline,0,1,1,"newline")
  FILE **out = ss_argc > 0 ? ss_argv[0] : ss_stdin;
  fprintf(*out, "\n");
ss_end

ss_syntax(quote,1,1,0,"quote value")
  ss_return(ss_box_quote(ss_argv[0]));
ss_end

ss_syntax(if,2,3,0,"if pred true ?false?") {
  ss_s_if *self = ss_alloc(ss_t_if, sizeof(*self));
  self->t = ss_argv[0];
  self->a = ss_argv[1];
  self->b = ss_argc == 3 ? ss_argv[2] : ss_undef;
  ss_return(self);
} ss_end

ss_syntax(lambda,1,-1,0,"lambda formals body...") {
  ss rest; int rest_i;
  ss_s_closure *self = ss_alloc(ss_t_closure, sizeof(*self));
  self->formals = ss_argv[0];
  self->params = ss_list_to_vector(ss_argv[0]);
  self->rest = ss_f;
  self->rest_i = -1;
  if ( ss_vector_l(self->params) > 0 ) {
    rest_i = ss_vector_l(self->params) - 1;
    rest = ss_vector_v(self->params)[rest_i];
    if ( ss_type(rest) == ss_t_pair && ss_car(rest) == ss_sym(_rest) ) {
      self->rest_i = rest_i;
      ss_vector_v(self->params)[rest_i] = self->rest = ss_cdr(rest);
    }
  }
  self->body = ss_vecnv(ss_argc - 1, ss_argv + 1);
  ss_return(self);
} ss_end

ss_syntax(let,1,-1,0,"let bindings body...") {
  ss params = ss_nil, *pp = &params;
  ss args = ss_nil, *ap = &args;
  ss body = ss_vecnv(ss_argc - 1, ss_argv + 1);
  ss bindings = ss_argv[0];
  while ( bindings != ss_nil ) {
    ss binding = ss_car(bindings);
    bindings = ss_cdr(bindings);
    *pp = ss_cons(ss_car(binding), ss_nil);
    pp = &ss_CDR(*pp);
    *ap = ss_cons(ss_car(ss_cdr(binding)), ss_nil);
    ap = &ss_CDR(*ap);
  }
  ss_return(ss_cons(ss_cons(ss_sym(lambda), ss_cons(params, body)), args));
} ss_end

ss_prim(cons,2,2,1,"cons car cdr")
  ss_return(ss_cons(ss_argv[0], ss_argv[1]));
ss_end

ss_prim(car,1,1,1,"car pair")
  ss_constantFold = 1;
  ss_typecheck(ss_t_pair,ss_argv[0]);
  ss_return(ss_CAR(ss_argv[0]));
ss_end

ss_prim(cdr,1,1,1,"cdr pair")
  ss_constantFold = 1;
  ss_typecheck(ss_t_pair,ss_argv[0]);
  ss_return(ss_CDR(ss_argv[0]));
ss_end

static
void ss_number_coerce_2(ss *argv)
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

ss_syntax(ADD,0,-1,0,"+ z...")
  switch ( ss_argc ) {
  case 0:
    ss_return(ss_box(integer,0));
  case 1:
    ss_return(ss_argv[0]);
  case 2:
    ss_return(ss_vec(3, ss_sym(_ADD), ss_argv[0], ss_argv[1]));
  default:
    ss_return(ss_vec(3, ss_sym(_ADD), ss_argv[1], ss_cons(ss_sym(ADD), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(SUB,1,-1,0,"- z...")
  switch ( ss_argc ) {
  case 1:
    ss_return(ss_vec(2, ss_sym(_NEG), ss_argv[0]));
  case 2:
    ss_return(ss_vec(3, ss_sym(_SUB), ss_argv[0], ss_argv[1]));
  default:
    ss_return(ss_vec(3, ss_sym(_SUB), ss_argv[1], ss_cons(ss_sym(ADD), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(MUL,0,-1,0,"* z...")
  switch ( ss_argc ) {
  case 0:
    ss_return(ss_box(integer,1));
  case 1:
    ss_return(ss_argv[0]);
  case 2:
    ss_return(ss_vec(3, ss_sym(_MUL), ss_argv[0], ss_argv[1]));
  default:
    ss_return(ss_vec(3, ss_sym(_MUL), ss_argv[1], ss_cons(ss_sym(MUL), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(DIV,1,-1,0,"/ z...")
  switch ( ss_argc ) {
  case 1:
    ss_return(ss_vec(2, ss_sym(_DIV), ss_box(real, 1.0), ss_argv[0]));
  case 2:
    ss_return(ss_vec(3, ss_sym(_DIV), ss_argv[0], ss_argv[1]));
  default:
    ss_return(ss_vec(3, ss_sym(_DIV), ss_argv[1], ss_cons(ss_sym(MUL), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

#define BOP(NAME,OP)                                                    \
  ss_prim(_##NAME,2,2,1,#OP " z...")                                    \
  {                                                                     \
    ss_constantFold = 1;                                                \
    ss_number_coerce_2(ss_argv);                                        \
    switch ( ss_type(ss_argv[0]) ) {                                    \
    case ss_t_integer:                                                  \
      ss_return(ss_box(integer, ss_UNBOX(integer,ss_argv[0]) OP ss_UNBOX(integer,ss_argv[1]))); \
    case ss_t_real:                                                     \
      ss_return(ss_box(real, ss_UNBOX(real,ss_argv[0]) OP ss_UNBOX(real,ss_argv[1]))); \
    default: abort();                                                   \
    }                                                                   \
  }                                                                     \
  ss_end

#define UOP(NAME,OP)                                                    \
  ss_prim(NAME,1,1,1,#OP " z")                                          \
  {                                                                     \
    ss_constantFold = 1;                                                \
    switch ( ss_type(ss_argv[0]) ) {                                    \
    case ss_t_integer:                                                  \
      ss_return(ss_box(integer, OP ss_UNBOX(integer,ss_argv[0])));      \
    case ss_t_real:                                                     \
      ss_return(ss_box(real, OP ss_UNBOX(real,ss_argv[0])));            \
    default: abort();                                                   \
    }                                                                   \
  }                                                                     \
  ss_end

#define ROP(NAME,OP)                                                    \
  ss_prim(NAME,2,2,1,#OP " x y")                                        \
  {                                                                     \
    ss_constantFold = 1;                                                \
    ss_number_coerce_2(ss_argv);                                        \
    switch ( ss_type(ss_argv[0]) ) {                                    \
    case ss_t_integer:                                                  \
      ss_return(ss_BOX(boolean, ss_UNBOX(integer,ss_argv[0]) OP ss_UNBOX(integer,ss_argv[1]))); \
    case ss_t_real:                                                     \
      ss_return(ss_BOX(boolean, ss_UNBOX(real,ss_argv[0])    OP ss_UNBOX(real,ss_argv[1]))); \
    default: abort();                                                   \
    }                                                                   \
  }                                                                     \
  ss_end

#define IBOP(NAME,OP)                                                   \
  ss_prim(NAME,2,2,1,#OP " i j")                                        \
  {                                                                     \
    ss_constantFold = 1;                                                \
    ss_typecheck(ss_t_integer, ss_argv[0]);                             \
    ss_typecheck(ss_t_integer, ss_argv[1]);                             \
    ss_return(ss_box(integer, ss_UNBOX(integer,ss_argv[0]) OP ss_UNBOX(integer,ss_argv[1]))); \
  }                                                                     \
  ss_end

#define IUOP(NAME,OP)                                                   \
  ss_prim(NAME,1,1,1,#OP " i")                                          \
  {                                                                     \
    ss_constantFold = 1;                                                \
    ss_typecheck(ss_t_integer, ss_argv[0]);                             \
    ss_return(ss_box(integer, OP ss_UNBOX(integer,ss_argv[1])));        \
  }                                                                     \
  ss_end

#include "cops.def"

ss _ss_exec(ss_s_environment *ss_env, ss *_ss_expr)
{
  ss rtn, var;
#define return(X) do { rtn = (X); goto _return; } while(0)
  again:
  ss_constantExprQ = 0;
  rtn = ss_expr;
  if ( ss_exec_verbose ) {
    fprintf(*ss_stderr, ";; exec: "); ss_write(ss_expr, ss_stderr); fprintf(*ss_stderr, "\n");
  }
  switch ( ss_type(ss_expr) ) {
  case ss_t_quote:
    ss_constantExprQ = 1;
    return(ss_UNBOX(quote,ss_expr));
  case ss_t_symbol: {
    var = ss_expr;
    rtn = ss_get(&ss_expr, ss_env, var);
    rewrite_const_var:
    if ( (ss_constantExprQ = ss_UNBOX(symbol, var).is_const) )
      ss_rewrite_expr(ss_box_quote(rtn), "variable is constant");
    return(rtn);
  }
  case ss_t_var: {
    var = ss_UNBOX(var, ss_expr).name;
    rtn = ss_get(&ss_expr, ss_env, ss_expr);
    goto rewrite_const_var;
  }
  case ss_t_global: {
    return(ss_UNBOX(global, ss_expr));
  }
  case ss_t_if: {
    ss_s_if *self = ss_expr;
    ss *expr;
    rtn = ss_exec(self->t);
    expr = rtn != ss_f ? &self->a : &self->b;
    if ( ss_constantExprQ ) ss_rewrite_expr(*expr, rtn != ss_f ? "constant test is true" : "constant test is false");
    _ss_expr = expr;
    goto again;
  }
  case ss_t_closure:
    {
      ss_s_closure *self = ss_alloc_copy(ss_t_closure, sizeof(*self), ss_expr);
      self->env = ss_env;
      return(self);
    }
  case ss_t_pair:
    ss_rewrite_expr(ss_list_to_vector(ss_expr), "application pair should be a vector");
    /* FALL THROUGH */
  case ss_t_vector: {
    ss op;
    if ( ss_vector_l(ss_expr) < 1 ) return(ss_error("apply empty-vector", ss_expr));
    op = ss_exec(ss_vector_v(ss_expr)[0]);
    if ( ss_constantExprQ )
      ss_vector_v(ss_expr)[0] = ss_box_quote(op);
    switch ( ss_type(op) ) {
    case ss_t_syntax:
      ss_rewrite_expr((ss_UNBOX(prim,op)->func)(ss_env, &ss_expr, op, ss_vector_l(ss_expr) - 1, ss_vector_v(ss_expr) + 1), "syntax rewrite");
      goto again;
    case ss_t_prim:
      return((ss_UNBOX(prim,op)->func)(ss_env, &ss_expr, op, ss_vector_l(ss_expr) - 1, ss_vector_v(ss_expr) + 1));
    case ss_t_closure:
      {
        ss_s_closure *self = (ss_s_closure*) op;
        size_t i, ss_argc = ss_vector_l(ss_expr) - 1;
        ss *ss_argv = ss_vector_v(ss_expr) + 1;
        ss_s_environment *env;
        if ( self->rest_i >= 0 ) {
          if ( ss_argc < self->rest_i )
            return(ss_error("apply wrong-number-of-arguments given %lu, expected at least %lu", self, (unsigned long) ss_argc, (unsigned long) self->rest_i));
        } else {
          if ( ss_argc != ss_vector_l(self->params) )
            return(ss_error("apply wrong-number-of-arguments given %lu, expected %lu", self, (unsigned long) ss_argc, (unsigned long) ss_vector_l(self->params)));
        }
        env = ss_m_environment(self->env);
        env->argc = ss_argc;
        env->symv = ss_vector_v(self->params);
        env->argv = memcpy(ss_malloc(sizeof(env->argv[0]) * (ss_argc + 1)), ss_argv, sizeof(env->argv[0]) * ss_argc);
        for ( i = 0; i < ss_argc; ++ i )
          env->argv[i] = ss_exec(ss_argv[i]);
        if ( self->rest_i >= 0 )
          env->argv[self->rest_i] = ss_listnv(ss_argc - self->rest_i, env->argv + self->rest_i);
        ss_constantExprQ = 0;
        if ( ss_exec_verbose ) {
          fprintf(*ss_stderr, ";; apply closure "); ss_write(self->params, ss_stderr); ss_write(ss_expr, ss_stderr); fprintf(*ss_stderr, "\n");
        }
        rtn = ss_unspec;
        for ( i = 0; i < ss_vector_l(self->body) - 1; ++ i ) {
          rtn = _ss_exec(env, &ss_vector_v(self->body)[i]);
        }
        if ( i < ss_vector_l(self->body) ) {
          ss_env = env;
          _ss_expr = &ss_vector_v(self->body)[i];
          goto again; // tail recursion.
        }
      }
      break;
    default:
      return(ss_error("apply cannot apply type=%d", op, (int) ss_type(op)));
    }
  }
  default:
    ss_constantExprQ = 1;
  }
#undef return
  _return:
  if ( ss_exec_verbose ) {
    fprintf(*ss_stderr, ";; exec result expr: "); ss_write(ss_expr, ss_stderr); fprintf(*ss_stderr, "\n");
    fprintf(*ss_stderr, ";; exec result  val: "); ss_write(rtn, ss_stderr); fprintf(*ss_stderr, "\n");
  }
  return rtn;
}

ss_prim(apply,2,2,1,"apply func args") {
  ss args = ss_argv[1];
  args = ss_cons(ss_argv[0], args);
  args = ss_list_to_vector(args);
  for ( size_t i = 0; i < ss_vector_l(args); ++ i )
    ss_vector_v(args)[i] = ss_box_quote(ss_vector_v(args)[i]);
  ss_return(ss_exec(args));
} ss_end

ss_prim(ss_call_cfunc,0,5,1,"call cfunc")
{
  switch ( ss_argc ) {
  case 0:
    ss_return(((ss(*)()) ss_prim->c_func)());
  case 1:
    ss_return(((ss(*)(ss)) ss_prim->c_func)(ss_argv[0]));
  case 2:
    ss_return(((ss(*)(ss,ss)) ss_prim->c_func)(ss_argv[0], ss_argv[1]));
  case 3:
    ss_return(((ss(*)(ss,ss,ss)) ss_prim->c_func)(ss_argv[0], ss_argv[1], ss_argv[2]));
  case 4:
    ss_return(((ss(*)(ss,ss,ss,ss)) ss_prim->c_func)(ss_argv[0], ss_argv[1], ss_argv[2], ss_argv[3]));
  case 5:
    ss_return(((ss(*)(ss,ss,ss,ss,ss)) ss_prim->c_func)(ss_argv[0], ss_argv[1], ss_argv[2], ss_argv[3], ss_argv[4]));
  default: abort();
  }
}
ss_end

ss ss_m_cfunc(void *ptr, const char *name, const char *docstr)
{
  ss_s_prim *self = ss_alloc(ss_t_prim, sizeof(*self));
  self->func = _ss_pf_ss_call_cfunc;
  self->minargs = 0; self->maxargs = 5;
  self->evalq = 1;
  self->name = name;
  self->docstring = docstr ? docstr : name;
  self->c_func = ptr;
  return self;
}

ss_prim(ss_symbols,0,0,0,"")
  ss_return(ss_symbols);
ss_end

void ss_init_const(ss_s_environment *ss_env)
{
  ss_undef  = ss_alloc(ss_t_undef, 0);
  ss_unspec = ss_alloc(ss_t_unspec, 0);
  ss_nil    = ss_alloc(ss_t_null, 0);
  ss_t      = ss_alloc(ss_t_boolean, 0);
  ss_f      = ss_alloc(ss_t_boolean, 0);
  ss_eos    = ss_alloc(ss_t_eos, 0);

  ss_symbols = ss_nil;
}

void ss_init_prim(ss_s_environment *ss_env)
{
  ss sym;
#define ss_prim_def(TYPE,NAME,MINARGS,MAXARGS,EVALQ,DOCSTRING) \
  sym = ss_sym(NAME); \
  ss_define(ss_env, sym, ss_alloc_copy(TYPE, sizeof(ss_s_prim), &ss_PASTE2(_ss_p_,NAME))); \
  ss_UNBOX(symbol, sym).is_const = 1;
#include "prim.def"
}
void ss_init_cfunc(ss_s_environment *ss_env);

ss ss_prompt(ss input)
{
  fprintf(*ss_stderr, " ss> ");
  return ss_read(input);
}

void ss_repl(ss_s_environment *ss_env, ss input)
{
  ss expr, value = ss_undef;
  while ( (expr = ss_prompt(input)) != ss_eos ) {
    value = ss_exec(expr);
    fprintf(*ss_stderr, ";; => "); ss_write(expr, ss_stderr); fprintf(*ss_stderr, "\n");
    if ( value != ss_undef ) {
      ss_write(value, ss_stdout); fprintf(*ss_stdout, "\n");
      fprintf(*ss_stderr, ";; %lld (%p)\n", (long long) ss_unbox(integer, value), (void*) value);
      fprintf(*ss_stderr, ";; %llu bytes %llu objects\n",
             (unsigned long long) ss_malloc_bytes,
             (unsigned long long) ss_malloc_objects);

    }
  }
}

ss ss_m_port(FILE *fp, const char *name, const char *mode)
{
  ss_s_port *self = ss_alloc(ss_t_port, sizeof(*self));
  self->fp = fp;
  self->name = ss_s((void*) name);
  self->mode = ss_s((void*) mode);
  return self;
}

void ss_init_port(ss_s_environment *ss_env)
{
#define P(NAME,MODE)                                    \
  ss_##NAME = ss_m_port(NAME, "<" #NAME ">", MODE);     \
  ss_define(ss_env, ss_sym(ss_##NAME), ss_m_global(ss_sym(ss_##NAME), &ss_##NAME));
  P(stdin, "r");
  P(stdout, "w");
  P(stderr, "w");
#undef P
}

int main(int argc, char **argv)
{
  ss_s_environment *ss_env;
  GC_INIT();
  ss_env = ss_m_environment(0);
  ss_init_const(ss_env);
  ss_init_symbol(ss_env);
  ss_init_port(ss_env);
  ss_init_prim(ss_env);
  ss_init_cfunc(ss_env);
  {
    FILE *fp = fopen("boot.scm", "r");
    ss_repl(ss_env, &fp);
    fclose(fp);
  }
  ss_repl(ss_env, ss_stdin);
  return 0;
}

#define VALUE ss
#define READ_DECL ss ss_read(ss stream)
#define READ_CALL() ss_read(stream)
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
#define ERROR(msg,args...) ss_error("read: " msg, stream, ##args)
#define RETURN(X) return X
#if 0
#define MALLOC(S) GC_malloc_atomic(S)
#define REALLOC(P,S) GC_realloc(P,S)
#define FREE(P) GC_free(P)
#endif
#include "lispread/lispread.c"

void ss_init_cfunc(ss_s_environment *ss_env)
{
  ss sym;
#define ss_cfunc_def(TYPE,NAME,ARGS)                                    \
  sym = ss_sym(C_##NAME);                                               \
  ss_define(ss_env, sym, ss_m_cfunc(NAME, #NAME, TYPE " " #NAME ARGS)); \
  ss_UNBOX(symbol, sym).is_const = 1;
#include "cfunc.def"
}
