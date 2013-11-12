#include "ss.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h> /* memcpy() */
#include <assert.h>

ss ss_undef, ss_unspec, ss_nil, ss_t, ss_f, ss_eos;

ss _ss_exec(ss_s_environment *ss_env, ss *_ss_expr);
#define ss_expr (*_ss_expr)
#define ss_exec(X) _ss_exec(ss_env, &(X))
#define ss_constantExprQ ss_env->constantExprQ
#define ss_rewrite_expr(X)                               \
  do {                                                   \
  fprintf(stdout, ";; rewrite: ");                       \
  ss_write(ss_expr);                                     \
  fprintf(stdout, "\n");                                 \
  ss_expr = (X);                                         \
  fprintf(stdout, ";;      as: ");                       \
  ss_write(ss_expr);                                     \
  fprintf(stdout, "\n");                                 \
  } while ( 0 )

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

ss ss_error(const char *format, ...)
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

void ss_write_real(ss v, FILE *out)
{
  char buf[64];
  snprintf(buf, 63, "%g", ss_unbox(real, v));
  if ( ! (strchr(buf, 'e') || strchr(buf, '.')) ) {
    strcat(buf, ".0");
  }
  fprintf(out, "%s", buf);
}

ss ss_write(ss v)
{
  FILE *out = stdout;
  switch ( ss_type(v) ) {
  case ss_t_undef:   fprintf(out, "#<undef>"); break;
  case ss_t_integer: fprintf(out, "%lld",   (long long) ss_unbox(integer, v)); break;
  case ss_t_real:    ss_write_real(v, out); break;
  case ss_t_string:  fprintf(out, "\"%s\"", ss_string_v(v)); break;
  case ss_t_char:    fprintf(out, "#\\%c",  ss_unbox(char, v)); break;
  case ss_t_boolean: fprintf(out, "#%c",    v == ss_t ? 't' : 'f'); break;
  case ss_t_syntax:  fprintf(out, "#<syntax %s>", ss_UNBOX(syntax, v)->_name); break;
  case ss_t_prim:    fprintf(out, "#<prim %s>",   ss_UNBOX(prim, v)->_name); break;
  case ss_t_symbol:  fprintf(out, "%s",   ss_string_v(ss_UNBOX(symbol, v)._str)); break;
  case ss_t_var_ref:
    fprintf(out, "#<var_ref ");
    ss_write(ss_UNBOX(var_ref, v)._name);
    fprintf(out, " %d %d>", ss_UNBOX(var_ref, v)._up, ss_UNBOX(var_ref, v)._over);
    break;
  case ss_t_quote:   fprintf(out, "'"); ss_write(ss_UNBOX(quote, v)); break;
  case ss_t_eos:     fprintf(out, "#<eos>"); break;
  case ss_t_null:    fprintf(out, "()"); break;
  case ss_t_closure:
    fprintf(out, "#<closure ");
    ss_write(ss_UNBOX(closure, v).params);
    fprintf(out, ">");
    break;
  default:           fprintf(out, "#<??? %d @%p>", ss_type(v), (void*) v); break;
  case ss_t_cons:
    fprintf(out, "(");
    while ( v != ss_nil ) {
      switch ( ss_type(v) ) {
      case ss_t_cons:
        ss_write(ss_car(v));
        v = ss_cdr(v);
        if ( v != ss_nil )
          fprintf(out, " ");
        break;
      default:
        fprintf(out, " . ");
        ss_write(v);
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
        ss_write(ss_vector_v(v)[i]);
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

ss_integer_t ss_unbox_integer(ss v)
{
  ss_typecheck(ss_t_integer,v);
  return ss_UNBOX_integer(v);
}

ss ss_box_real(ss_real_t v)
{
  ss_s_real *self = ss_alloc(ss_t_real, sizeof(*self));
  self->_v = v;
  return self;
}

ss_real_t ss_unbox_real(ss v)
{
  ss_typecheck(ss_t_real, v);
  return ss_UNBOX_real(v);
}

ss ss_box_char(int _v)
{
  return ss_BOX_char(_v);
}
int      ss_unbox_char(ss v)
{
  ss_typecheck(ss_t_char, v);
  return ((int) ss_UNBOX_char(v)) & 0xff;
}

ss ss_strn(size_t l)
{
  ss_s_string *self = ss_alloc(ss_t_string, sizeof(*self));
  self->_v = ss_malloc(sizeof(self->_v[0]) * (l + 1));
  self->_l = l;
  return self;
}

ss ss_strnv(size_t l, const char *v)
{
  ss self = ss_strn(l);
  memcpy(ss_string_v(self), v, sizeof(ss_string_v(self)[0]) * l);
  ss_string_v(self)[l] = 0;
  return self;
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
      if ( strcmp(name, ss_string_v(sym->_str)) == 0 )
        goto rtn;
    }
  }

  sym = ss_alloc(ss_t_symbol, sizeof(*sym));
  sym->_str = ss_strnv(strlen(name), name);
  sym->_value = ss_undef;
  sym->_const = 0;

  ss_symbols = ss_cons(sym, ss_symbols);

 rtn:
  // fprintf(stderr, "  symbol(%s) => %p\n", name, sym);
  return sym;
}

void ss_init_symbol(ss_s_environment *ss_env)
{
#define ss_sym_def(X) ss_PASTE2(_ss_sym_, X) = ss_box_symbol(#X);
#include "sym.def"
  ss_sym(ADD) = ss_box_symbol("+");
  ss_sym(SUB) = ss_box_symbol("-");
  ss_sym(MUL) = ss_box_symbol("*");
  ss_sym(DIV) = ss_box_symbol("/");
  ss_sym(DOT) = ss_box_symbol(".");
  ss_sym(unquote_splicing) = ss_box_symbol("unquote-splicing");
}

ss ss_box_quote(ss v)
{
  if ( ss_literalQ(v) ) {
    return(v);
  } else {
    ss_s_quote *self = ss_alloc(ss_t_quote, sizeof(*self));
    self->_value = v;
    return self;
  }
}

ss ss_unbox_quote(ss v)
{
  ss_typecheck(ss_t_quote,v);
  return ss_UNBOX_quote(v);
}

ss ss_cons(ss a, ss d)
{
  ss_s_cons *self = ss_alloc(ss_t_cons, sizeof(*self));
  self->_car = a;
  self->_cdr = d;
  return self;
}
ss* _ss_car(ss a)
{
  ss_typecheck(ss_t_cons,a);
  return &ss_CAR(a);
}
ss* _ss_cdr(ss a)
{
  ss_typecheck(ss_t_cons,a);
  return &ss_CDR(a);
}

size_t ss_list_length(ss x)
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

ss ss_list_to_vector(ss x)
{
  size_t l = 0;
  ss v = ss_vecn(ss_list_length(x));
  again:
  switch ( ss_type(x) ) {
  case ss_t_cons:
    ss_vector_v(v)[l ++] = ss_CAR(x);
    x = ss_CDR(x);
    goto again;
  case ss_t_null:
    return v;
  case ss_t_vector:
    memcpy(ss_vector_v(v) + l, ss_vector_v(x), sizeof(ss_vector_v(v)[0]) * ss_vector_l(x));
    return v;
  default: abort();
  }
  return v;
}

ss ss_vecn(size_t l)
{
  ss_s_vector *self = ss_alloc(ss_t_vector, sizeof(*self));
  self->_v = ss_malloc(sizeof(self->_v[0]) * l);
  self->_l = l;
  return self;
}

ss ss_vecnv(size_t l, const ss *v)
{
  ss self = ss_vecn(l);
  memcpy(ss_vector_v(self), v, sizeof(ss_vector_v(self)[0]) * l);
  return self;
}

ss ss_vec(int n, ...)
{
  ss x = ss_vecn(n);
  int i;
  va_list vap;
  va_start(vap,n);  
  for ( i = 0; i < n; i ++ )
    ss_vector_v(x)[i] = va_arg(vap, ss);
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

ss ss_m_var_ref(ss sym, int up, int over)
{
  ss_s_var_ref *self = ss_alloc(ss_t_var_ref, sizeof(*self));
  self->_name = sym;
  self->_up = up;
  self->_over = over;
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

ss *ss_bind(ss *_ss_expr, ss_s_environment *env, ss var)
{
  int up, over;
  switch ( ss_type(var) ) {
  case ss_t_symbol:
    up = 0;
    while ( env ) {
      for ( over = 0; over < env->argc; ++ over ) {
        // fprintf(stdout, ";; bind "); ss_write(var); fprintf(stdout, " = "); ss_write(env->symv[over]); fprintf(stdout, "\n");
        if ( ss_EQ(var, env->symv[over]) ) {
          ss_expr = ss_m_var_ref(var, up, over);
          return &env->argv[over];
        }
      }
      ++ up;
      env = env->parent;
    }
    return(ss_error("unbound ~S", var));
  case ss_t_var_ref:
    up   = ss_UNBOX(var_ref, var)._up;
    over = ss_UNBOX(var_ref, var)._over;
    while ( up -- > 0 ) env = env->parent;
    assert(env);
    return &env->argv[over];
  default:
    return(ss_error("unbound ~S", var));
  }
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

void _ss_min_args_error(const char *DOCSTRING, int ss_argc, int MINARGS)
{
  ss_error("apply not-enough-args (%s) got %d expected %d", DOCSTRING, ss_argc, MINARGS);
}

void _ss_max_args_error(const char *DOCSTRING, int ss_argc, int MAXARGS)
{
  ss_error("apply too-many-args (%s) got %d expected %d", DOCSTRING, ss_argc, MAXARGS);
}

ss_syntax(define,2,2,0,"define name value") {
  ss_return(ss_vec(3, ss_sym(_define), ss_box(quote, ss_argv[0]), ss_argv[1]));
} ss_end

ss_prim(_define,2,2,1,"define name value") {
  ss_return(ss_define(ss_env->top_level, ss_argv[0], ss_argv[1]));
} ss_end

ss_prim(write,1,1,1,"write object")
  ss_write(ss_argv[0]);
ss_end

ss_prim(newline,0,0,0,"newline")
  fprintf(stdout, "\n");
ss_end

ss_syntax(quote,1,1,0,"quote value")
  ss_constantExprQ = 1;
  ss_return(ss_box(quote,ss_argv[0]));
ss_end

ss_syntax(if,2,3,1,"if pred true ?false?")
  ss_return(ss_vec(4, ss_sym(_if), ss_argv[0], ss_argv[1], (ss_argc == 3 ? ss_exec(ss_argv[2]) : ss_undef)));
ss_end

ss_prim(_if,-1,-1,0,"if pred true ?false?")
  ss x = ss_exec(ss_argv[0]);
  if ( ss_constantExprQ ) {
    ss_rewrite_expr(ss_NE(x, ss_f) ? ss_argv[1] : ss_argv[2]);
    ss_return(ss_exec(ss_expr));
  }
  ss_return(ss_NE(x, ss_f) ? ss_exec(ss_argv[1]) : ss_exec(ss_argv[2]));
ss_end

ss_syntax(lambda,1,-1,0,"lambda formals body...")
  ss_s_closure *self = ss_alloc(ss_t_closure, sizeof(*self));
  self->params = ss_list_to_vector(ss_argv[0]);
  self->body = ss_vecnv(ss_argc - 1, ss_argv + 1);
  ss_return(self);
ss_end

ss_prim(cons,2,2,1,"cons car cdr")
  ss_return(ss_cons(ss_argv[0], ss_argv[1]));
ss_end

ss_prim(car,1,1,1,"car pair")
  ss_typecheck(ss_t_cons,ss_argv[0]);
  ss_return(ss_CAR(ss_argv[0]));
ss_end

ss_prim(cdr,1,1,1,"cdr pair")
  ss_typecheck(ss_t_cons,ss_argv[0]);
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
  ss_prim(_##NAME,1,1,1,#OP " z")                                       \
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
#include "cops.def"

ss _ss_exec(ss_s_environment *ss_env, ss *_ss_expr)
{
  ss var ,rtn;
#define return(X) do { rtn = (X); goto _return; } while(0)
  again:
  ss_constantExprQ = 0;
  rtn = ss_expr;
  fprintf(stdout, ";; exec: "); ss_write(ss_expr); fprintf(stdout, "\n");
  switch ( ss_type(ss_expr) ) {
  case ss_t_quote:
    ss_constantExprQ = 1;
    return(ss_UNBOX(quote,ss_expr));
  case ss_t_symbol: {
    var = ss_expr;
    rtn = ss_get(&ss_expr, ss_env, var);
    rewrite_const_var:
    if ( (ss_constantExprQ = ss_symbol_const(var)) )
      ss_rewrite_expr(ss_box(quote,rtn));
    return(rtn);
  }
  case ss_t_var_ref: {
    var = ss_UNBOX(var_ref, ss_expr)._name;
    rtn = ss_get(&ss_expr, ss_env, ss_expr);
    goto rewrite_const_var;
  }
  case ss_t_closure:
    {
      ss_s_closure *self = ss_alloc_copy(ss_t_closure, sizeof(*self), ss_expr);
      self->env = ss_env;
      return(self);
    }
  case ss_t_cons:
    ss_rewrite_expr(ss_list_to_vector(ss_expr));
    /* FALL THROUGH */
  case ss_t_vector: {
    ss op;
    if ( ss_vector_l(ss_expr) < 1 ) return(ss_error("apply empty-vector"));
    op = ss_exec(ss_vector_v(ss_expr)[0]);
    if ( ss_constantExprQ )
      ss_vector_v(ss_expr)[0] = op;
    switch ( ss_type(op) ) {
    case ss_t_syntax:
      ss_rewrite_expr((ss_UNBOX(prim,op)->_func)(ss_env, &ss_expr, ss_vector_l(ss_expr) - 1, ss_vector_v(ss_expr) + 1));
      goto again;
    case ss_t_prim:
      return((ss_UNBOX(prim,op)->_func)(ss_env, &ss_expr, ss_vector_l(ss_expr) - 1, ss_vector_v(ss_expr) + 1));
    case ss_t_closure:
      {
        ss_s_closure *self = (ss_s_closure*) op;
        size_t i, ss_argc = ss_vector_l(ss_expr) - 1;
        ss *ss_argv = ss_vector_v(ss_expr) + 1;
        ss_s_environment *env;

        if ( ss_argc != ss_vector_l(self->params) )
          return(ss_error("apply wrong-number-of-arguments given %lu, expected %lu", (unsigned long) ss_argc, (unsigned long) ss_vector_l(self->params)));
        env = ss_m_environment(self->env);
        env->argc = ss_argc;
        env->symv = ss_vector_v(self->params);
        env->argv = memcpy(ss_malloc(sizeof(env->argv[0]) * ss_argc), ss_argv, sizeof(env->argv[0]) * ss_argc);
        for ( i = 0; i < ss_argc; ++ i )
          env->argv[i] = ss_exec(ss_argv[i]);
        fprintf(stdout, ";; apply closure "); ss_write(self->params); ss_write(ss_expr); fprintf(stdout, "\n");
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
      return(ss_error("apply cannot apply type=%d", (int) ss_type(op)));
    }
  }
  default:
    ss_constantExprQ = 1;
  }
#undef return
  _return:
  fprintf(stdout, ";; exec result expr: "); ss_write(ss_expr); fprintf(stdout, "\n");
  fprintf(stdout, ";; exec result  val: "); ss_write(rtn); fprintf(stdout, "\n");
  return rtn;
}

ss ss_read(ss port);
ss_prim(_read,1,1,1,"_read port")
{
  ss_return(ss_read(ss_argv[0]));
}
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
  ss_define(ss_env, sym, ss_alloc_copy(TYPE, sizeof(ss_s_prim), &ss_PASTE2(_ss_p_,NAME)));
#include "prim.def"
}

ss ss_prompt()
{
  fprintf(stderr, " ss> ");
  return ss_read(&stdin);
}

void ss_repl(ss_s_environment *ss_env)
{
  ss expr, value = ss_undef;
  ss_constantExprQ = 0;
  while ( (expr = ss_prompt()) != ss_eos ) {
    value = ss_exec(expr);
    if ( value != ss_undef ) {
      ss_write(value); fprintf(stdout, "\n");
      printf(";; %lld (%p)\n", (long long) ss_unbox(integer, value), (void*) value);
    }
  }
}

int main(int argc, char **argv)
{
  ss_s_environment *ss_env;
  GC_INIT();
  ss_env = ss_m_environment(0);
  ss_init_const(ss_env);
  ss_init_symbol(ss_env);
  ss_init_prim(ss_env);
  ss_repl(ss_env);
  return 0;
}

#define VALUE ss
#define READ_DECL ss ss_read(ss stream)
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

