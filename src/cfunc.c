
#undef T

ss_prim(ss_call_cfunc,0,5,1,"call cfunc")
{
#define T ss
#define A(X) X
#define R(X) ss_return(X)
#include "cfunc_call.h"
#undef T
#undef A
#undef R
}
ss_end

ss_prim(ss_call_cfunc_double,0,5,1,"call cfunc double")
{
#define T double
#define A(X) ss_flonum_(X)
#define R(X) ss_return(ss_box_flonum(X))
#include "cfunc_call.h"
#undef T
#undef A
#undef R
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

#ifdef ss_throw
#undef ss_throw
#endif
static inline
ss ss_throw (ss_s_env *ss_env, ss_s_catch *catch, ss_s_throwable *thrown)
{
  assert(catch);
  thrown->data.env = ss_env;
  thrown->data.expr = ss_env->expr;
  return __ss_throw(ss_env, catch, thrown);
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
    ss_s_prim *func = ss_m_cfunc(inits[i].ptr, inits[i].name, inits[i].desc);
    if ( strcmp(inits[i].rtn, "double") == 0 &&
         (
          strcmp(inits[i].args, "(double)") == 0 ||
          strcmp(inits[i].args, "(double,double)") == 0
          ) ) {
      func->func = _ss_pf_ss_call_cfunc_double;
    }
    ss_define(ss_env, sym, func);
    ss_UNB(symbol, sym).is_const = 1;
    cfunc_list = ss_cons(ss_cons(sym, func), cfunc_list);
  }
}

