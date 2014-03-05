void ss_init_const(ss_s_env *ss_env)
{
  ss_immediate_types[(ss_word_t) ss_eos] = ss_t_eos;
}
void ss_init_global(ss_s_env *ss_env)
{
}
void ss_init_prim(ss_s_env *ss_env);
void ss_init_cwrap(ss_s_env *ss_env);

ss ss_main_repl(ss_s_env *ss_env)
{
  ss input  = ss_stdin;
  ss output = ss_stdout;
  ss prompt = ss_stderr;
  ss trap_errors = ss_t;
  if ( ! isatty(0) ) {
    prompt = trap_errors = ss_f;
  }
  return ss_repl(ss_env, input, output, prompt, trap_errors);
}

ss ss_load_file(ss_s_env *ss_env, const char *filename)
{
  ss rtn;
  FILE *fh = 0;
  fh = fopen(filename, "r");
  fprintf(stderr, "  load-file %s FILE* %p\n", filename, fh);
  if ( ! fh )
    return ss_error(ss_env, "load-file", ss_s(filename), "cannot open");
  rtn = ss_repl(ss_env, &fh, ss_stderr, ss_stderr, ss_f);
  fclose(fh);
  return rtn;
}

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
  ss_init_port(ss_env);
  ss_init_global(ss_env);
  ss_init_main(ss_env, argc, argv);
  ss_init_prim(ss_env);
  ss_init_cwrap(ss_env);

  if ( 1 ) {
    char fn[1024];
    snprintf(fn, 1023, "%s/%s", ss_lib_dir, "boot.scm");
    FILE *fp = fopen(fn, "r");
    ss out = ss_f; // ss_stderr;
    ss_repl(ss_env, &fp, out, out, ss_f);
    fclose(fp);
  }

  { int argi; for ( argi = 1; argi < argc; ++ argi ) {
    char *arg = argv[argi];
    ss_load_file(ss_env, file = arg);
  } }

  if ( ! file ) {
    ss_main_repl(ss_env);
  }

  return 0;
}
