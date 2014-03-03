#ifndef _SS_WRAP_H
#define _SS_WRAP_H

#define BOX(T,V)   ss_PASTE2(ss_B_C_,T)(V)
#define UNBOX(T,V) ss_PASTE2(ss_U_C_,T)(V)

#define PARAM_ctype(P) ctype_##P
#define ctype_PARAM(T,M,R,I,N) T
#define PARAM_mtype(P) mtype_##P
#define mtype_PARAM(T,M,R,I,N) M
#define PARAM_rtype(P) rtype_##P
#define rtype_PARAM(T,M,R,I,N) R
#define PARAM_i(P)     i_##P
#define i_PARAM(T,M,R,I,N)     I
#define PARAM_name(P)  name_##P
#define name_PARAM(T,M,R,I,N)  N

#define PARAMS_ss(PARAMS) PARAMS_ss_##PARAMS
#define PARAMS_ss_PARAMS_0()
#define PARAMS_ss_PARAMS_1(P1) ss PARAM_name(P1)
#define PARAMS_ss_PARAMS_2(P1,P2) ss PARAM_name(P1), ss PARAM_name(P2)
#define PARAMS_ss_PARAMS_3(P1,P2,P3) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3)
#define PARAMS_ss_PARAMS_4(P1,P2,P3,P4) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4)
#define PARAMS_ss_PARAMS_5(P1,P2,P3,P4,P5) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4), ss PARAM_name(P5)
#define PARAMS_ss_PARAMS_6(P1,P2,P3,P4,P5,P6) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4), ss PARAM_name(P5), ss PARAM_name(P6)
#define PARAMS_ss_PARAMS_7(P1,P2,P3,P4,P5,P6,P7) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4), ss PARAM_name(P5), ss PARAM_name(P6), ss PARAM_name(P7)
#define PARAMS_ss_PARAMS_8(P1,P2,P3,P4,P5,P6,P7,P8) ss PARAM_name(P1), ss PARAM_name(P2), ss PARAM_name(P3), ss PARAM_name(P4), ss PARAM_name(P5), ss PARAM_name(P6), ss PARAM_name(P7), ss PARAM_name(P8)

#define PARAM_param(P) ss_PASTE2(param_,PARAM_name(P))
#define PARAM_unbox(P) PARAM_ctype(P) PARAM_param(P) = UNBOX(PARAM_rtype(P),PARAM_name(P))
#define PARAMS_unbox(PARAMS) PARAMS_unbox_##PARAMS
#define PARAMS_unbox_PARAMS_0()
#define PARAMS_unbox_PARAMS_1(P1) PARAM_unbox(P1);
#define PARAMS_unbox_PARAMS_2(P1,P2) PARAM_unbox(P1); PARAM_unbox(P2);
#define PARAMS_unbox_PARAMS_3(P1,P2,P3) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3);
#define PARAMS_unbox_PARAMS_4(P1,P2,P3,P4) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4);
#define PARAMS_unbox_PARAMS_5(P1,P2,P3,P4,P5) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4); PARAM_unbox(P5);
#define PARAMS_unbox_PARAMS_6(P1,P2,P3,P4,P5,P6) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4); PARAM_unbox(P5); PARAM_unbox(P6);
#define PARAMS_unbox_PARAMS_7(P1,P2,P3,P4,P5,P6,P7) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4); PARAM_unbox(P5); PARAM_unbox(P6); PARAM_unbox(P7);
#define PARAMS_unbox_PARAMS_8(P1,P2,P3,P4,P5,P6,P7,P8) PARAM_unbox(P1); PARAM_unbox(P2); PARAM_unbox(P3); PARAM_unbox(P4); PARAM_unbox(P5); PARAM_unbox(P6); PARAM_unbox(P7); PARAM_unbox(P8);

#define PARAMS_params(PARAMS)PARAMS_params_##PARAMS
#define PARAMS_params_PARAMS_0()
#define PARAMS_params_PARAMS_1(P1) PARAM_param(P1)
#define PARAMS_params_PARAMS_2(P1,P2) PARAM_param(P1), PARAM_param(P2)
#define PARAMS_params_PARAMS_3(P1,P2,P3) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3)
#define PARAMS_params_PARAMS_4(P1,P2,P3,P4) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4)
#define PARAMS_params_PARAMS_5(P1,P2,P3,P4,P5) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4), PARAM_param(P5)
#define PARAMS_params_PARAMS_6(P1,P2,P3,P4,P5,P6) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4), PARAM_param(P5), PARAM_param(P6)
#define PARAMS_params_PARAMS_7(P1,P2,P3,P4,P5,P6,P7) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4), PARAM_param(P5), PARAM_param(P6), PARAM_param(P7)
#define PARAMS_params_PARAMS_8(P1,P2,P3,P4,P5,P6,P7,P8) PARAM_param(P1), PARAM_param(P2), PARAM_param(P3), PARAM_param(P4), PARAM_param(P5), PARAM_param(P6), PARAM_param(P7), PARAM_param(P8)

#define PARAMS_free(PARAMS)

#define ss_NAME_CF(CTYPE,MTYPE,RTYPE,NAME,PARAMS) ss_##NAME
#define DECLARE_CF(CTYPE,MTYPE,RTYPE,NAME,PARAMS) ss ss_Cf_##NAME (PARAMS_ss(PARAMS))
#define DEFINE_CF(CTYPE,MTYPE,RTYPE,NAME,PARAMS)        \
  DECLARE_CF(CTYPE,MTYPE,RTYPE,NAME,PARAMS) {           \
    ss __return;                                        \
    PARAMS_unbox(PARAMS);                               \
    __return = BOX(RTYPE,NAME (PARAMS_params(PARAMS))); \
    PARAMS_free(PARAMS);                                \
    return __return;                                    \
  }

#define WRAP_CT1(TYPE,NAME)                                             \
  struct ss_PASTE2(ss_ts_,NAME) {                                       \
    TYPE value;                                                         \
    ss size;                                                            \
  };                                                                    \
  ss   ss_PASTE2(ss_B_C_,NAME) (TYPE value) {                           \
    struct ss_PASTE2(ss_ts_,NAME) *self = ss_alloc(ss_PASTE2(ss_t_C_,NAME), sizeof(*self)); \
    self->value = value;                                                \
    return (ss) self;                                                   \
  }                                                                     \
  ss   ss_PASTE2(ss_B0_C_,NAME) () {                                    \
    static TYPE _zero;                                                  \
    return ss_PASTE2(ss_B_C_,NAME)(_zero);                              \
  }                                                                     \
  ss   ss_PASTE2(ss_BF_C_,NAME) (ss self) {                             \
    return ss_PASTE2(ss_B_C_,NAME) (*(TYPE*) &self);                    \
  }                                                                     \
  TYPE ss_PASTE2(ss_U_C_,NAME) (ss self) {                              \
    return ((struct ss_PASTE2(ss_ts_,NAME) *) self)->value;             \
  }

#define WRAP_CT(TYPE,NAME)                                              \
  WRAP_CT1(TYPE,NAME)                                                   \
  WRAP_CT1(TYPE*,ss_PASTE2(NAME,P))                                     \
  WRAP_CT1(TYPE**,ss_PASTE2(NAME,PP))                                   \
  ss ss_PASTE3(ss_B_C_,NAME,Pv) (ss count, ss value) {                  \
    size_t i;                                                           \
    struct ss_PASTE3(ss_ts_,NAME,P) *self =                             \
      ss_alloc(ss_PASTE3(ss_t_C_,NAME,P), sizeof(*self));               \
    self->value = ss_malloc(sizeof(self->value[0]) * ss_I(count));      \
    self->size = count;                                                 \
    for ( i = 0; i < ss_I(count); ++ i )                                \
      self->value[i] = UNBOX(NAME,value);                               \
    return self;                                                        \
  }                                                                     \
  ss ss_PASTE3(ss_R_C_,NAME,P) (ss self, ss i) {                        \
    return BOX(NAME, ((struct ss_PASTE3(ss_ts_,NAME,P) *) self)->value[ss_I(i)]); \
  }                                                                     \
  ss ss_PASTE3(ss_S_C_,NAME,P) (ss self, ss i, ss value) {              \
    ((struct ss_PASTE3(ss_ts_,NAME,P) *) self)->value[ss_I(i)] =        \
      UNBOX(NAME,value);                                                \
    return self;                                                        \
  }                                                                     \
  ss ss_PASTE2(ss_A_C_,NAME) (ss self) {                                \
    return BOX(ss_PASTE2(NAME,P), (&((struct ss_PASTE2(ss_ts_,NAME) *) self)->value)); \
  }                                                                     \

#endif
