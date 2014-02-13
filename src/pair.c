static inline
ss ss_cons(ss a, ss d)
{
  ss_s_cons *self = ss_alloc(ss_t_pair, sizeof(*self));
  self->a = a;
  self->d = d;
  return self;
}

static inline
ss ss_car(ss a)
{
  ss_typecheck(ss_t_pair,a);
  return ss_CAR(a);
}

static inline
ss ss_cdr(ss a)
{
  ss_typecheck(ss_t_pair,a);
  return ss_CDR(a);
}

static inline
ss ss_set_carE(ss a, ss v)
{
  ss_typecheck(ss_t_pair,a);
  ss_CAR(a) = v;
  return a;
}

static inline
ss ss_set_cdrE(ss a, ss v)
{
  ss_typecheck(ss_t_pair,a);
  ss_CDR(a) = v;
  return a;
}
