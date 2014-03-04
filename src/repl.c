ss ss_prompt(ss_s_env *ss_env, ss input, ss prompt)
{
  if ( ! feof(FP(input)) && prompt != ss_f )
    fprintf(FP(prompt), " #; ss> ");
  return ss_read(ss_env, input);
}

ss ss_repl(ss_s_env *ss_env, ss input, ss output, ss prompt, ss trap_error)
{
  ss expr, value = ss_undef;
  ss_s_catch *catch = ss_m_catch(ss_undef);
  while ( 1 ) {
    ss_CATCH(catch) {
      if ( trap_error != ss_f ) ss_env->error_catch = catch;
      if ( (expr = ss_prompt(ss_env, input, prompt)) == ss_eos ) goto stop;
      if ( 0 && prompt != ss_f ) {
        fprintf(*ss_stderr, "  ;; ss: read => "); ss_write(expr, ss_stderr); fprintf(*ss_stderr, "\n");
      }
      value = ss_undef;
      value = ss_eval(expr);
      if ( 0 && prompt != ss_f ) {
        fprintf(*ss_stderr, "  ;; ss: rewrite => "); ss_write(expr, ss_stderr); fprintf(*ss_stderr, "\n");
      }
      if ( value != ss_undef && value != ss_unspec ) {
        if ( output != ss_f ) {
          ss_write(value, ss_stdout); fprintf(*ss_stdout, "\n");
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

