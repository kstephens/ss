
#undef T

ss_prim(ss_call_cfunc,0,5,1,"call cfunc")
{
#define T ss
#define A(X) X
#define R(X) ss_return(X)
#include "cfunc_call.h"
}
ss_end

ss_prim(ss_call_cfunc_double,0,5,1,"call cfunc double")
{
#define T double
#define A(X) ss_flonum_(X)
#define R(X) ss_return(ss_box_flonum(X))
#include "cfunc_call.h"
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

ss ss_cfunc_sym(const char *name)
{
  char buf[128] = { 0 };
  snprintf(buf, sizeof(buf) - 1, "C:%s", name);
  assert(buf[126] == 0);
  return ss_box_symbol(buf);
}

ss ss_symbol_list();
ss ss_prim_list();
ss ss_syntax_list();

static
ss cfunc_list;
ss ss_cfunc_list()
{
  return cfunc_list;
}

ss ss_define_cfunc(ss_s_env *ss_env, const char *name, void *func, int nargs, const char *fname, const char *docstr)
{
  ss sym = ss_cfunc_sym(name ? name : fname);
  ss_s_prim *prim = ss_m_cfunc(func, fname, docstr);
  ss_define(ss_env, sym, prim);
  ss_UNB(symbol, sym).is_const = 1;
  cfunc_list = ss_cons(ss_cons(sym, prim), cfunc_list);
  return prim;
}

static inline
ss ss_throw_(ss_s_env *ss_env, ss _1, ss _2)
{
  ss_s_catch *catch = _1;
  ss_s_throwable *thrown = _2;
  assert(catch);
  assert(thrown);
  thrown->data.env = ss_env;
  thrown->data.expr = ss_env->expr;
  return __ss_throw(ss_env, catch, thrown);
}

