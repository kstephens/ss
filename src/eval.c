ss _ss_eval(ss_s_env *ss_env, ss *_ss_expr, ss *ss_argv)
{
  ss rtn, expr;
  int const_argsQ;
  size_t ss_argc;
#define return(X) do { rtn = (X); goto _return; } while(0)
  expr = ss_expr;
  ++ ss_env->depth;
  if ( ss_argv ) {
    if ( ss_eval_verbose ) {
      fprintf(*ss_stderr, "  ;; eval %3d E#@%p #@%p (", (int) ss_env->depth, ss_env, _ss_expr); ss_write(expr, ss_stderr); fprintf(*ss_stderr, " . "); ss_write(ss_argv, ss_stderr); fprintf(*ss_stderr, ") (apply)\n");
    }
    rtn = expr;
    ss_argc = ss_vector_L(ss_argv);
    ss_argv = ss_vector_V(ss_argv);
    const_argsQ = 0;
    ss_current_env = ss_env;
    goto call;
  }
  again:
  ss_env->expr = expr;
  ss_current_env = ss_env;
#define ss_eval_tail(X) do {     \
    expr = *(_ss_expr = &(X));   \
    goto again;                  \
  } while(0)
  ss_constantExprQ = 0;
  if ( ss_eval_verbose ) {
    fprintf(*ss_stderr, "  ;; eval %3d E#@%p #@%p ", (int) ss_env->depth, ss_env, _ss_expr); ss_write(expr, ss_stderr); fprintf(*ss_stderr, "\n");
  }
  switch ( ss_type_te(expr) ) {
  case ss_te_quote:
    ss_constantExprQ = 1;
    return(ss_UNB(quote, expr));
  case ss_te_symbol:
    if ( expr == ss_sym(_env) ) return(ss_env);
  case ss_te_var:
    return(ss_var_get(ss_env, _ss_expr, expr));
  case ss_te_var_set:
    {
      ss_s_var_set *self = expr;
      rtn = ss_eval(self->expr);
      ss_var_set(ss_env, &self->var, self->var, rtn);
      return(ss_undef);
    }
  case ss_te_global:
    return(ss_UNB(global, expr));
  case ss_te_if:
    {
      ss_s_if *self = ss_expr;
      ss *subexpr;
      rtn = ss_eval(self->t);
      subexpr = rtn != ss_f ? &self->a : &self->b;
      if ( 0 && ss_constantExprQ )
        ss_rewrite_expr(*subexpr, rtn != ss_f ? "constant test is true" : "constant test is false");
      else
        _ss_expr = subexpr;
      expr = *subexpr;
      goto again;
    }
  case ss_te_begin:
    {
      size_t i;
      for ( i = 0; i < ss_vector_L(expr) - 1; ++ i )
        ss_eval(ss_vector_V(expr)[i]);
      ss_eval_tail(ss_vector_V(expr)[i]);
    }
  case ss_te_lambda:
    {
      ss_s_closure *self = ss_alloc(ss_t_closure, sizeof(*self));
      self->lambda = expr;
      self->env = ss_env;
      // fprintf(stderr, "  #@%p => #<c #@%p E#@%p>\n", expr, self, self->env);
      return(self);
    }
  case ss_te_pair:
    rtn = ss_car(expr);
    if ( ss_type_te(rtn) == ss_te_symbol && (rtn = ss_UNB(symbol, rtn).syntax) != ss_f ) {
      expr = ss_apply(ss_env, rtn, ss_cdr(expr));
      ss_rewrite_expr(expr, "syntax rewrite");
      goto again;
    }
    expr = ss_list_to_vector(expr);
    /* FALL THROUGH */
  case ss_te_vector: /* FIXME: Some syntaxes expand into vectors. */
    ss_set_type(ss_t_app, expr);
    ss_rewrite_expr(expr, "application vector");
    /* FALL THROUGH */
  case ss_te_app: {
    if ( ss_vector_L(expr) < 1 ) return(ss_error(ss_env, "apply-empty", expr, 0));
    ss_argc = ss_vector_L(expr) - 1;

    rtn = ss_eval(ss_vector_V(expr)[0]);

    const_argsQ = 1;
    ss_argv = ss_malloc(sizeof(ss_argv[0]) * (ss_argc + 1)); // +1 restarg.
    for ( size_t i = 0; i < ss_argc; i ++ ) {
      ss_argv[i] = ss_eval(ss_vector_V(expr)[i + 1]);
      const_argsQ &= ss_constantExprQ;
    }

    call:
    ss_constantExprQ = 0;

    switch ( ss_type_te(rtn) ) {
    case ss_te_catch:
      {
        return((ss_UNB(prim, rtn)->func)(ss_env, _ss_expr, rtn, ss_argc, ss_argv));
      }
    case ss_te_prim:
      {
        expr = (ss_UNB(prim, rtn)->func)(ss_env, _ss_expr, rtn, ss_argc, ss_argv);
        if ( ss_eval_verbose ) {
          if ( const_argsQ ) fprintf(*ss_stderr, "    ;; const_argsQ %s\n", ss_UNB(prim, rtn)->no_side_effect ? "no-side-effect" : "");
        }
        if ( (ss_constantExprQ = const_argsQ && ss_UNB(prim, rtn)->no_side_effect) )
          ss_rewrite_expr(ss_box_quote(expr), "constant folding");
        return(expr);
      }
    case ss_te_closure:
      {
        ss_s_lambda *self = ((ss_s_closure*) rtn)->lambda;
        ss_s_env *env;

        if ( self->rest_i >= 0 ) {
          if ( ss_argc < self->rest_i )
            return(ss_error(ss_env, "not-enough-args", self, "given %lu, expected at least %lu", (unsigned long) ss_argc, (unsigned long) self->rest_i));
        } else {
          if ( ss_argc != ss_vector_L(self->params) )
            return(ss_error(ss_env, "too-many-args", self, "given %lu, expected %lu", (unsigned long) ss_argc, (unsigned long) ss_vector_L(self->params)));
        }

        env = ss_m_env(((ss_s_closure*) rtn)->env);
        env->catch = ss_env->catch;
        env->error_catch = ss_env->error_catch;
        env->expr = ss_expr;
        env->argc = ss_argc;
        env->symv = ss_vector_V(self->params);
        env->argv = ss_argv;
        if ( self->rest_i >= 0 ) {
          env->argv[self->rest_i] = ss_listnv(ss_argc - self->rest_i, env->argv + self->rest_i);
          env->argc = self->rest_i + 1;
        }
        if ( ss_eval_verbose ) {
          fprintf(*ss_stderr, "    ;; apply closure:\n");
          fprintf(*ss_stderr, "    ;;   args: (");
          ss_write_vec(env->argc, env->argv, ss_stderr);
          fprintf(*ss_stderr, ")\n    ;;     to: ");
          ss_write(self, ss_stderr);
          fprintf(*ss_stderr, "\n");
        }
        if ( ss_eval_verbose ) {
          size_t i;
          fprintf(*ss_stderr, "    ;; binding:\n");
          for ( i = 0; i < env->argc; ++ i ) {
            fprintf(*ss_stderr, "    ;;   ");
            ss_write(env->symv[i], ss_stderr);
            fprintf(*ss_stderr, " => ");
            ss_write(env->argv[i], ss_stderr);
            fprintf(*ss_stderr, "\n");
          }
        }

        ss_env = env;
        ss_eval_tail(self->body);
      }
      break;
    default:
      return(ss_error(ss_env, "typecheck", rtn, "cannot apply type %s", ss_type_(rtn)->name));
    }
  }
  default:
    rtn = expr;
    ss_constantExprQ = ss_literalQ(expr);
  }
#undef return
#undef ss_eval_tail
  _return:
  if ( ss_eval_verbose ) {
    fprintf(*ss_stderr, "  ;; eval %3d result expr: ", (int) ss_env->depth); ss_write(ss_expr, ss_stderr); fprintf(*ss_stderr, "\n");
    fprintf(*ss_stderr, "  ;; eval %3d result  val: ", (int) ss_env->depth);
    ss_write(rtn, ss_stderr);
    fprintf(*ss_stderr, " (%s) ", ss_constantExprQ ? "const" : "non-const");
    fprintf(*ss_stderr, "\n");
  }
  -- ss_env->depth;
  return rtn;
}

ss ss_apply(ss_s_env *ss_env, ss func, ss args)
{
  if ( ss_type_te(args) != ss_te_vector )
    args = ss_list_to_vector(args);
  return _ss_eval(ss_env, &func, args);
}

#define ss_apply_sym(SYM, NARGS, ARGS, ...)                     \
  ({ ss __sym = ss_sym(SYM);                                    \
    ss_apply(ss_env, ss_eval(__sym), ss_vec(NARGS, ARGS));      \
  })

ss_prim(apply,2,2,0,"apply func args") {
  ss_return(ss_apply(ss_env, ss_argv[0], ss_argv[1]));
} ss_end

ss_prim(eval,1,2,0,"eval expr env?") {
  ss_return(_ss_eval(ss_argc > 1 ? ss_argv[1] : ss_top_level_env, &ss_argv[0], 0));
} ss_end

