
static ss prim_list;
ss ss_prim_list()
{ return prim_list; }

static ss syntax_list;
ss ss_syntax_list()
{ return syntax_list; }

void ss_init_prim(ss_s_env *ss_env)
{
  static struct {
    const char *name;
    int minargs, maxargs, no_side_effect;
    const char *desc;
    void *prim_struct;
    ss *primp;
  } inits[] = {
#define ss_prim_def(NAME,MINARGS,MAXARGS,NO_SIDE_EFFECT,DOCSTRING) \
    { #NAME, MINARGS, MAXARGS, NO_SIDE_EFFECT, DOCSTRING, &ss_PASTE2(_ss_p_,NAME), &ss_PASTE2(ss_p_,NAME) },
#include "prim.def"
    { 0 }
  };
  int i;
  ss sym;

  prim_list = ss_nil;
  for ( i = 0; inits[i].name; ++ i ) {
    ss prim = ss_alloc_copy(ss_t_prim, sizeof(ss_s_prim), inits[i].prim_struct);
    *inits[i].primp = prim;
    sym = ss_box_symbol(inits[i].name);
    ss_define(ss_env, sym, prim);
    ((ss_s_symbol*) sym)->is_const = 1;
    prim_list = ss_cons(ss_cons(sym, prim), prim_list);
  }

  {
    ss syntax;

#define ss_syntax_def(NAME,MINARGS,MAXARGS,NO_SIDE_EFFFECT,DOCSTRING)   \
    sym = ss_sym(NAME);                                                 \
    syntax = ss_PASTE2(ss_p_ss_syn_,NAME);                              \
    ((ss_s_symbol*) sym)->syntax = syntax;                              \
    syntax_list = ss_cons(ss_cons(sym, syntax), syntax_list);
#include "syntax.def"
  }
}
