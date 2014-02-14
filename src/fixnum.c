ss ss_box_fixnum(ss_fixnum_t v)
{
  return ss_BOX_fixnum(v);
}

ss_fixnum_t ss_unb_fixnum(ss v)
{
  ss_typecheck(ss_t_fixnum,v);
  return ss_UNB_fixnum(v);
}

ss_fixnum_t ss_fixnum_(ss v)
{
  switch ( ss_type_e(v) ) {
  case ss_t_flonum:  return ss_UNB_flonum(v);
  case ss_t_fixnum:  return ss_I(v);
  default:           ss_typecheck_error(v); return 0;
  }
}

ss ss_fixnum_to_string(ss v, ss radix /* ignored */)
{
  char buf[64];
  snprintf(buf, sizeof(buf) - 1, "%lld", (long long) ss_UNB(fixnum, v));
  return ss_s(buf);
}

