
size_t ss_malloc_bytes, ss_malloc_objects;
#if 0
#undef ss_malloc
ss_INLINE
void* ss_malloc(size_t s)
{
  ss_malloc_bytes += s;
  ss_malloc_objects ++;
  return GC_malloc(s);
}
#endif

ss_INLINE
ss* _ss_m_cell(ss v)
{
  ss *c = ss_malloc(sizeof(*c));
  *c = v;
  return c;
}
#define ss_m_cell(v) *_ss_m_cell(v)

ss ss_memmove(ss dst, ss src, ss size)
{
  memmove(dst, src, ss_I(size));
  return dst;
}

ss ss_memcmp(ss a, ss b, ss as, ss bs)
{
  int cmp = memcmp(a, b, ss_I(as) < ss_I(bs) ? ss_I(as) : ss_I(bs));
  return ss_i(cmp ? (cmp < 0 ? -1 : 1) : (ss_I(as) == ss_I(bs) ? 0 : (ss_I(as) < ss_I(bs) ? -1 : 1)));
}

ss_INLINE
ss ss_alloc(ss type, size_t size)
{
  ss *ptr = ss_malloc(sizeof(ss) + size);
  *(ptr ++) = type;
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

