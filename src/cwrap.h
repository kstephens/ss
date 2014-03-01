#ifndef _SS_WRAP_H
#define _SS_WRAP_H

#define ss_PASTE2(A,B)ss_PASTE2_(A,B)
#define ss_PASTE2_(A,B)A##B
#define ss_PASTE3(A,B,C)ss_PASTE3_(A,B,C)
#define ss_PASTE3_(A,B,C)A##B##C
#define ss_PASTE4(A,B,C,D)ss_PASTE4_(A,B,C,D)
#define ss_PASTE4_(A,B,C,D)A##B##C##D

#define ss_ss(V) (V)
#define ss_ss_(V) (V)

#define ss_ss_s_envP(V) (V)
#define ss_ss_s_envP_(V) (V)

#define ss_void_(V) (void) (V)
#define ss_void(V) ((V), ss_unspec)

#define ss_int_(V) ss_I(V)
#define ss_int(V)  ss_i(V)

#define ss_size_t_(V) ss_I(V)
#define ss_size_t(V)  ss_i(V)

#define ss_unsigned_int_(V) ss_I(V)
#define ss_unsigned_int(V)  ss_i(V)

#define UNBOX(T,V) ss_PASTE3(ss_,T,_)(V)
#define BOX(D,T,V) D = ss_PASTE2(ss_,T)(V)

#define PARAM_ctype(P) ctype_##P
#define ctype_PARAM(T,M,I,N) T
#define PARAM_mtype(P) mtype_##P
#define mtype_PARAM(T,M,I,N) M
#define PARAM_i(P)     i_##P
#define i_PARAM(T,M,I,N)     I
#define PARAM_name(P)  name_##P
#define name_PARAM(T,M,I,N)  N

#define PARAMS_ss(PARAMS) PARAMS_ss_##PARAMS
#define PARAMS_ss_PARAMS_0()
#define PARAMS_ss_PARAMS_1(P1) ss PARAM_name(P1)
#define PARAMS_ss_PARAMS_2(P1,P2) ss PARAM_name(P1), ss PARAM_name(P2)
#define PARAMS_ss_PARAMS_3(P1,P2,P3) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3)
#define PARAMS_ss_PARAMS_4(P1,P2,P3,P4) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4)
#define PARAMS_ss_PARAMS_5(P1,P2,P3,P4,P5) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4), ss PARAM_name(P5)
#define PARAMS_ss_PARAMS_6(P1,P2,P3,P4,P5,P6) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4), ss PARAM_name(P5), ss PARAM_name(P6)
#define PARAMS_ss_PARAMS_7(P1,P2,P3,P4,P5,P6,P7) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4), ss PARAM_name(P5), ss PARAM_name(P6), ss PARAM_name(P7)

#define PARAM_param(P) ss_PASTE2(param_,PARAM_name(P))
#define PARAM_unbox(P) PARAM_ctype(P) PARAM_param(P) = UNBOX(PARAM_mtype(P),PARAM_name(P))
#define PARAMS_unbox(PARAMS) PARAMS_unbox_##PARAMS
#define PARAMS_unbox_PARAMS_0()
#define PARAMS_unbox_PARAMS_1(P1) PARAM_unbox(P1);
#define PARAMS_unbox_PARAMS_2(P1,P2) PARAM_unbox(P1); PARAM_unbox(P2);
#define PARAMS_unbox_PARAMS_3(P1,P2,P3) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3);
#define PARAMS_unbox_PARAMS_4(P1,P2,P3,P4) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4);
#define PARAMS_unbox_PARAMS_5(P1,P2,P3,P4,P5) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4); PARAM_unbox(P5);
#define PARAMS_unbox_PARAMS_6(P1,P2,P3,P4,P5,P6) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4); PARAM_unbox(P5); PARAM_unbox(P6);
#define PARAMS_unbox_PARAMS_7(P1,P2,P3,P4,P5,P6,P7) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4); PARAM_unbox(P5); PARAM_unbox(P6); PARAM_unbox(P7);

#define PARAMS_params(PARAMS)PARAMS_params_##PARAMS
#define PARAMS_params_PARAMS_0()
#define PARAMS_params_PARAMS_1(P1) PARAM_param(P1)
#define PARAMS_params_PARAMS_2(P1,P2) PARAM_param(P1), PARAM_param(P2)
#define PARAMS_params_PARAMS_3(P1,P2,P3) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3)
#define PARAMS_params_PARAMS_4(P1,P2,P3,P4) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4)
#define PARAMS_params_PARAMS_5(P1,P2,P3,P4,P5) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4), PARAM_param(P5)
#define PARAMS_params_PARAMS_6(P1,P2,P3,P4,P5,P6) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4), PARAM_param(P5), PARAM_param(P6)
#define PARAMS_params_PARAMS_7(P1,P2,P3,P4,P5,P6,P7) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4), PARAM_param(P5), PARAM_param(P6), PARAM_param(P7)

#define PARAMS_free(PARAMS)

#define ss_NAME_CF(RTYPE,MTYPE,NAME,PARAMS) ss_##NAME
#define DECLARE_CF(RTYPE,MTYPE,NAME,PARAMS) ss ss_C_##NAME(PARAMS_ss(PARAMS))
#define DEFINE_CF(RTYPE,MTYPE,NAME,PARAMS)              \
  DECLARE_CF(RTYPE,MTYPE,NAME,PARAMS) {                 \
    ss __return;                                        \
    PARAMS_unbox(PARAMS);                               \
    BOX(__return,MTYPE,NAME (PARAMS_params(PARAMS)));   \
    PARAMS_free(PARAMS);                                \
    return __return;                                    \
  }

#define WRAP_CT1(TYPE,NAME)                                             \
  static ss ss_t_##NAME;                                                \
  struct ss_ts_##NAME {                                                 \
    TYPE value;                                                         \
    ss size;                                                            \
  };                                                                    \
  static ss ss_##NAME(TYPE value) {                                     \
    struct ss_ts_##NAME *self = ss_allocate(ss_t_##NAME, sizeof(*self)); \
    self->value = value;                                                \
    return (ss) self;                                                   \
  }                                                                     \
  static ss ss_##NAME##_FORCE(ss self) {                                \
    return ss_##NAME(*(TYPE*) &self);                                   \
  }                                                                     \
  static TYPE ss_##NAME##_(ss self) {                                   \
    return ((struct ss_ts_##NAME *) self)->value;                       \
  }

#define WRAP_CT(TYPE,NAME)                                              \
  WRAP_CT1(TYPE,NAME)                                                   \
  typedef TYPE *NAME##P;                                                \
  WRAP_CT1(TYPE##P,NAME##P)                                             \
  static ss ss_##NAME##Pv(ss count, ss value) {                         \
    size_t i;                                                           \
    struct ss_ts_##NAME##P *self = ss_allocate(ss_t_##NAME##P, sizeof(*self)); \
    self->value = GC_malloc(sizeof(self->value[0]) * ss_I(count));      \
    self->size = count;                                                 \
    for ( i = 0; i < ss_I(count); ++ i ) self->value[i] = ss_##NAME##_(value); \
    return self;                                                        \
  }                                                                     \
  static ss ss_##NAME##P_get(ss self, ss i) {                             \
    return ss_##NAME(((struct ss_ts_##NAME##P *) self)->value[ss_I(i)]); \
  }                                                                     \
  static ss ss_##NAME##P_setE(ss self, ss i, ss value) {                   \
    ((struct ss_ts_##NAME##P *) self)->value[ss_I(i)] =                 \
      ss_##NAME##_(value);                                              \
    return self;                                                        \
  }                                                                     \
  static ss ss_##NAME##_array(ss self) {                                    \
    return ss_##NAME##P(&((struct ss_ts_##NAME *) self)->value);        \
  }                                                                     \

#endif
