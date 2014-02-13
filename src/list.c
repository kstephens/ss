
ss ss_listn(int n, ...)
{
  va_list vap;
  va_start(vap, n);
  ss l = ss_nil, *lp = &l;
  while ( n -- ) {
    *lp = ss_cons(va_arg(vap, ss), ss_nil);
    lp = &ss_CDR(*lp);
  }
  va_end(vap);
  return l;
}

ss ss_listnv(size_t n, const ss *v)
{
  ss l = ss_nil, *lp = &l;
  while ( n -- ) {
    *lp = ss_cons(*(v ++), ss_nil);
    lp = &ss_CDR(*lp);
  }
  return l;
}

size_t ss_list_length(ss x)
{
  size_t l = 0;
  
  again:
  switch ( ss_type_e(x) ) {
  case ss_t_pair:
    x = ss_CDR(x);
    l ++;
    goto again;
  case ss_t_null:    return l;
  case ss_t_vector:  return l + ss_vector_L(x);
  default:           return l + 1;
  }
}

ss ss_list_to_vector(ss x)
{
  size_t l = 0;
  ss v = ss_vecn(ss_list_length(x));
  again:
  switch ( ss_type_e(x) ) {
  case ss_t_pair:
    ss_vector_V(v)[l ++] = ss_CAR(x);
    x = ss_CDR(x);
    goto again;
  case ss_t_null:
    break;
  case ss_t_vector:
    memcpy(ss_vector_V(v) + l, ss_vector_V(x), sizeof(ss_vector_V(v)[0]) * ss_vector_L(x));
    break;
  default:
    ss_vector_V(v)[l] = ss_cons(ss_sym(_rest), x);
    break;
  }
  return v;
}
