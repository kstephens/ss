#include "ss.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <errno.h>
#include <string.h> /* memcpy(), strerror() */
#if 1 /* DARWIN */
#define _DONT_USE_CTYPE_INLINE_ 1
#define _ANSI_SOURCE 1
#endif
#include <ctype.h>
#include <unistd.h>
#include <assert.h>

static inline
ss ss_get(ss o, ss i)
{
  return ((ss*) o)[ss_I(i)];
}
static inline
ss ss_set(ss o, ss i, ss v)
{
  return ((ss*) o)[ss_I(i)] = v;
}
static inline
ss ss_get_0(ss x) { return ((ss*)x)[0]; }
static inline
ss ss_get_1(ss x) { return ((ss*)x)[1]; }
static inline
ss ss_get_2(ss x) { return ((ss*)x)[2]; }

ss_s_env *ss_top_level_env, *ss_current_env;
FILE **ss_stdin = &stdin, **ss_stdout = &stdout, **ss_stderr = &stderr;
ss ss_write(ss obj, ss port);
ss ss_write_3(ss v, ss port, ss mode);

size_t ss_malloc_bytes, ss_malloc_objects;
#if 0
#undef ss_malloc
void* ss_malloc(size_t s)
{
  ss_malloc_bytes += s;
  ss_malloc_objects ++;
  return GC_malloc(s);
}
#endif

ss ss_memmove(ss dst, ss src, ss size)
{
  memmove(dst, src, ss_I(size));
  return dst;
}

ss ss_memcmp(ss a, ss b, ss as, ss bs)
{
  int cmp = memcmp(a, b, ss_I(as) < ss_I(bs) ? ss_I(as) : ss_I(bs));
  return ss_i(cmp ? (cmp < 0 ? -1 : 1) : (ss_I(as) == ss_I(bs) ? 0 : (ss_I(as) < ss_I(bs) ? -1 : 1)));
}

ss ss_undef, ss_unspec, ss_nil, ss_t, ss_f, ss_eos;

ss _ss_eval(ss_s_env *ss_env, ss *_ss_expr, ss *ss_argv);
#define ss_expr (*_ss_expr)
#define ss_eval(X) _ss_eval(ss_env, &(X), 0)
#define ss_callv(P,ARGS) _ss_eval(ss_env, &(P), ARGS)
#if 1
int ss_rewrite_verbose = 0;
int ss_eval_verbose = 0;
ss ss_set_eval_verbose(ss x)
{
  ss_eval_verbose = ss_unbox(fixnum, x); return x;
}
ss ss_set_rewrite_verbose(ss x)
{
  ss_rewrite_verbose = ss_unbox(fixnum, x); return x;
}
static inline
void _ss_rewrite_expr(ss *_ss_expr, ss X, const char *REASON, const char *func, int line)
{
  if ( ss_rewrite_verbose || ss_eval_verbose ) {
    fprintf(*ss_stderr, ";; rewrite: ");
    ss_write(ss_expr, ss_stderr);
    fprintf(*ss_stderr, "\n;;      at: #@%p", _ss_expr);
    fprintf(*ss_stderr, "\n;;      in: %s line:%d", func, line);
    fprintf(*ss_stderr, "\n;;  reason: %s\n", (REASON));
  }
  ss_expr = X;
  if ( ss_rewrite_verbose || ss_eval_verbose ) {
    fprintf(*ss_stderr, ";;      as: ");
    ss_write(ss_expr, ss_stderr);
    fprintf(*ss_stderr, "\n\n");
  }
}
#define ss_rewrite_expr(X,REASON) _ss_rewrite_expr(&ss_expr, (X), REASON, __FUNCTION__, __LINE__)
#else
#define ss_rewrite_verbose 0
#define ss_eval_verbose    0
#define ss_rewrite_expr(X,REASON) (ss_expr = (X))
#endif

ss ss_set_type(ss_e_type type, ss obj)
{
  ((ss*) obj)[-1] = (ss) type;
  return obj;
}

ss ss_alloc(ss_e_type type, size_t size)
{
  void *ptr = ss_malloc(sizeof(ss) + size);
  // *((ss*) ptr) = 0;
  // ptr += sizeof(ss);
  *((ss*) ptr) = (ss) type;
  ptr += sizeof(ss);
  return ptr;
}

ss ss_alloc_copy(ss_e_type type, size_t size, void *ptr)
{
  void *self = ss_alloc(type, size);
  memcpy(self, ptr, size);
  return self;
}

void ss_error_init(ss_s_env *ss_env, jmp_buf *jb)
{
  ss_env->error_jmp = jb;
  ss_env->error_val = ss_undef;
}

ss ss_error_raise(ss_s_env *ss_env, ss val)
{
  ss_s_env *e = ss_env;
  if ( ! e ) e = ss_current_env;
  while ( e && ! e->error_jmp ) e = e->parent;
  if ( ! e ) {
    fprintf(*ss_stderr, "ss: no error catch: aborting\n");
    abort();
  }
  e->error_val = val;
  {
    jmp_buf *tmp = e->error_jmp;
    e->error_jmp = 0;
    longjmp(*tmp, 1);
  }
}

#define FP(port) (*(FILE**) (port))
ss ss_error(ss_s_env *ss_env, const char *format, ss obj, ...)
{
  va_list vap;
  va_start(vap, obj);
  fprintf(FP(ss_stderr), "\n  ss: error ");
  vfprintf(FP(ss_stderr), format, vap);
  fprintf(FP(ss_stderr), ": ");
  ss_write(obj, ss_stderr);
  fprintf(FP(ss_stderr), "\n");
  va_end(vap);
  for ( ss_s_env *env = ss_env; env; env = env->parent ) {
    fprintf(FP(ss_stderr), "  ss: %3d ", (int) env->depth);
    ss_write(env->expr, ss_stderr);
    fprintf(FP(ss_stderr), "\n");
  }
  ss_error_raise(ss_env, ss_undef);
  return 0;
}

static inline
ss ss_typecheck_error(ss v)
{
  return ss_error(ss_current_env, "typecheck", v);
}

static inline
ss ss_typecheck(ss_e_type t, ss v)
{
  if ( ss_type(v) != t )
    return ss_typecheck_error(v);
  return v;
}

static inline
ss ss_cons(ss a, ss d)
{
  ss_s_cons *self = ss_alloc(ss_t_pair, sizeof(*self));
  self->a = a;
  self->d = d;
  return self;
}

static inline
ss ss_car(ss a)
{
  ss_typecheck(ss_t_pair,a);
  return ss_CAR(a);
}

static inline
ss ss_cdr(ss a)
{
  ss_typecheck(ss_t_pair,a);
  return ss_CDR(a);
}

static inline
ss ss_set_carE(ss a, ss v)
{
  ss_typecheck(ss_t_pair,a);
  ss_CAR(a) = v;
  return a;
}

static inline
ss ss_set_cdrE(ss a, ss v)
{
  ss_typecheck(ss_t_pair,a);
  ss_CDR(a) = v;
  return a;
}

void ss_minimal_double_str(double x, char *buf, size_t buflen)
{
  char format[8];
  int len = 1;
  double y;
  do {
    snprintf(format, 8, "%%.%dg", len);
    snprintf(buf, buflen, format, (double) x);
    y = strtod(buf, 0);
    len ++;
  } while ( y != x && buf[buflen - 1] == 0 );
}

void ss_write_flonum(ss v, ss port)
{
  char buf[64];
  ss_minimal_double_str(ss_unbox(flonum, v), buf, 63);
  if ( ! (strchr(buf, 'e') || strchr(buf, '.')) ) {
    strcat(buf, ".0");
  }
  fprintf(FP(port), "%s", buf);
}

void ss_write_vec(size_t n, const ss *v, ss port)
{
  size_t i = 0;
  while ( i < n ) {
    ss_write(v[i], port);
    if ( ++ i < n )
      fprintf(FP(port), " ");
  }
}

ss ss_write(ss v, ss port)
{
  return ss_write_3(v, port, ss_sym(write));
}

ss ss_write_3(ss v, ss port, ss mode)
{
#define ss_write(v,p) ss_write_3(v, p, mode)
  FILE *out = FP(port);
  switch ( ss_type(v) ) {
  case ss_t_undef:   fprintf(out, "#<undef>"); break;
  case ss_t_fixnum:  fprintf(out, "%lld", (long long) ss_UNBOX(fixnum, v)); break;
  case ss_t_flonum:  ss_write_flonum(v, port); break;
  case ss_t_string:
    if ( mode == ss_sym(display) ) {
      fwrite(ss_string_V(v), ss_string_L(v), 1, out);
    } else {
      fprintf(out, "\"%s\"", ss_string_V(v));
    }
    break;
  case ss_t_char:
    if ( mode == ss_sym(display) ) {
      fprintf(out, "%c",  ss_C(v));
    } else {
      fprintf(out, "#\\%c",  ss_C(v));
    }
    break;
  case ss_t_boolean: fprintf(out, "#%c",    v == ss_t ? 't' : 'f'); break;
  case ss_t_prim:    fprintf(out, "#<p %s>", ss_UNBOX(prim, v)->name); break;
  case ss_t_symbol:
    if ( ss_UNBOX(symbol, v).name == ss_f ) {
      fprintf(out, "#<symbol #@%p>", v);
    } else {
      fprintf(out, "%s",   ss_string_V(ss_UNBOX(symbol, v).name));
    }
    break;
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
  case ss_t_var_set:
    fprintf(out, "(set! ");
    ss_write(ss_UNBOX(var_set, v).var, port);
    fprintf(out, " ");
    ss_write(ss_UNBOX(var_set, v).expr, port);
    fprintf(out, ")");
    break;
  case ss_t_global:
    fprintf(out, "#<g ");
    ss_write(((ss_s_global*) v)->name, port);
    fprintf(out, " >");
    break;
  case ss_t_quote:   fprintf(out, "'"); ss_write(ss_UNBOX(quote, v), port); break;
  case ss_t_eos:     fprintf(out, "#<eos>"); break;
  case ss_t_null:    fprintf(out, "()"); break;
  case ss_t_lambda:
    fprintf(out, "(lambda ");
    ss_write(ss_UNBOX(lambda, v).formals, port);
    fprintf(out, " ");
    ss_write(ss_UNBOX(lambda, v).body, port);
    fprintf(out, ")");
    break;
  case ss_t_closure:
    // fprintf(out, "#<c #@%p E#@%p ", v, ss_UNBOX(closure, v).env);
    ss_write(ss_UNBOX(closure, v).lambda, port);
    // fprintf(out, ">");
    break;
  case ss_t_port:
    fprintf(out, "#<port ");
    ss_write(ss_UNBOX(port, v).name, port);
    fprintf(out, " ");
    ss_write(ss_UNBOX(port, v).mode, port);
    fprintf(out, ">");
    break;
  case ss_t_env:
    fprintf(out, "#<env #@%p %ld >", v, (long) ((ss_s_env*) v)->level);
    break;
  default:
    fprintf(out, "#<??? %d #@%p >", ss_type(v), (void*) v);
    break;
  case ss_t_pair:
    fprintf(out, "(");
    while ( v != ss_nil ) {
      switch ( ss_type(v) ) {
      case ss_t_pair:
        ss_write(ss_CAR(v), port);
        v = ss_CDR(v);
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
  case ss_t_begin:
    fprintf(out, "(begin ");
    goto vector_body;
  case ss_t_app:
    fprintf(out, "(");
    goto vector_body;
  case ss_t_vector:
    fprintf(out, "#(");
  vector_body:
    ss_write_vec(ss_vector_L(v), ss_vector_V(v), port);
    fprintf(out, ")");
    break;
  }
#undef ss_write
  return ss_undef;
}

#undef ss_sym_def
#define ss_sym_def(X) ss ss_PASTE2(_ss_sym_,X);
#include "sym.def"

ss ss_box_fixnum(ss_fixnum_t v)
{
  return ss_BOX_fixnum(v);
}

ss_fixnum_t ss_unbox_fixnum(ss v)
{
  ss_typecheck(ss_t_fixnum,v);
  return ss_UNBOX_fixnum(v);
}

ss ss_box_flonum(ss_flonum_t v)
{
  ss_s_flonum *self = ss_alloc(ss_t_flonum, sizeof(*self));
  self->value = v;
  return self;
}

ss_flonum_t ss_unbox_flonum(ss v)
{
  ss_typecheck(ss_t_flonum, v);
  return ss_UNBOX_flonum(v);
}

ss ss_r(ss v)
{
  ss x[2] = { 0, 0 };
  x[0] = v;
  return ss_box_flonum(*(ss_flonum_t*) &x);
}

ss ss_R(ss v)
{
  ss rtn[2] = { 0, 0 };
  *((ss_flonum_t*) &rtn) = ss_unbox_flonum(v);
  return rtn[0];
}

ss ss_c(int _v)
{
  return ss_BOX_char(_v);
}
int ss_C(ss v)
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
  if ( v )
    memcpy(ss_string_V(self), v, sizeof(ss_string_V(self)[0]) * l);
  ss_string_V(self)[l] = 0;
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
  return ss_string_V(p);
}

ss ss_string_TO_number(ss s, int radix)
{
  char *endp;
  double d;
  long long ll;

  ll = strtoll(ss_string_V(s), &endp, radix);
  if ( ! *endp )
    return ss_box(fixnum, ll);
  d = strtod(ss_string_V(s), &endp);
  if ( ! *endp )
    return ss_box(flonum, d);
  return ss_f;
}

ss ss_errno() { return ss_BOX_fixnum(errno); }
ss ss_errstr(ss en)
{
  int i;
  if ( en == ss_f ) en = ss_errno();
  i = ss_UNBOX_fixnum(en);
  return ss_s(strerror(i));
}

static ss symbols;
ss ss_box_symbol(const char *name)
{
  ss_s_symbol *sym;

  if ( name ) {
    for ( ss l = symbols; l != ss_nil; l = ss_cdr(l) ) {
      sym = (ss_s_symbol*) ss_car(l);
      if ( strcmp(name, ss_string_V(sym->name)) == 0 )
        goto rtn;
    }
  }

  sym = ss_alloc(ss_t_symbol, sizeof(*sym));
  sym->name = name ? ss_strnv(strlen(name), name) : ss_f;
  sym->docstring = ss_f;
  sym->syntax = ss_f;
  sym->is_const = 0;
  if ( name )
    symbols = ss_cons(sym, symbols);

 rtn:
  // fprintf(stderr, "  symbol(%s) => %p\n", name, sym);
  return sym;
}

ss ss_symbols() { return symbols; }

void ss_init_symbol(ss_s_env *ss_env)
{
#undef ss_sym_def
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
  ss_sym(_env) = ss_box_symbol("&env");
  ss_sym(unquote_splicing) = ss_box_symbol("unquote-splicing");
}

ss ss_box_quote(ss v)
{
  if ( ss_literalQ(v) && ss_type(v) != ss_t_quote ) {
    return(v);
  } else {
    ss_s_quote *self = ss_alloc(ss_t_quote, sizeof(*self));
    self->value = v;
    return self;
  }
}

ss ss_listn(int n, ...)
{
  va_list vap;
  va_start(vap, n);
  ss l = ss_nil, *lp = &l;
  while ( n -- ) {
    *lp = ss_cons(va_arg(vap, ss), ss_nil);
    lp = &ss_CDR(*lp);
  }
  va_end(vap);
  return l;
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
  case ss_t_null:    return l;
  case ss_t_vector:  return l + ss_vector_L(x);
  default:           return l + 1;
  }
}

ss ss_list_to_vector(ss x)
{
  size_t l = 0;
  ss v = ss_vecn(ss_list_length(x));
  again:
  switch ( ss_type(x) ) {
  case ss_t_pair:
    ss_vector_V(v)[l ++] = ss_CAR(x);
    x = ss_CDR(x);
    goto again;
  case ss_t_null:
    break;
  case ss_t_vector:
    memcpy(ss_vector_V(v) + l, ss_vector_V(x), sizeof(ss_vector_V(v)[0]) * ss_vector_L(x));
    break;
  default:
    ss_vector_V(v)[l] = ss_cons(ss_sym(_rest), x);
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
    ss_vector_V(x)[i ++] = va_arg(vap, ss);
  va_end(vap);
  return x;
}

ss ss_m_env(ss_s_env *parent)
{
  ss_s_env *env = ss_alloc(ss_t_env, sizeof(*env));
  env->argc = 0;
  env->symv = env->argv = 0;
  env->parent = parent;
  env->top_level = parent ? parent->top_level : env;
  env->depth     = parent ? parent->depth     : 0;
  env->level     = parent ? parent->level + 1 : 0;
  env->constantExprQ = env->constantExprQAll = 0;
  env->expr      = ss_undef;
  env->error_jmp = 0; env->error_val = ss_undef;
  // fprintf(stderr, "  ss_m_env(#@%p) => #<c E#@%p -> E#@%p>\n", parent, env, env->parent);
  return env;
}

#if 1
ss ss_env_symv(ss _o)
{
  ss_s_env *o = _o;
  return ss_vecnv(o->argc, o->symv);
}

ss ss_closure_env(ss _o)
{
  ss_s_closure *o = _o;
  return o->env;
}
#endif

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

ss ss_define(ss_s_env *env, ss sym, ss val)
{
  int i;
  for ( i = 0; i < env->argc; ++ i ) {
    if ( sym == env->symv[i] ) {
      env->argv[i] = val;
      return sym;
    }
  }
  env->symv = memcpy(ss_malloc(sizeof(env->symv) * (env->argc + 1)), env->symv, sizeof(env->symv[0]) * env->argc);
  env->symv[env->argc] = sym;
  env->argv = memcpy(ss_malloc(sizeof(env->argv) * (env->argc + 1)), env->argv, sizeof(env->argv[0]) * env->argc);
  env->argv[env->argc] = val;
  ++ env->argc;

  // ss_write(sym, ss_stderr); fprintf(*ss_stderr, " = "); ss_write(val, ss_stderr); fprintf(*ss_stderr, " #@%d\n", (int) env->argc);
 
  return sym;
}

ss* ss_bind(ss_s_env *ss_env, ss *_ss_expr, ss var, int set)
{
  ss_s_env *env = ss_env;
  int up, over;
  ss sym, *ref;
  ss_constantExprQ = 0;
  switch ( ss_type(var) ) {
  case ss_t_symbol:
    sym = var;
    up = 0;
    while ( env ) {
      for ( over = 0; over < env->argc; ++ over ) {
        if ( var == env->symv[over] ) {
          ss_rewrite_expr(ss_m_var(var, up, over), "var binding is known");
          goto rtn;
        }
      }
      ++ up;
      env = env->parent;
    }
    break;
  case ss_t_var:
    sym  = ss_UNBOX(var, var).name;
    up   = ss_UNBOX(var, var).up;
    over = ss_UNBOX(var, var).over;
    while ( up -- > 0 ) env = env->parent;
    assert(env);
#if 0
    // Does not work because env->argv can grow.
    if ( ! env->parent ) {
      ref = &env->argv[over];
      ss_rewrite_expr(ss_m_global(sym, ref), "var is global");
      goto ref;
    }
#endif
    goto rtn;
  default: break;
  }
  return(ss_error(ss_env, "unbound", var));

 rtn:
  ref = &env->argv[over];
 ref:
  if ( ss_type(*ref) == ss_t_global ) {
    sym = ((ss_s_global*) *ref)->name;
    ss_rewrite_expr(*ref, "global binding is known");
    ref = &ss_UNBOX(global, *ref);
  }
  if ( ss_UNBOX(symbol, sym).is_const && env->parent == 0 ) {
    if ( set ) return(ss_error(ss_env, "constant-variable", sym));
    ss_constantExprQ = 1;
    ss_rewrite_expr(ss_box_quote(*ref), "variable constant in top-level");
  }

  return ref;
}

ss ss_var_set(void *env, ss *_ss_expr, ss var, ss val)
{
  *ss_bind(env, _ss_expr, var, 1) = val;
  return var;
}

ss ss_var_get(void *env, ss *_ss_expr, ss var)
{
  return *ss_bind(env, _ss_expr, var, 0);
}

void _ss_min_args_error(ss_s_env *ss_env, ss op, const char *DOCSTRING, int ss_argc, int MINARGS)
{
  ss_error(ss_env, "apply not-enough-args (%s) got %d expected %d", op, DOCSTRING, ss_argc, MINARGS);
}

void _ss_max_args_error(ss_s_env *ss_env, ss op, const char *DOCSTRING, int ss_argc, int MAXARGS)
{
  ss_error(ss_env, "apply too-many-args (%s) got %d expected %d", op, DOCSTRING, ss_argc, MAXARGS);
}

ss ss_make_constant(ss sym)
{
  ss_typecheck(ss_t_symbol, sym);
  ss_UNBOX(symbol, sym).is_const = 1;
  return sym;
}
ss ss_constantQ(ss sym)
{
  ss_typecheck(ss_t_symbol, sym);
  return ss_box(boolean, ss_UNBOX(symbol, sym).is_const);
}
ss ss_make_syntax(ss sym, ss proc)
{
  ss_typecheck(ss_t_symbol, sym);
  ss_UNBOX(symbol, sym).syntax = proc;
  return sym;
}

ss_syntax(define,1,-1,0,"define name value") {
  ss name = ss_argv[0];
  if ( ss_type(name) == ss_t_pair ) {
    ss_return(ss_cons(ss_sym(define), ss_cons(ss_car(name), ss_cons(ss_cons(ss_sym(lambda), ss_cons(ss_cdr(name), ss_listnv(ss_argc - 1, ss_argv + 1))), ss_nil))));
  } else {
    ss_return(ss_cons(ss_sym(_define), ss_cons(ss_box_quote(name), ss_cons(ss_argv[1], ss_nil))));
  }
} ss_end

ss_prim(_define,2,2,0,"define name value") {
  ss_return(ss_define(ss_env->top_level, ss_argv[0], ss_argv[1]));
} ss_end

ss_syntax(setE,2,2,0,"set! name value") {
  ss_s_var_set *self = ss_alloc(ss_t_var_set, sizeof(*self));
  self->var = ss_argv[0];
  self->expr = ss_argv[1];
  ss_return(self);
} ss_end

ss_prim(ss_getc,1,1,0,"port")
{
  int c = getc(FP(ss_argv[0]));
  ss_return(ss_i(c));
}
ss_end

ss_prim(ss_ungetc,2,2,0,"port c")
{
  int c = ss_I(ss_argv[1]);
  ungetc(c, FP(ss_argv[0]));
}
ss_end

ss ss_read(ss_s_env *ss_env, ss port);

ss_prim(newline,0,1,0,"newline")
  FILE **out = ss_argc > 0 ? ss_argv[0] : ss_stdout;
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
  ss_s_lambda *self = ss_alloc(ss_t_lambda, sizeof(*self));
  self->formals = ss_argv[0];
  self->params = ss_list_to_vector(ss_argv[0]);
  self->rest = ss_f;
  self->rest_i = -1;
  if ( ss_vector_L(self->params) > 0 ) {
    rest_i = ss_vector_L(self->params) - 1;
    rest = ss_vector_V(self->params)[rest_i];
    if ( ss_type(rest) == ss_t_pair && ss_CAR(rest) == ss_sym(_rest) ) {
      self->rest_i = rest_i;
      ss_vector_V(self->params)[rest_i] = self->rest = ss_CDR(rest);
    }
  }
  self->body = ss_cons(ss_sym(begin), ss_listnv(ss_argc - 1, ss_argv + 1));
  ss_return(self);
} ss_end

ss_syntax(let,1,-1,0,"let bindings body...") {
  ss params = ss_nil, *pp = &params;
  ss args = ss_nil, *ap = &args;
  ss body = ss_listnv(ss_argc - 1, ss_argv + 1);
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

ss_syntax(begin,0,-1,0,"begin body...") {
  switch ( ss_argc ) {
  case 0:
    ss_return(ss_undef);
  case 1:
    ss_return(ss_argv[0]);
  default:
    ss_return(ss_set_type(ss_t_begin, ss_vecnv(ss_argc, ss_argv)));
  }
} ss_end

static
ss ss_to_flonum(ss x)
{
  switch ( ss_type(x)) {
  case ss_t_flonum:  return x;
  case ss_t_fixnum:  return ss_box(flonum, ss_I(x));
  default:           return ss_typecheck_error(x);
  }
}

static
void ss_number_coerce_2(ss *argv)
{
  switch ( ss_type(argv[0]) ) {
  case ss_t_fixnum:
    switch ( ss_type(argv[1]) ) {
    case ss_t_flonum:
      argv[0] = ss_box(flonum, ss_UNBOX(fixnum, argv[0]));
      break;
    case ss_t_fixnum:
      break;
    default: ss_typecheck_error(argv[1]);
    }
    break;
  case ss_t_flonum:
    switch ( ss_type(argv[1]) ) {
    case ss_t_fixnum:
      argv[1] = ss_box(flonum, ss_UNBOX(fixnum, argv[1]));
      break;
    case ss_t_flonum: break;
    default: ss_typecheck_error(argv[1]);
    }
    break;
  default: ss_typecheck_error(argv[1]);
  }
}

ss_syntax(ADD,0,-1,1,"+ z...")
  switch ( ss_argc ) {
  case 0:  ss_return(ss_box(fixnum,0));
  case 1:  ss_return(ss_argv[0]);
  case 2:  ss_return(ss_vec(3, ss_sym(_ADD), ss_argv[0], ss_argv[1]));
  default: ss_return(ss_vec(3, ss_sym(_ADD), ss_argv[0],
                            ss_cons(ss_sym(ADD), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(SUB,1,-1,1,"- z...")
  switch ( ss_argc ) {
  case 1:  ss_return(ss_vec(2, ss_sym(_NEG), ss_argv[0]));
  case 2:  ss_return(ss_vec(3, ss_sym(_SUB), ss_argv[0], ss_argv[1]));
  default: ss_return(ss_vec(3, ss_sym(_SUB), ss_argv[0],
                            ss_cons(ss_sym(ADD), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(MUL,0,-1,1,"* z...")
  switch ( ss_argc ) {
  case 0:  ss_return(ss_box(fixnum,1));
  case 1:  ss_return(ss_argv[0]);
  case 2:  ss_return(ss_vec(3, ss_sym(_MUL), ss_argv[0], ss_argv[1]));
  default: ss_return(ss_vec(3, ss_sym(_MUL), ss_argv[0],
                            ss_cons(ss_sym(MUL), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(DIV,1,-1,1,"/ z...")
  switch ( ss_argc ) {
  case 1:  ss_return(ss_vec(3, ss_sym(_DIV), ss_box(flonum, 1.0), ss_argv[0]));
  case 2:  ss_return(ss_vec(3, ss_sym(_DIV), ss_argv[0], ss_argv[1]));
  default: ss_return(ss_vec(3, ss_sym(_DIV), ss_argv[0], ss_cons(ss_sym(MUL), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

#define BOP(NAME,OP)                                                    \
  ss_prim(_##NAME,2,2,1,#OP " z...")                                    \
  {                                                                     \
    ss_number_coerce_2(ss_argv);                                        \
    switch ( ss_type(ss_argv[0]) ) {                                    \
    case ss_t_fixnum:                                                   \
      ss_return(ss_box(fixnum, ss_UNBOX(fixnum,ss_argv[0]) OP ss_UNBOX(fixnum,ss_argv[1]))); \
    case ss_t_flonum:                                                   \
      ss_return(ss_box(flonum, ss_UNBOX(flonum,ss_argv[0]) OP ss_UNBOX(flonum,ss_argv[1]))); \
    default: ss_typecheck_error(ss_argv[0]);                            \
    }                                                                   \
  }                                                                     \
  ss_end

#define UOP(NAME,OP)                                                    \
  ss_prim(_##NAME,1,1,1,#OP " z")                                       \
  {                                                                     \
    switch ( ss_type(ss_argv[0]) ) {                                    \
    case ss_t_fixnum:                                                   \
      ss_return(ss_box(fixnum, OP ss_UNBOX(fixnum,ss_argv[0])));        \
    case ss_t_flonum:                                                   \
      ss_return(ss_box(flonum, OP ss_UNBOX(flonum,ss_argv[0])));        \
    default: ss_typecheck_error(ss_argv[0]);                            \
    }                                                                   \
  }                                                                     \
  ss_end

#define ROP(NAME,OP)                                                    \
  ss_prim(_##NAME,2,2,1,#OP " x y")                                     \
  {                                                                     \
    ss_number_coerce_2(ss_argv);                                        \
    switch ( ss_type(ss_argv[0]) ) {                                    \
    case ss_t_fixnum:                                                   \
      ss_return(ss_box(boolean, ss_UNBOX(fixnum,ss_argv[0]) OP ss_UNBOX(fixnum,ss_argv[1]))); \
    case ss_t_flonum:                                                   \
      ss_return(ss_box(boolean, ss_UNBOX(flonum,ss_argv[0]) OP ss_UNBOX(flonum,ss_argv[1]))); \
    default: ss_typecheck_error(ss_argv[0]);                            \
    }                                                                   \
  }                                                                     \
  ss_end

#define IBOP(NAME,OP)                                                   \
  ss_prim(_##NAME,2,2,1,#OP " i j")                                     \
  {                                                                     \
    ss_typecheck(ss_t_fixnum, ss_argv[0]);                              \
    ss_typecheck(ss_t_fixnum, ss_argv[1]);                              \
    ss_return(ss_box(fixnum, ss_UNBOX(fixnum,ss_argv[0]) OP ss_UNBOX(fixnum,ss_argv[1]))); \
  }                                                                     \
  ss_end

#define IUOP(NAME,OP)                                                   \
  ss_prim(_##NAME,1,1,1,#OP " i")                                       \
  {                                                                     \
    ss_typecheck(ss_t_fixnum, ss_argv[0]);                              \
    ss_return(ss_box(fixnum, OP ss_UNBOX(fixnum,ss_argv[1])));          \
  }                                                                     \
  ss_end

#include "cops.def"

ss ss_apply(ss_s_env *ss_env, ss func, ss args)
{
  if ( ss_type(args) != ss_t_vector ) 
    args = ss_list_to_vector(args);
  return _ss_eval(ss_env, &func, args);
}

int ss_sort_cmp(void *thunk, const void *a, const void *b)
{
  ss ss_env = ((ss*)thunk)[0];
  ss func   = ((ss*)thunk)[1];
  return ss_apply(ss_env, func, ss_vec(2, *(ss*)a, *(ss*)b)) == ss_f ? 1 : -1;
}

ss ss_sort(ss_s_env *ss_env, ss v, ss cmp)
{
  ss thunk[] = { ss_env, cmp };
  qsort_r(ss_vector_V(v), ss_vector_L(v), sizeof(ss), thunk, ss_sort_cmp);
  return v;
}

ss_prim(apply,2,2,0,"apply func args") {
  ss_return(ss_apply(ss_env, ss_argv[0], ss_argv[1]));
} ss_end

ss_prim(eval,1,2,0,"eval expr env?") {
  ss_return(_ss_eval(ss_argc > 1 ? ss_argv[1] : ss_top_level_env, &ss_argv[0], 0));
} ss_end

ss _ss_eval(ss_s_env *ss_env, ss *_ss_expr, ss *ss_argv)
{
  ss rtn, expr;
  int const_argsQ;
  size_t ss_argc;
#define return(X) do { rtn = (X); goto _return; } while(0)
  expr = ss_expr;
  ++ ss_env->depth;
  if ( ss_argv ) {
    if ( ss_eval_verbose ) {
      fprintf(*ss_stderr, "  ;; eval %3d E#@%p #@%p (", (int) ss_env->depth, ss_env, _ss_expr); ss_write(expr, ss_stderr); fprintf(*ss_stderr, " . "); ss_write(ss_argv, ss_stderr); fprintf(*ss_stderr, ") (apply)\n");
    }
    rtn = expr;
    ss_argc = ss_vector_L(ss_argv);
    ss_argv = ss_vector_V(ss_argv);
    const_argsQ = 0;
    ss_current_env = ss_env;
    goto call;
  }
  again:
  ss_current_env = ss_env;
#define ss_eval_tail(X) do {     \
    expr = *(_ss_expr = &(X));   \
    goto again;                  \
  } while(0)
  ss_constantExprQ = 0;
  if ( ss_eval_verbose ) {
    fprintf(*ss_stderr, "  ;; eval %3d E#@%p #@%p ", (int) ss_env->depth, ss_env, _ss_expr); ss_write(expr, ss_stderr); fprintf(*ss_stderr, "\n");
  }
  switch ( ss_type(expr) ) {
  case ss_t_quote:
    ss_constantExprQ = 1;
    return(ss_UNBOX(quote, expr));
  case ss_t_symbol:
    if ( expr == ss_sym(_env) ) return(ss_env);
  case ss_t_var:
    return(ss_var_get(ss_env, _ss_expr, expr));
  case ss_t_var_set:
    {
      ss_s_var_set *self = expr;
      rtn = ss_eval(self->expr);
      ss_var_set(ss_env, &self->var, self->var, rtn);
      return(ss_undef);
    }
  case ss_t_global:
    return(ss_UNBOX(global, expr));
  case ss_t_if:
    {
      ss_s_if *self = ss_expr;
      ss *subexpr;
      rtn = ss_eval(self->t);
      subexpr = rtn != ss_f ? &self->a : &self->b;
      if ( 0 && ss_constantExprQ )
        ss_rewrite_expr(*subexpr, rtn != ss_f ? "constant test is true" : "constant test is false");
      else
        _ss_expr = subexpr;
      expr = *subexpr;
      goto again;
    }
  case ss_t_begin:
    {
      size_t i;
      for ( i = 0; i < ss_vector_L(expr) - 1; ++ i )
        ss_eval(ss_vector_V(expr)[i]);
      ss_eval_tail(ss_vector_V(expr)[i]);
    }
  case ss_t_lambda:
    {
      ss_s_closure *self = ss_alloc(ss_t_closure, sizeof(*self));
      self->lambda = expr;
      self->env = ss_env;
      // fprintf(stderr, "  #@%p => #<c #@%p E#@%p>\n", expr, self, self->env);
      return(self);
    }
  case ss_t_pair:
    rtn = ss_car(expr);
    if ( ss_type(rtn) == ss_t_symbol && (rtn = ss_UNBOX(symbol, rtn).syntax) != ss_f ) {
      expr = ss_apply(ss_env, rtn, ss_cdr(expr));
      ss_rewrite_expr(expr, "syntax rewrite");
      goto again;
    }
    expr = ss_list_to_vector(expr);
    /* FALL THROUGH */
  case ss_t_vector: /* FIXME: Some syntaxes expand into vectors. */
    ss_set_type(ss_t_app, expr);
    ss_rewrite_expr(expr, "application vector");
    /* FALL THROUGH */
  case ss_t_app: {
    if ( ss_vector_L(expr) < 1 ) return(ss_error(ss_env, "apply empty-vector", expr));
    ss_argc = ss_vector_L(expr) - 1;

    rtn = ss_eval(ss_vector_V(expr)[0]);

    const_argsQ = 1;
    ss_argv = ss_malloc(sizeof(ss_argv[0]) * (ss_argc + 1)); // +1 restarg.
    for ( size_t i = 0; i < ss_argc; i ++ ) {
      ss_argv[i] = ss_eval(ss_vector_V(expr)[i + 1]);
      const_argsQ &= ss_constantExprQ;
    }

    call:
    ss_constantExprQ = 0;

    switch ( ss_type(rtn) ) {
    case ss_t_prim:
      {
        expr = (ss_UNBOX(prim, rtn)->func)(ss_env, _ss_expr, rtn, ss_argc, ss_argv);
        if ( ss_eval_verbose ) {
          if ( const_argsQ ) fprintf(*ss_stderr, "    ;; const_argsQ %s\n", ss_UNBOX(prim, rtn)->no_side_effect ? "no-side-effect" : "");
        }
        if ( (ss_constantExprQ = const_argsQ && ss_UNBOX(prim, rtn)->no_side_effect) )
          ss_rewrite_expr(ss_box_quote(expr), "constant folding");
        return(expr);
      }

    case ss_t_closure:
      {
        ss_s_lambda *self = ((ss_s_closure*) rtn)->lambda;
        ss_s_env *env;

        if ( self->rest_i >= 0 ) {
          if ( ss_argc < self->rest_i )
            return(ss_error(ss_env, "apply wrong-number-of-arguments given %lu, expected at least %lu", self, (unsigned long) ss_argc, (unsigned long) self->rest_i));
        } else {
          if ( ss_argc != ss_vector_L(self->params) )
            return(ss_error(ss_env, "apply wrong-number-of-arguments given %lu, expected %lu", self, (unsigned long) ss_argc, (unsigned long) ss_vector_L(self->params)));
        }

        env = ss_m_env(((ss_s_closure*) rtn)->env);
        env->expr = ss_expr;
        env->argc = ss_argc;
        env->symv = ss_vector_V(self->params);
        env->argv = ss_argv;
        if ( self->rest_i >= 0 ) {
          env->argv[self->rest_i] = ss_listnv(ss_argc - self->rest_i, env->argv + self->rest_i);
          env->argc = self->rest_i + 1;
        }
        if ( ss_eval_verbose ) {
          fprintf(*ss_stderr, "    ;; apply closure:\n");
          fprintf(*ss_stderr, "    ;;   args: (");
          ss_write_vec(env->argc, env->argv, ss_stderr);
          fprintf(*ss_stderr, ")\n    ;;     to: ");
          ss_write(self, ss_stderr);
          fprintf(*ss_stderr, "\n");
        }
        if ( ss_eval_verbose ) {
          size_t i;
          fprintf(*ss_stderr, "    ;; binding:\n");
          for ( i = 0; i < env->argc; ++ i ) {
            fprintf(*ss_stderr, "    ;;   ");
            ss_write(env->symv[i], ss_stderr);
            fprintf(*ss_stderr, " => ");
            ss_write(env->argv[i], ss_stderr);
            fprintf(*ss_stderr, "\n");
          }
        }

        ss_env = env;
        ss_eval_tail(self->body);
      }
      break;
    default:
      return(ss_error(ss_env, "apply cannot apply type=%d", rtn, (int) ss_type(rtn)));
    }
  }
  default:
    rtn = expr;
    ss_constantExprQ = ss_literalQ(expr);
  }
#undef return
#undef ss_eval_tail
  _return:
  if ( ss_eval_verbose ) {
    fprintf(*ss_stderr, "  ;; eval %3d result expr: ", (int) ss_env->depth); ss_write(ss_expr, ss_stderr); fprintf(*ss_stderr, "\n");
    fprintf(*ss_stderr, "  ;; eval %3d result  val: ", (int) ss_env->depth);
    ss_write(rtn, ss_stderr);
    fprintf(*ss_stderr, " (%s) ", ss_constantExprQ ? "const" : "non-const");
    fprintf(*ss_stderr, "\n");
  }
  -- ss_env->depth;
  return rtn;
}

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
  self->min_args = 0; self->max_args = 5;
  self->no_side_effect = 0;
  self->name = name;
  self->docstring = docstr ? docstr : name;
  self->c_func = ptr;
  return self;
}

void ss_init_const(ss_s_env *ss_env)
{
  ss_undef  = ss_alloc(ss_t_undef, 0);
  ss_unspec = ss_alloc(ss_t_unspec, 0);
  ss_nil    = ss_alloc(ss_t_null, 0);
  ss_t      = ss_alloc(ss_t_boolean, 0);
  ss_f      = ss_alloc(ss_t_boolean, 0);
  ss_eos    = ss_alloc(ss_t_eos, 0);

  symbols = ss_nil;
}

void ss_init_prim(ss_s_env *ss_env)
{
  ss sym;
#define ss_prim_def(NAME,MINARGS,MAXARGS,NO_SIDE_EFFECT,DOCSTRING)      \
  sym = ss_sym(NAME);                                                   \
  ss_PASTE2(ss_p_,NAME) =                                               \
    ss_alloc_copy(ss_t_prim, sizeof(ss_s_prim), &ss_PASTE2(_ss_p_,NAME)); \
  ss_define(ss_env, sym, ss_PASTE2(ss_p_,NAME));                        \
  ss_UNBOX(symbol, sym).is_const = 1;
#include "prim.def"

#define ss_syntax_def(NAME,MINARGS,MAXARGS,NO_SIDE_EFFFECT,DOCSTRING)     \
  sym = ss_sym(NAME);                                           \
  ss_UNBOX(symbol, sym).syntax = ss_PASTE2(ss_p_ss_syn_,NAME);
#include "syntax.def"
}
void ss_init_cfunc(ss_s_env *ss_env);

ss ss_prompt(ss_s_env *ss_env, ss input, ss prompt)
{
  if ( ! feof(FP(input)) && prompt != ss_f )
    fprintf(FP(prompt), " ss> ");
  return ss_read(ss_env, input);
}

ss ss_repl(ss_s_env *ss_env, ss input, ss output, ss prompt, ss trap_error)
{
  ss expr, value = ss_undef;
  while ( 1 ) {
    jmp_buf jb;

    if ( ! setjmp(jb) ) {
      if ( trap_error != ss_f ) ss_error_init(ss_env, &jb);
      if ( (expr = ss_prompt(ss_env, input, prompt)) == ss_eos ) break;

    if ( prompt != ss_f ) {
      fprintf(*ss_stderr, ";; read => "); ss_write(expr, ss_stderr); fprintf(*ss_stderr, "\n");
    }

    value = ss_undef;
    value = ss_eval(expr);

    if ( prompt != ss_f ) {
      fprintf(*ss_stderr, ";; rewrite => "); ss_write(expr, ss_stderr); fprintf(*ss_stderr, "\n");
    }
    if ( value != ss_undef ) {
      if ( output != ss_f ) {
        ss_write(value, ss_stdout); fprintf(*ss_stdout, "\n");
      }
      if ( 0 ) {
        fprintf(*ss_stderr, ";; %lld (%p)\n", (long long) ss_unbox(fixnum, value), (void*) value);
        fprintf(*ss_stderr, ";; %llu bytes %llu objects\n",
                (unsigned long long) ss_malloc_bytes,
                (unsigned long long) ss_malloc_objects);
      }
    }
    }
  }
  return value;
}

ss ss_port_close(ss port)
{
  ss_s_port *self = (ss_s_port*) port;
  if ( self->fp ) {
    fclose(self->fp);
    self->fp = 0;
  }
  return ss_undef;
}

void ss_s_port_finalize(void *port, void *arg)
{
  ss_s_port *self = (ss_s_port*) port;
  if ( self->fp ) {
    fprintf(stderr, "  ;; finalizing #@%p %s\n", self, ss_string_V(self->name));
  }
  ss_port_close(port);
}

ss ss_m_port(FILE *fp, const char *name, const char *mode)
{
  ss_s_port *self;
  if ( ! fp ) return ss_f;
  self = ss_alloc(ss_t_port, sizeof(*self));
  self->fp = fp;
  self->name = ss_s((void*) name);
  self->mode = ss_s((void*) mode);
  GC_register_finalizer(self, ss_s_port_finalize, 0, 0, 0);
  return self;
}

void ss_init_port(ss_s_env *ss_env)
{
#define P(NAME,MODE)                                    \
  ss_##NAME = ss_m_port(NAME, "<" #NAME ">", MODE);     \
  ss_define(ss_env, ss_sym(ss_##NAME), ss_m_global(ss_sym(ss_##NAME), &ss_##NAME));
  P(stdin, "r");
  P(stdout, "w");
  P(stderr, "w");
#undef P
}

void ss_init_global(ss_s_env *ss_env)
{
}

int main(int argc, char **argv)
{
  ss_s_env *ss_env;
  GC_INIT();
  // GC_register_displacement(sizeof(ss) * 2);
  ss_top_level_env = ss_current_env = ss_env = ss_m_env(0);
  ss_init_const(ss_env);
  ss_init_symbol(ss_env);
  ss_init_port(ss_env);
  ss_init_global(ss_env);
  ss_init_prim(ss_env);
  ss_init_cfunc(ss_env);
  if ( 1 ) {
    FILE *fp = fopen("lib/boot.scm", "r");
    ss out = ss_f; // ss_stderr;
    ss_repl(ss_env, &fp, out, out, ss_f);
    fclose(fp);
  }
  {
    ss prompt = ss_stderr;
    ss input  = ss_stdin;
    ss output = ss_stdout;
    ss trap_errors = ss_t;
    if ( ! isatty(0) ) {
      prompt = trap_errors = ss_f;
    }
    ss_repl(ss_env, input, output, prompt, trap_errors);
  }
  return 0;
}

#define ss_apply_sym(SYM, NARGS, ARGS, ...)                     \
  ({ ss __sym = ss_sym(SYM);                                    \
    ss_apply(ss_env, ss_eval(__sym), ss_vec(NARGS, ARGS));      \
  })

#define VALUE ss
#define READ_DECL ss ss_read(ss_s_env *ss_env, ss stream)
#define READ_CALL() ss_read(ss_env, stream)
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
#define SET_CDR(C,R) (ss_CDR(C) = (R))
#define MAKE_CHAR(I) ss_c((I))
#define LIST_2_VECTOR(X) ss_list_to_vector(X)
#define STRING(S,L) ss_strnv((L), (S))
#define SYMBOL_DOT ss_sym(DOT)
#define SYMBOL(N) ss_sym(N)
#define STRING_2_NUMBER(s, radix) ss_string_TO_number(s, radix)
#define STRING_2_SYMBOL(s) ss_box(symbol, ss_string_V(s))
#define ERROR(msg,args...) ss_error(ss_env, "read: " msg, stream, ##args)
#define RETURN(X) return X
#define MALLOC(S) GC_malloc_atomic(S)
#define REALLOC(P,S) GC_realloc(P,S)
#define FREE(P) GC_free(P)
#define CALL_MACRO_CHAR(c) ss_apply_sym(ss_call_macro_char, 2, c, stream)
#include "lispread/lispread.c"

ss ss_cfunc_sym(const char *name)
{
  char buf[128];
  snprintf(buf, sizeof(buf) - 1, "C:%s", name);
  return ss_box_symbol(buf);
}

void ss_init_cfunc(ss_s_env *ss_env)
{
  static struct {
    const char *rtn, *name, *args, *desc;
    void *ptr;
  } inits[] = {
#define ss_cfunc_def(TYPE,NAME,ARGS) { TYPE, #NAME, ARGS, TYPE " " #NAME ARGS, &NAME },
#include "cfunc.def"
    { 0 }
  };
  for ( int i = 0; inits[i].rtn; ++ i ) {
    ss sym = ss_cfunc_sym(inits[i].name);
    ss_define(ss_env, sym, ss_m_cfunc(inits[i].ptr, inits[i].name, inits[i].desc));
    ss_UNBOX(symbol, sym).is_const = 1;
  }
}
