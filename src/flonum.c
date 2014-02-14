ss_flonum_t ss_flonum_(ss v)
{
  switch ( ss_type_e(v) ) {
  case ss_t_flonum:  return ss_UNB_flonum(v);
  case ss_t_fixnum:  return ss_I(v);
  default:           ss_typecheck_error(v); return 1.0 / 0.0;
  }
}

static inline
ss ss_to_flonum(ss x)
{
  switch ( ss_type_e(x)) {
  case ss_t_flonum:  return x;
  case ss_t_fixnum:  return ss_box(flonum, ss_I(x));
  default:           return ss_typecheck_error(x);
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

