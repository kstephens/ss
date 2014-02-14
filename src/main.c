void ss_init_const(ss_s_env *ss_env)
{
}
void ss_init_global(ss_s_env *ss_env)
{
}
void ss_init_prim(ss_s_env *ss_env);
void ss_init_cfunc(ss_s_env *ss_env);

int main(int argc, char **argv)
{
  ss_s_env *ss_env;
  GC_INIT();
  // GC_register_displacement(sizeof(ss) * 2);
  ss_top_level_env = ss_current_env = ss_env = ss_m_env(0);
  ss_init_const(ss_env);
  ss_init_symbol(ss_env);
  ss_init_port(ss_env);
  ss_init_global(ss_env);
  ss_init_prim(ss_env);
  ss_init_cfunc(ss_env);
  {
    ss args = ss_vecn(argc);
    for ( int i = 0; i < argc; ++ i ) {
      ss_vector_V(args)[i] = ss_s(argv[i]);
    }
    ss_define(ss_env, ss_sym(ss_main_args), args);
  }
  if ( 1 ) {
    FILE *fp = fopen("lib/boot.scm", "r");
    ss out = ss_f; // ss_stderr;
    ss_repl(ss_env, &fp, out, out, ss_f);
    fclose(fp);
  }
  {
    ss prompt = ss_stderr;
    ss input  = ss_stdin;
    ss output = ss_stdout;
    ss trap_errors = ss_t;
    if ( ! isatty(0) ) {
      prompt = trap_errors = ss_f;
    }
    ss_repl(ss_env, input, output, prompt, trap_errors);
  }
  return 0;
}
