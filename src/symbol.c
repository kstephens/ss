#undef ss_sym_def
#define ss_sym_def(X) ss ss_PASTE2(_ss_sym_,X);
#include "sym.def"

static ss symbol_list;
ss ss_symbol_list()
{ return symbol_list; }

static ss* symbol_table;
static size_t symbol_table_n;

int _ss_symbolP_cmp(const void *_a, const void *_b)
{
  ss_s_string *a = (*(ss_s_symbol**)_a)->name, *b = (*(ss_s_symbol**)_b)->name;
  int cmp = _ss_memcmpn(a->v, b->v, a->l, b->l);
  return cmp;
}

ss ss_box_symbol(const char *name)
{
  ss_s_symbol *sym;
  size_t name_len;

  if ( name ) {
    ss_s_string _name = { name, name_len = strlen(name) };
    ss_s_symbol _tmp;
    ss *entry;

    sym = &_tmp; sym->name = &_name;
    if ( (entry = bsearch(&sym, symbol_table, symbol_table_n, sizeof(symbol_table[0]), _ss_symbolP_cmp)) )
      return *entry;
  }

  sym = ss_alloc(name && name[0] == ':' ? ss_t_keyword : ss_t_symbol, sizeof(*sym));
  sym->name = name ? ss_strnv(name_len, name) : ss_f;
  sym->docstring = ss_f;
  sym->syntax = ss_f;
  sym->is_const = 0;
  if ( name ) {
    symbol_list = ss_cons(sym, symbol_list);
    symbol_table = GC_realloc(symbol_table, sizeof(symbol_table[0]) * (symbol_table_n + 1));
    symbol_table[symbol_table_n ++] = sym;
    mergesort(symbol_table, symbol_table_n, sizeof(symbol_table[0]), _ss_symbolP_cmp);
  }
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

