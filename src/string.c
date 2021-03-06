
ss_fixnum_t ss_char_(ss x)
{
  switch ( ss_type_te(x) ) {
  case ss_te_char:   return ss_C(x);
  case ss_te_fixnum: return ss_I(x);
  default: ss_typecheck_error(ss_t_char, x); return -1;
  }
}

ss ss_strn(size_t l)
{
  ss_s_string *self = ss_alloc(ss_t_string, sizeof(*self));
  self->v = ss_malloc(sizeof(self->v[0]) * (l + 1));
  self->l = l;
  return self;
}

ss ss_strnv(size_t l, const char *v)
{
  ss self = ss_strn(l);
  if ( v )
    memcpy(ss_string_V(self), v, sizeof(ss_string_V(self)[0]) * l);
  ss_string_V(self)[l] = 0;
  return self;
}

ss ss_s(const char *p)
{
  return p ? ss_strnv(strlen(p), (void*) p) : ss_f;
}

ss ss_S(ss p)
{
  return ( p == ss_f || p == ss_nil ) ? 0 : ss_string_V(p);
}

ss ss_string_TO_number(ss s, int radix)
{
   long long ll;
   double d;
   char *endp;
   ss n;

  ll = strtoll(ss_string_V(s), &endp, radix);
  if ( ! *endp && ss_I(n = ss_i(ll)) == ll )
    return n;
  d = strtod(ss_string_V(s), &endp);
  if ( ! *endp )
    return ss_box(flonum, d);
  return ss_f;
}

