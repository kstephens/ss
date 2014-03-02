
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
  env->catch       = parent ? parent->catch       : 0;
  env->error_catch = parent ? parent->error_catch : 0;
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
  switch ( ss_type_te(var) ) {
  case ss_te_symbol:
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
  case ss_te_var:
    sym  = ss_UNB(var, var).name;
    up   = ss_UNB(var, var).up;
    over = ss_UNB(var, var).over;
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
  return(ss_error(ss_env, "unbound", var, 0));

 rtn:
  ref = &env->argv[over];
 ref:
  if ( ss_type_te(*ref) == ss_te_global ) {
    sym = ((ss_s_global*) *ref)->name;
    ss_rewrite_expr(*ref, "global binding is known");
    ref = &ss_UNB(global, *ref);
  }
  if ( ss_UNB(symbol, sym).is_const && env->parent == 0 ) {
    if ( set ) return(ss_error(ss_env, "constant-variable", sym, 0));
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

