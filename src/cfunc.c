
#undef T

ss_prim(ss_call_cfunc,-1,-1,1,"call cfunc")
{
#define MINARGS ss_prim->min_args
#define MAXARGS ss_prim->max_args
#define DOCSTRING ss_prim->docstring
  if ( MINARGS >= 0 && ss_argc < MINARGS )
    _ss_min_args_error(ss_env, ss_prim, DOCSTRING, ss_argc, MINARGS);
  if ( MAXARGS >= 0 && ss_argc > MAXARGS )
    _ss_max_args_error(ss_env, ss_prim, DOCSTRING, ss_argc, MAXARGS);
#undef MINARGS
#undef MAXARGS
#undef DOCSTRING
#define T ss
#define A(X) X
#define R(X) ss_return(X)
#include "cfunc_call.h"
}
ss_end

ss ss_m_cfunc(void *ptr, const char *name, const char *docstr)
{
  ss_s_prim *self = ss_alloc(ss_t_prim, sizeof(*self));
  self->c_func = ptr;
  self->prim = _ss_pf_ss_call_cfunc;
  self->min_args = 0; self->max_args = 5;
  self->no_side_effect = 0;
  self->name = name;
  self->docstring = docstr ? docstr : name;
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
  prim->min_args = prim->max_args = nargs;
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

