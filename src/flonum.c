
ss_INLINE
ss ss_box_flonum(ss_flonum_t v)
{
  ss_s_flonum *self = ss_alloc(ss_t_flonum, sizeof(*self));
  self->value = v;
  return self;
}

ss_INLINE
ss_flonum_t ss_unb_flonum(ss v)
{
  ss_typecheck(ss_t_flonum, v);
  return ss_UNB_flonum(v);
}

ss_INLINE
ss_flonum_t ss_flonum_(ss v)
{
  switch ( ss_type_te(v) ) {
  case ss_te_flonum:  return ss_UNB_flonum(v);
  case ss_te_fixnum:  return ss_I(v);
  default:           ss_typecheck_error(ss_t_number, v); return 1.0 / 0.0;
  }
}

ss_INLINE
ss ss_to_flonum(ss x)
{
  switch ( ss_type_te(x)) {
  case ss_te_flonum:  return x;
  case ss_te_fixnum:  return ss_box(flonum, ss_I(x));
  default:            return ss_typecheck_error(ss_t_number, x);
  }
}

ss ss_r(ss v)
{
  ss x[2] = { 0, 0 };
  x[0] = v;
  return ss_box_flonum(*(ss_flonum_t*) &x);
}

ss ss_R(ss v)
{
  ss rtn[2] = { 0, 0 };
  *((ss_flonum_t*) &rtn) = ss_flonum_(v);
  return rtn[0];
}

void ss_minimal_double_str(double x, char *buf, size_t buflen)
{
  char format[8];
  int len = 1;
  double y;
  do {
    snprintf(format, 8, "%%.%dg", len);
    snprintf(buf, buflen, format, (double) x);
    y = strtod(buf, 0);
    len ++;
  } while ( y != x && buf[buflen - 1] == 0 );
  if ( ! (strchr(buf, 'e') || strchr(buf, '.')) ) {
    strcat(buf, ".0");
  }
}

ss ss_flonum_to_string(ss v, ss radix /* ignored */)
{
  char buf[64];
  ss_minimal_double_str(ss_unb(flonum, v), buf, 63);
  return ss_s(buf);
}

void ss_write_flonum(ss v, ss port)
{
  char buf[64];
  ss_minimal_double_str(ss_unb(flonum, v), buf, 63);
  fprintf(FP(port), "%s", buf);
}
