
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

  ll = strtoll(ss_string_V(s), &endp, radix);
  if ( ! *endp )
    return ss_box(fixnum, ll);
  d = strtod(ss_string_V(s), &endp);
  if ( ! *endp )
    return ss_box(flonum, d);
  return ss_f;
}

