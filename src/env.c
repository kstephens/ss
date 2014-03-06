
ss ss_const_var_assign(ss_s_env *ss_env, ss sym)
{
  return(ss_error(ss_env, "constant-variable-assignment", sym, 0));
}

ss_INLINE
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

ss_INLINE
ss ss_env_symv(ss _o)
{
  ss_s_env *o = _o;
  return ss_vecnv(o->argc, o->symv);
}

ss_INLINE
ss ss_closure_env(ss _o)
{
  ss_s_closure *o = _o;
  return o->env;
}

ss_INLINE
ss ss_m_var(ss sym, int up, int over)
{
  ss_s_var *self = ss_alloc(ss_t_var, sizeof(*self));
  self->name = sym;
  self->up = up;
  self->over = over;
  return self;
}

ss_INLINE
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

  if ( ((ss_s_symbol*) sym)->is_const )
    return ss_const_var_assign(env, sym);

  for ( i = 0; i < env->argc; ++ i ) {
    if ( sym == env->symv[i] ) {
      env->argv[i] = val;
      return sym;
    }
  }
  env->symv = memcpy(ss_malloc(sizeof(env->symv) * (i + 1)), env->symv, sizeof(env->symv[0]) * i);
  env->symv[i] = sym;
  env->argv = memcpy(ss_malloc(sizeof(env->argv) * (i + 1)), env->argv, sizeof(env->argv[0]) * i);
  env->argv[i] = val;
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
          ref    = &env->argv[over];
          ss_rewrite_expr(var = ss_m_var(var, up, over), "var binding found");
          goto global_check;
        }
      }
      ++ up;
      env = env->parent;
    }
    return(ss_error(ss_env, "unbound", var, 0));

  case ss_te_var:
    up   = ((ss_s_var*) var)->up;
    while ( up -- > 0 ) env = env->parent;
    over = ((ss_s_var*) var)->over;
    ref  = &env->argv[over];
    sym  = ((ss_s_var*) var)->name;
  global_check:
    // if var ref is at top-level,
    if ( ! env->parent ) {
      if ( ss_type_te(*ref) == ss_te_global ) {
        // Replace var value with a global var.
        ss_rewrite_expr(*ref, "variable is global");
        ref = ((ss_s_global*) *ref)->ref;
      } else {
        // Create new cell with current var value.
        ss *cell = &ss_m_cell(*ref);
        // Replace var value with a global var.
        *ref = ss_m_global(sym, cell);
        // Replace expr with the global var.
        ss_rewrite_expr(*ref, "var is global");
        // Point ref into cell.
        ref = cell;
      }
    }
    goto const_var_check;

  case ss_te_global:
    sym = ((ss_s_global*) var)->name;
    ref = ((ss_s_global*) var)->ref;
  const_var_check:
    if ( ((ss_s_symbol*) sym)->is_const ) {
      if ( set ) return ss_const_var_assign(ss_env, sym);
      ss_constantExprQ = 1;
      ss_rewrite_expr(ss_box_quote(*ref), "top-level variable is constant");
    }
    return ref;

  default:
    return(ss_error(ss_env, "bind", var, "expected var"));
    break;
  }
}

ss_INLINE
ss ss_var_set(void *env, ss *_ss_expr, ss var, ss val)
{
  *ss_bind(env, _ss_expr, var, 1) = val;
  return var;
}

ss_INLINE
ss ss_var_get(void *env, ss *_ss_expr, ss var)
{
  return *ss_bind(env, _ss_expr, var, 0);
}

