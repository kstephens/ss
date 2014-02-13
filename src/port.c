
ss ss_read(ss_s_env *ss_env, ss port);

ss ss_port_close(ss port)
{
  ss_s_port *self = (ss_s_port*) port;
  if ( self->fp ) {
    fclose(self->fp);
    self->fp = 0;
  }
  return ss_undef;
}

void ss_s_port_finalize(void *port, void *arg)
{
  ss_s_port *self = (ss_s_port*) port;
  if ( self->fp ) {
    fprintf(stderr, "  ;; finalizing #@%p %s\n", self, ss_string_V(self->name));
  }
  ss_port_close(port);
}

ss ss_m_port(FILE *fp, const char *name, const char *mode)
{
  ss_s_port *self;
  if ( ! fp ) return ss_f;
  self = ss_alloc(ss_t_port, sizeof(*self));
  self->fp = fp;
  self->name = ss_s((void*) name);
  self->mode = ss_s((void*) mode);
  GC_register_finalizer(self, ss_s_port_finalize, 0, 0, 0);
  return self;
}

void ss_init_port(ss_s_env *ss_env)
{
#define P(NAME,MODE)                                    \
  ss_##NAME = ss_m_port(NAME, "<" #NAME ">", MODE);     \
  ss_define(ss_env, ss_sym(ss_##NAME), ss_m_global(ss_sym(ss_##NAME), &ss_##NAME));
  P(stdin, "r");
  P(stdout, "w");
  P(stderr, "w");
#undef P
}

ss_prim(ss_getc,1,1,0,"port")
{
  int c = getc(FP(ss_argv[0]));
  ss_return(ss_i(c));
}
ss_end

ss_prim(ss_ungetc,2,2,0,"port c")
{
  int c = ss_I(ss_argv[1]);
  ungetc(c, FP(ss_argv[0]));
}
ss_end

ss_prim(newline,0,1,0,"newline")
  FILE **out = ss_argc > 0 ? ss_argv[0] : ss_stdout;
  fprintf(*out, "\n");
ss_end


