void ss_write_vec(size_t n, const ss *v, ss port)
{
  size_t i = 0;
  while ( i < n ) {
    ss_write(v[i], port);
    if ( ++ i < n )
      fprintf(FP(port), " ");
  }
}

ss ss_write(ss v, ss port)
{
  return ss_write_3(v, port, ss_sym(write));
}

ss ss_write_3(ss v, ss port, ss mode)
{
#define ss_write(v,p) ss_write_3(v, p, mode)
  FILE *out = FP(port);
  switch ( ss_type_te(v) ) {
  case ss_te_undef:   fprintf(out, "#<undef>"); break;
  case ss_te_fixnum:  fprintf(out, "%lld", (long long) ss_UNB(fixnum, v)); break;
  case ss_te_flonum:  ss_write_flonum(v, port); break;
  case ss_te_string:
    if ( mode == ss_sym(display) ) {
      fwrite(ss_string_V(v), ss_string_L(v), 1, out);
    } else {
      fprintf(out, "\"%s\"", ss_string_V(v));
    }
    break;
  case ss_te_char: {
    int c = ss_C(v);
    if ( mode == ss_sym(display) ) {
      fprintf(out, "%c", c);
    } else {
      c == -1 ? fprintf(out, "#<\\eof> ") : fprintf(out, "#\\%c", c);
    }
  }
    break;
  case ss_te_boolean: fprintf(out, "#%c",    v == ss_t ? 't' : 'f'); break;
  case ss_te_prim:    fprintf(out, "#<p %s %s >", ss_UNB(prim, v)->name, ss_UNB(prim, v)->docstring); break;
  case ss_te_symbol:
  case ss_te_keyword:
    if ( ss_UNB(symbol, v).name == ss_f ) {
      fprintf(out, "#<symbol #@%p>", v);
    } else {
      fprintf(out, "%s",   ss_string_V(ss_UNB(symbol, v).name));
    }
    break;
  case ss_te_if:
    {
      ss_s_if *self = v;
      fprintf(out, "(if ");
      ss_write(self->t, port);
      fprintf(out, " ");
      ss_write(self->a, port);
      fprintf(out, " ");
      ss_write(self->b, port);
      fprintf(out, ")");
    }
    break;
  case ss_te_var:
    fprintf(out, "#<v ");
    ss_write(ss_UNB(var, v).name, port);
    fprintf(out, " %d %d>", (int) ss_UNB(var, v).up, (int) ss_UNB(var, v).over);
    break;
  case ss_te_var_set:
    fprintf(out, "(set! ");
    ss_write(ss_UNB(var_set, v).var, port);
    fprintf(out, " ");
    ss_write(ss_UNB(var_set, v).expr, port);
    fprintf(out, ")");
    break;
  case ss_te_global:
    fprintf(out, "#<g ");
    ss_write(((ss_s_global*) v)->name, port);
    fprintf(out, " >");
    break;
  case ss_te_quote:   fprintf(out, "'"); ss_write(ss_UNB(quote, v), port); break;
  case ss_te_eos:     fprintf(out, "#<eos>"); break;
  case ss_te_null:    fprintf(out, "()"); break;
  case ss_te_lambda:
    fprintf(out, "(lambda ");
    ss_write(ss_UNB(lambda, v).formals, port);
    fprintf(out, " ");
    ss_write(ss_UNB(lambda, v).body, port);
    fprintf(out, ")");
    break;
  case ss_te_closure:
    // fprintf(out, "#<c #@%p E#@%p ", v, ss_UNB(closure, v).env);
    ss_write(ss_UNB(closure, v).lambda, port);
    // fprintf(out, ">");
    break;
  case ss_te_port:
    fprintf(out, "#<port ");
    ss_write(ss_UNB(port, v).name, port);
    fprintf(out, " ");
    ss_write(ss_UNB(port, v).mode, port);
    fprintf(out, ">");
    break;
  case ss_te_env:
    fprintf(out, "#<env #@%p %ld >", v, (long) ((ss_s_env*) v)->level);
    break;
  case ss_te_type:
    fprintf(out, "#<type #@%p %s >", v, ((ss_s_type*) v)->name);
    break;
  default:
    fprintf(out, "#<??? %s #@%p >", ss_type_(v)->name, (void*) v);
    break;
  case ss_te_pair:
    fprintf(out, "(");
    while ( v != ss_nil ) {
      switch ( ss_type_te(v) ) {
      case ss_te_pair:
        ss_write(ss_CAR(v), port);
        v = ss_CDR(v);
        if ( v != ss_nil )
          fprintf(out, " ");
        break;
      default:
        fprintf(out, ". ");
        ss_write(v, port);
        v = ss_nil;
        break;
      }
    }
    fprintf(out, ")");
    break;
  case ss_te_begin:
    fprintf(out, "(begin ");
    goto vector_body;
  case ss_te_app:
    fprintf(out, "(");
    goto vector_body;
  case ss_te_vector:
    fprintf(out, "#(");
  vector_body:
    ss_write_vec(ss_vector_L(v), ss_vector_V(v), port);
    fprintf(out, ")");
    break;
  }
#undef ss_write
  return ss_undef;
}
