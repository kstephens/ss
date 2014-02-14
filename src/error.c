
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

ss ss_errno() { return ss_BOX_fixnum(errno); }
ss ss_errstr(ss en)
{
  int i;
  if ( en == ss_f ) en = ss_errno();
  i = ss_UNBOX_fixnum(en);
  return ss_s(strerror(i));
}


