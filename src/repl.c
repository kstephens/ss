
ss ss_m_repl(ss_s_env *ss_env, ss input, ss output)
{
  ss_s_repl *self = ss_alloc(ss_t_repl, sizeof(*self));
  self->env = ss_env;
  self->input = input;
  self->output = output;
  self->prompt = ss_f;
  self->trap_errors = ss_f;
  self->echo_read = self->echo_rewrite = ss_f;
  return self;
}

ss ss_repl_prompt(ss repl)
{
  ss_s_repl *self = repl;
  if ( ! feof(FP(self->input)) && self->prompt != ss_f )
    fprintf(FP(self->prompt), "  #; ss> ");
  return ss_ea1_read(self->env, self->input);
}

ss ss_repl_run(ss repl)
{
  ss_s_repl *self = repl;
#define ss_env self->env
  ss expr, value = ss_undef;
  ss_s_catch *catch = ss_m_catch(ss_undef);
  while ( 1 ) {
    ss_CATCH(catch) {
      if ( self->trap_errors != ss_f ) ss_env->error_catch = catch;
      if ( (expr = ss_repl_prompt(self)) == ss_eos ) goto stop;
      if ( self->echo_read != ss_f ) {
        fprintf(*ss_stderr, "  ;; ss: read    => "); ss_write_3(expr, ss_stderr, ss_sym(internal)); fprintf(*ss_stderr, "\n");
      }
      value = ss_undef;
      value = ss_eval(expr);
      if ( self->echo_rewrite != ss_f ) {
        fprintf(*ss_stderr, "  ;; ss: rewrite => "); ss_write_3(expr, ss_stderr, ss_sym(internal)); fprintf(*ss_stderr, "\n");
      }
      if ( value != ss_undef && value != ss_unspec ) {
        if ( self->output != ss_f ) {
          ss_write_3(value, self->output, ss_sym(internal)); fprintf(FP(self->output), "\n");
        }
      }
    }
    ss_CATCH_RESCUE {
      fprintf(*ss_stderr, "  ;; ss: recovered from error: ");
      ss_write(catch->thrown->data.value, ss_stderr);
      fprintf(*ss_stderr, "\n");
    }
    ss_CATCH_ENSURE {
      // fprintf(*ss_stderr, "  ;; ss: ensure\n");
    }
    ss_CATCH_END;
  }
 stop:
  return value;
}
#undef ss_env

ss ss_main_repl(ss_s_env *ss_env)
{
  ss_s_repl *repl = ss_m_repl(ss_env, ss_stdin, ss_stdout);
  repl->prompt = ss_stderr;
  repl->trap_errors = ss_t;
  if ( ! isatty(0) ) {
    repl->prompt = repl->trap_errors = ss_f;
  }
  return repl;
}

ss ss_load_file(ss_s_env *ss_env, const char *filename)
{
  ss rtn;
  ss p = ss_m_port(fopen(filename, "r"), ss_s(filename), ss_s("r"));
  fprintf(stderr, "  ;; ss: load-file %s #@%p\n", filename, p);
  if ( p == ss_f )
    return ss_error(ss_env, "load-file", ss_s(filename), "cannot open");
  rtn = ss_repl_run(ss_m_repl(ss_env, p, ss_f));
  ss_port_close(p);
  return rtn;
}

