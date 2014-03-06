
ss_prim(close_port,1,1,0,"port")
{
  ss_s_port *p = ss_argv[0];
  p->next_char = ss_f;
  ss_return(ss_apply(p->close, 1, p));
}
ss_end

ss_prim(read_char,1,1,0,"port")
{
  ss_s_port *p = ss_argv[0];
  ss b;
  if ( (b = p->next_char) == ss_f ) {
    b = ss_apply(p->read_char, 1, p);
  } else {
    p->next_char = ss_f;
  }
  ss_return(b);
}
ss_end

ss_prim(peek_char,1,1,0,"port")
{
  ss_s_port *p = ss_argv[0];
  if ( p->next_char == ss_f ) {
    ss_return(p->next_char = ss_apply(p->read_char, 1, p));
  } else {
    ss_return(ss_eos);
  }
}
ss_end

ss_prim(write_char,2,2,0,"port char")
{
  ss_s_port *p = ss_argv[0];
  ss_apply(p->write_char, 2, p, ss_argv[1]);
}
ss_end

ss_prim(read_chars,4,4,0,"port string offset count")
{
  ss_s_port *p = ss_argv[0];
  ss_return(ss_apply(p->read_chars, 4, p, ss_argv[1], ss_argv[2], ss_argv[3]));
}
ss_end

// (write_chars ss_stdout "1234" 0 4)
ss_prim(write_chars,4,4,0,"port string offset count")
{
  ss_s_port *p = ss_argv[0];
  ss_return(ss_apply(p->write_chars, 4, p, ss_argv[1], ss_argv[2], ss_argv[3]));
}
ss_end

ss_prim(ss_file_read_char,1,1,0,"port")
{
  int c = -1;
  if ( FP(ss_argv[0]) ) c = fgetc(FP(ss_argv[0]));
  ss_return(ss_c(c));
}
ss_end

ss_prim(ss_file_write_char,2,2,0,"port")
{
  int ok = fputc(ss_C(ss_argv[1]), FP(ss_argv[0]));
  ss_return(ss_b(ok));
}
ss_end

ss_prim(ss_file_read_chars,4,4,0,"port string offset count")
{
  int c = -1;
  if ( FP(ss_argv[0]) )
    c = fread(ss_string_V(ss_argv[1]) + ss_I(ss_argv[2]),
              sizeof(ss_string_V(ss_argv[1])[0]),
              ss_I(ss_argv[3]),
              FP(ss_argv[0]));
  ss_return(ss_i(c));
}
ss_end

ss_prim(ss_file_write_chars,4,4,0,"port string offset count")
{
  int c = -1;
  if ( FP(ss_argv[0]) )
    c = fwrite(ss_string_V(ss_argv[1]) + ss_I(ss_argv[2]),
               sizeof(ss_string_V(ss_argv[1])[0]),
               ss_I(ss_argv[3]),
               FP(ss_argv[0]));
  ss_return(ss_i(c));
}
ss_end

ss_prim(ss_file_close,1,1,0,"port")
{
  ss_s_port *self = ss_argv[0];
  if ( self->opaque0 ) {
    fclose(self->opaque0);
    self->opaque0 = 0;
  }
}
ss_end

ss ss_read(ss_s_env *ss_env, ss port);
ss_prim(read,0,1,0,"port")
{
  ss_return(ss_read(ss_env, ss_argc > 0 ? ss_argv[0] : ss_stdin));
}
ss_end

ss_prim(newline,0,1,0,"newline")
{
  ss out = ss_argc > 0 ? ss_argv[0] : ss_stdout;
  ss_a2_write_char(out, ss_c('\n'));
}
ss_end

ss ss_port_close(ss port)
{
  return ss_a1_close_port(port);
}

void ss_s_port_finalize(void *port, void *arg)
{
  ss_s_port *self = (ss_s_port*) port;
  if ( self->opaque0 ) {
    fprintf(stderr, "  ;; finalizing #@%p %s\n", self, ss_string_V(self->name));
  }
  ss_a1_close_port(port);
}

// Assumes opaque0 is a FILE*.
ss ss_m_port(ss opaque0, ss name, ss mode)
{
  ss_s_port *self;
  if ( ! opaque0 ) return ss_f;
  self = ss_alloc(ss_t_port, sizeof(*self));
  self->opaque0 = opaque0;
  self->name = name;
  self->mode = mode;
  self->next_char   = ss_f;
  self->read_char   = ss_p_ss_file_read_char;
  self->write_char  = ss_p_ss_file_write_char;
  self->read_chars  = ss_p_ss_file_read_chars;
  self->write_chars = ss_p_ss_file_write_chars;
  self->close       = ss_p_ss_file_close;
  GC_register_finalizer(self, ss_s_port_finalize, 0, 0, 0);
  return self;
}

void ss_init_port(ss_s_env *ss_env)
{
#define P(NAME,MODE)                                                    \
  ss_##NAME = ss_m_port(NAME, ss_s("<" #NAME ">"), ss_s(MODE));         \
  ss_define(ss_env, ss_sym(ss_##NAME), ss_m_global(ss_sym(ss_##NAME), &ss_##NAME));
  P(stdin, "r");
  P(stdout, "w");
  P(stderr, "w");
#undef P
  ss_define(ss_env, ss_sym(_eos), ss_eos);
}

