
ss ss_m_catch()
{
  ss_s_catch *self = ss_alloc(ss_t_catch, sizeof(*self));
  memset(self, 0, sizeof(*self));
  self->body = self->rescue = self->ensure = self->value = ss_undef;
  return self;
}

ss ss_catch(ss_s_env *ss_env, ss body, ss rescue, ss ensure)
{
  ss_s_catch *c = ss_m_catch();
  ss rtn = ss_undef;
  c->body = body;
  c->rescue = rescue;
  c->ensure = ensure;
  ss_CATCH(c) {
    rtn = ss_apply(ss_env, c->body, ss_vec1(c));
  }
  ss_CATCH_RESCUE {
    if ( c->rescue != ss_f )
      rtn = ss_apply(ss_env, c->rescue, ss_vec2(c->src, c->src->value));
  }
  ss_CATCH_ENSURE {
    if ( c->ensure != ss_f )
      ss_apply(ss_env, c->ensure, ss_vec1(c));
  }
  ss_CATCH_END;
  return rtn;
}

ss ss_error_raise(ss_s_env *ss_env, ss error)
{
  ss_s_env *e = ss_env;
  if ( ! e ) e = ss_current_env;
  while ( e && ! e->error_catch ) e = e->parent;
  if ( ! e ) {
    fprintf(*ss_stderr, "ss: no error catch: aborting\n");
    abort();
  }
  return ss_throw(ss_env, e->error_catch, error);
}

#define FP(port) (*(FILE**) (port))
ss ss_error(ss_s_env *ss_env, const char *code, ss obj, const char *format, ...)
{
  char msg[128] = { 0 };
  size_t msg_len = sizeof(msg) - 1;
  va_list vap;
  if ( format ) {
    va_start(vap, format);
    msg_len -= vsnprintf(msg, msg_len, format, vap);
    va_end(vap);
  }

  // Move this to ss_write for ss_t_error.
  fprintf(FP(ss_stderr), "\n  ;; ss: error: %s ", code);
  ss_write(obj, ss_stderr);
  fprintf(FP(ss_stderr), ": %s\n", msg);
  for ( ss_s_env *env = ss_env; env; env = env->parent ) {
    fprintf(FP(ss_stderr), "  ;; ss: %3d ", (int) env->depth);
    ss_write(env->expr, ss_stderr);
    fprintf(FP(ss_stderr), "\n");
  }

  obj = ss_vec(4, ss_sym(error), ss_box_symbol(code), obj, ss_s(msg));
  ss_error_raise(ss_env, obj);
  return 0;
}

static inline
ss ss_typecheck_error(ss v)
{
  return ss_error(ss_current_env, "typecheck", v, 0);
}

static inline
ss ss_typecheck(ss_e_type t, ss v)
{
  if ( ss_type_e(v) != t )
    return ss_typecheck_error(v);
  return v;
}

ss ss_errno() { return ss_BOX_fixnum(errno); }
ss ss_errstr(ss en)
{
  int i;
  if ( en == ss_f ) en = ss_errno();
  i = ss_UNBOX_fixnum(en);
  return ss_s(strerror(i));
}

void _ss_min_args_error(ss_s_env *ss_env, ss op, const char *DOCSTRING, int ss_argc, int MINARGS)
{
  ss_error(ss_env, "not-enough-args", op, "(%s) given %d expected at least %d", DOCSTRING, ss_argc, MINARGS);
}

void _ss_max_args_error(ss_s_env *ss_env, ss op, const char *DOCSTRING, int ss_argc, int MAXARGS)
{
  ss_error(ss_env, "too-many-args", op, "(%s) given %d expected %d", DOCSTRING, ss_argc, MAXARGS);
}

