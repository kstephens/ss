#undef ss_sym_def
#define ss_sym_def(X) ss ss_PASTE2(_ss_sym_,X);
#include "sym.def"

static ss symbol_list;
ss ss_symbol_list()
{ return symbol_list; }

ss ss_box_symbol(const char *name)
{
  ss_s_symbol *sym;

  if ( name ) {
    for ( ss l = symbol_list; l != ss_nil; l = ss_cdr(l) ) {
      sym = (ss_s_symbol*) ss_car(l);
      if ( strcmp(name, ss_string_V(sym->name)) == 0 )
        goto rtn;
    }
  }

  sym = ss_alloc(name && name[0] == ':' ? ss_t_keyword : ss_t_symbol, sizeof(*sym));
  sym->name = name ? ss_strnv(strlen(name), name) : ss_f;
  sym->docstring = ss_f;
  sym->syntax = ss_f;
  sym->is_const = 0;
  if ( name )
    symbol_list = ss_cons(sym, symbol_list);

 rtn:
  return sym;
}

ss ss_make_constant(ss sym)
{
  ss_typecheck(ss_t_symbol, sym);
  ((ss_s_symbol*) sym)->is_const = 1;
  return sym;
}

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

