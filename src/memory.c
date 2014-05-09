
ss_INLINE
ss* ss_m_cell(ss v)
{
  ss *c = ss_malloc(sizeof(*c));
  *c = v;
  return c;
}

ss ss_memmove(ss dst, ss src, ss size)
{
  memmove(dst, src, ss_I(size));
  return dst;
}

ss_INLINE int _ss_memcmpn(const void *s1, const void *s2, size_t n1, size_t n2)
{
  int cmp = memcmp(s1, s2, n1 < n2 ? n1 : n2);
  cmp = cmp < 0 ? -1 : (cmp > 0 ? 1 : (n1 < n2 ? -1 : n1 > n2 ? 1 : 0));
  return cmp;
}

ss ss_memcmp(ss a, ss b, ss as, ss bs)
{
  return ss_i(_ss_memcmpn(a, b, ss_I(as), ss_I(bs)));
}

ss_INLINE
ss ss_alloc(ss type, size_t size)
{
  ss *ptr = ss_malloc(sizeof(ss) + size);
  *(ptr ++) = type;
  if ( type && ! ss_fixnumQ(((ss_s_type*) type)->instance_size) )
    ((ss_s_type*) type)->instance_size = ss_i(size);
  return ptr;
}

ss_INLINE
ss ss_alloc_copy(ss type, size_t size, void *ptr)
{
  void *self = ss_alloc(type, size);
  memcpy(self, ptr, size);
  return self;
}

ss_INLINE
ss ss_set_type(ss type, ss obj)
{
  ((ss*) obj)[-1] = type;
  return obj;
}

