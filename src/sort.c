#ifdef __APPLE__
int ss_sort_cmp(void *thunk, const void *a, const void *b)
#else
int ss_sort_cmp(const void *a, const void *b, void *thunk)
#endif
{
  ss ss_env = ((ss*)thunk)[0];
  ss func   = ((ss*)thunk)[1];
  ss ab[] = { *(ss*)a, *(ss*)b };
  ss args[] = { (ss) ss_t_vector, ab, (ss) 2 };
  return ss_apply(ss_env, func, args + 1) == ss_f ? 1 : -1;
}

ss ss_sort(ss_s_env *ss_env, ss v, ss cmp)
{
  ss thunk[] = { ss_env, cmp };
#ifdef __APPLE__
  qsort_r(ss_vector_V(v), ss_vector_L(v), sizeof(ss), thunk, ss_sort_cmp);
#else
  qsort_r(ss_vector_V(v), ss_vector_L(v), sizeof(ss), ss_sort_cmp, thunk);
#endif
  return v;
}

