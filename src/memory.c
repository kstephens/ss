
ss_e_type ss_ALIGNED(ss_immediate_types[64], 64) = {
  ss_t_null,   0,   // 0
  ss_t_undef,  0,   // 2
  ss_t_unspec, 0,   // 4
  ss_t_boolean, 0,  // 6
  ss_t_boolean, 0,  // 8
  0, 0,
};

size_t ss_malloc_bytes, ss_malloc_objects;
#if 0
#undef ss_malloc
static inline
void* ss_malloc(size_t s)
{
  ss_malloc_bytes += s;
  ss_malloc_objects ++;
  return GC_malloc(s);
}
#endif

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

ss ss_alloc(ss_e_type type, size_t size)
{
  void *ptr = ss_malloc(sizeof(ss) + size);
  // *((ss*) ptr) = 0;
  // ptr += sizeof(ss);
  *((ss*) ptr) = (ss) type;
  ptr += sizeof(ss);
  return ptr;
}

ss ss_alloc_copy(ss_e_type type, size_t size, void *ptr)
{
  void *self = ss_alloc(type, size);
  memcpy(self, ptr, size);
  return self;
}

ss ss_set_type(ss_e_type type, ss obj)
{
  ((ss*) obj)[-1] = (ss) type;
  return obj;
}

