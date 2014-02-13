ss ss_vecn(size_t l)
{
  ss_s_vector *self = ss_alloc(ss_t_vector, sizeof(*self));
  self->v = ss_malloc(sizeof(self->v[0]) * l);
  self->l = l;
  return self;
}

ss ss_vecnv(size_t l, const ss *v)
{
  ss_s_vector *self = ss_vecn(l);
  memcpy(self->v, v, sizeof(self->v[0]) * l);
  return self;
}

ss ss_vec(int n, ...)
{
  ss x = ss_vecn(n);
  int i = 0;
  va_list vap;
  va_start(vap,n);
  while ( i < n )
    ss_vector_V(x)[i ++] = va_arg(vap, ss);
  va_end(vap);
  return x;
}
