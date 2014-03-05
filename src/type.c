
ss_s_type* ss_ALIGNED(ss_te_to_t[64], 64);
#define ss_t_def(N) ss_s_type *ss_t_##N;
#include "t.def"
ss_s_type* ss_ALIGNED(ss_immediate_types[64], 64);
void ss_init_type(ss_s_env *ss_env)
{
  static struct {
    ss_s_type **var;
    const char *name;
  } inits[] = {
#define ss_t_def(N) { &ss_t_##N, #N },
#include "t.def"
    { 0 },
  };
  int i;
  for ( i = 0; inits[i].var; ++ i ) {
    *inits[i].var = ss_alloc(ss_t_type, sizeof(ss_s_type));
  }
  for ( i = 0; inits[i].var; ++ i ) {
    ss_s_type *t = *inits[i].var;
    ((ss*) t)[-1] = ss_t_type;
    t->name = inits[i].name;
  }
#define ss_te_def(N) \
  ss_te_to_t[ss_te_##N] = ss_t_##N; \
  ((ss_s_type *)ss_t_##N)->e = ss_te_##N;
#include "te.def"
  for ( i = 0; i < 64; ++ i ) ss_immediate_types[i] = ss_t_undef;
  ss_immediate_types[0] = ss_t_null;
  ss_immediate_types[2] = ss_t_undef;
  ss_immediate_types[4] = ss_t_unspec;
  ss_immediate_types[6] = ss_t_boolean;
  ss_immediate_types[8] = ss_t_boolean;
}

