ss ss_make_syntax(ss sym, ss proc)
{
  ss_typecheck(ss_t_symbol, sym);
  ss_UNB(symbol, sym).syntax = proc;
  return sym;
}

ss_syntax(define,1,-1,0,"define name value") {
  ss name = ss_argv[0];
  if ( ss_type_te(name) == ss_te_pair ) {
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
    if ( ss_type_te(rest) == ss_te_pair && ss_CAR(rest) == ss_sym(_rest) ) {
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
