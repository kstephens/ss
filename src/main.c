void ss_init_const(ss_s_env *ss_env)
{
  ss_immediate_types[(ss_word_t) ss_eos] = ss_t_eos;
}
void ss_init_global(ss_s_env *ss_env)
{
}
void ss_init_prim(ss_s_env *ss_env);
void ss_init_cwrap(ss_s_env *ss_env);

void ss_init_main(ss_s_env *ss_env, int argc, char **argv)
{
  char *r;
  ss_prog_path = ss_prog_name = argv[0];
  if ( (r = strrchr(ss_prog_name, '/')) ) {
    ss_prog_dir = malloc(r - ss_prog_name + 1);
    strncpy(ss_prog_dir, ss_prog_name, r - ss_prog_name);
    ss_prog_dir[r - ss_prog_name] = 0;
    ss_prog_name = r + 1;
  } else {
    ss_prog_dir = ".";
  }
  if ( ! (ss_lib_dir = getenv("ss_LIB_DIR")) ) {
    static char lib_dir[] = "/lib"; // FIXME bin/../lib
    ss_lib_dir = malloc(strlen(ss_prog_dir) + sizeof(lib_dir));
    strcat(strcpy(ss_lib_dir, ss_prog_dir), lib_dir);
  }

  {
    ss args = ss_vecn(argc);
    int i;
    for ( i = 0; i < argc; ++ i ) {
      ss_vector_V(args)[i] = ss_s(argv[i]);
    }
    ss_define(ss_env, ss_sym(ss_prog_args), args);
  }
  ss_define(ss_env, ss_sym(ss_prog_path), ss_s(ss_prog_path));
  ss_define(ss_env, ss_sym(ss_prog_dir), ss_s(ss_prog_dir));
  ss_define(ss_env, ss_sym(ss_prog_name), ss_s(ss_prog_name));
  ss_define(ss_env, ss_sym(ss_lib_dir), ss_s(ss_lib_dir));
}

int main(int argc, char **argv)
{
  ss_s_env *ss_env = 0;
  const char *file = 0;

  GC_INIT();
  // GC_register_displacement(sizeof(ss) * 2);
  ss_init_type(ss_env);
  ss_top_level_env = ss_current_env = ss_env = ss_m_env(0);
  ss_init_const(ss_env);
  ss_init_symbol(ss_env);
  ss_init_global(ss_env);
  ss_init_main(ss_env, argc, argv);
  ss_init_prim(ss_env);
  ss_init_cwrap(ss_env);
  ss_init_port(ss_env);

  if ( 1 ) {
    char fn[1024];
    snprintf(fn, 1023, "%s/%s", ss_lib_dir, "boot.scm");
    ss_load_file(ss_env, fn);
  }

  { int argi; for ( argi = 1; argi < argc; ++ argi ) {
    char *arg = argv[argi];
    ss_load_file(ss_env, file = arg);
  } }

  if ( ! file ) {
    ss_repl_run(ss_main_repl(ss_env));
  }

  return 0;
}
