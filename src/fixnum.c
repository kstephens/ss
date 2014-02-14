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

ss ss_box_flonum(ss_flonum_t v)
{
  ss_s_flonum *self = ss_alloc(ss_t_flonum, sizeof(*self));
  self->value = v;
  return self;
}

ss_flonum_t ss_unb_flonum(ss v)
{
  ss_typecheck(ss_t_flonum, v);
  return ss_UNB_flonum(v);
}

