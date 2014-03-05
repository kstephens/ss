ss_INLINE
ss ss_get(ss o, ss i)
{
  return ((ss*) o)[ss_I(i)];
}
ss_INLINE
ss ss_set(ss o, ss i, ss v)
{
  return ((ss*) o)[ss_I(i)] = v;
}
ss_INLINE
ss ss_get_0(ss x) { return ((ss*)x)[0]; }
ss_INLINE
ss ss_get_1(ss x) { return ((ss*)x)[1]; }
ss_INLINE
ss ss_get_2(ss x) { return ((ss*)x)[2]; }

