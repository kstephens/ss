#ifndef _SS_WRAP_H
#define _SS_WRAP_H

#define ss_PASTE2_ARGS(_1,_2,ARGS) ss_PASTE2(_1,_2)ARGS
#define BOX(T,V)   ss_PASTE2_ARGS(ss_B_C_,T,(V))
#define UNBOX(T,V) ss_PASTE2_ARGS(ss_U_C_,T,(V))

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

// BOX/UNBOX primitives.
//
// Basic types:
// ss    BP(TYPE*) : box C value from pointer.
// ss    B (TYPE)  : box C value.
// ss    B0()      : box "zero" C value.
// ss    B1(ss)    : create boxed C from ss value.
// ss    BF(ss)    : forcefully box a C value from ss value.
// ss    BB(ss)    : forcefully box a C value from an ss value as if it was a dereferenced TYPE*.
// TYPE  U(ss)     : unbox boxed C value to C value.
// TYPE* UP(ss)    : C pointer to boxed C value.
// ss    US(ss)    : coerce boxed C value to ss value.
// ss    BS(ss)    : set boxed C value from ss value.
// ss    BV(ss count, ss value) : create vector of C value of length count with value.
//
// Pointer types:
// ss    R (ss ptr, ss i) : vector reference.
// ss    S (ss ptr, ss i, ss v) : set vector reference.
// ss    A (ss)     : boxed C pointer address of boxed C value.
// ss    D (ss ptr) : dereference pointer; same as R(ptr,0).

// Wrap undefined C type.
#define WRAP_CT_UNDEFINED_1(TYPE,NAME)                                  \
  TYPE* ss_PASTE2(ss_UP_C_,NAME) (ss self) {                            \
    return (TYPE*) self;                                                \
  }

#define WRAP_CT_STRUCT(TYPE,NAME)                                       \
  struct ss_PASTE2(ss_ts_,NAME) {                                       \
    TYPE value;                                                         \
    ss size;                                                            \
  };

// Wrap known C type.  Will not work with undefined struct/union.
#define WRAP_CT_ADDRESSABLE_0(TYPE,NAME)                                \
  WRAP_CT_UNDEFINED_1(TYPE,NAME);                                       \
  WRAP_CT_STRUCT(TYPE,NAME);                                            \
  ss   ss_PASTE2(ss_BP_C_,NAME) (TYPE *value) {                         \
    struct ss_PASTE2(ss_ts_,NAME) *self =                               \
      ss_alloc(ss_PASTE2(ss_t_C_,NAME), sizeof(*self));                 \
    self->value = *value;                                               \
    self->size = ss_i(1);                                               \
    return self;                                                        \
  }                                                                     \
  ss   ss_PASTE2(ss_B0_C_,NAME) () {                                    \
    static TYPE _zero;                                                  \
    return ss_PASTE2(ss_BP_C_,NAME)(&_zero);                            \
  }                                                                     \
  ss   ss_PASTE2(ss_B1_C_,NAME) (ss self) {                             \
    TYPE __value = UNBOX(NAME,self);                                    \
    return ss_PASTE2(ss_BP_C_,NAME)(&__value);                          \
  }                                                                     \
  ss   ss_PASTE2(ss_BF_C_,NAME) (ss self) {                             \
    return ss_PASTE2(ss_BP_C_,NAME)((void*) &self);                     \
  }                                                                     \
  ss   ss_PASTE2(ss_BS_C_,NAME) (ss self, ss value) {                   \
    *(TYPE*) self = UNBOX(NAME,value);                                  \
    return self;                                                        \
  }                                                                     \
  ss ss_PASTE2(ss_BB_C_,NAME) (ss self) {                               \
    return BOX(NAME,* (TYPE *) self);                                   \
  }

// Addressable with boxing primitive.
#define WRAP_CT_ADDRESSABLE(TYPE,NAME)                                  \
  ss   ss_PASTE2(ss_BP_C_,NAME) (TYPE *value);                          \
  ss   ss_PASTE2(ss_B_C_,NAME) (TYPE value) {                           \
    return ss_PASTE2(ss_BP_C_,NAME)(&value);                            \
  }                                                                     \
  TYPE ss_PASTE2(ss_U_C_,NAME) (ss self) {                              \
    return *(TYPE*) self;                                               \
  }                                                                     \
  WRAP_CT_ADDRESSABLE_0(TYPE,NAME)

// Pointer: may be to undefined type.
#define WRAP_CT_PTR_UNDEFINED(TYPE,NAME)                                \
  WRAP_CT_ADDRESSABLE(TYPE*,ss_PASTE2(NAME,P))                          \
  ss ss_PASTE2(ss_A_C_,NAME) (ss self) {                                \
    return BOX(ss_PASTE2(NAME,P),(TYPE*) self);                         \
  }

// Pointer to defined type.
#define WRAP_CT_PTR(TYPE,NAME)                                          \
  WRAP_CT_PTR_UNDEFINED(TYPE,NAME)                                      \
  WRAP_CT_PTR_0(TYPE,NAME)

#define WRAP_CT_PTR_0(TYPE,NAME)                                        \
  ss ss_PASTE3(ss_D_C_,NAME,P) (ss ptrobj) {                            \
    return BOX(NAME,** (TYPE**) ptrobj);                                \
  }                                                                     \
  ss ss_PASTE3(ss_R_C_,NAME,P) (ss self, ss i) {                        \
    return BOX(NAME, (*(TYPE**) self)[ss_I(i)]);                        \
  }                                                                     \
  ss ss_PASTE3(ss_S_C_,NAME,P) (ss self, ss i, ss value) {              \
    (*(TYPE**) self)[ss_I(i)] = UNBOX(NAME,value);                      \
    return self;                                                        \
  }                                                                     \
  ss ss_PASTE3(ss_BV_C_,NAME,P) (ss count, ss value) {                  \
    size_t i;                                                           \
    struct ss_PASTE3(ss_ts_,NAME,P) *self =                             \
      ss_alloc(ss_PASTE3(ss_t_C_,NAME,P), sizeof(*self));               \
    TYPE __value = UNBOX(NAME,value);                                   \
    self->value = ss_malloc(sizeof(self->value[0]) * ss_I(count));      \
    self->size = count;                                                 \
    for ( i = 0; i < ss_I(count); ++ i )                                \
      self->value[i] = __value;                                         \
    return self;                                                        \
  }

#define WRAP_CT(TYPE,NAME)                                              \
  WRAP_CT_ADDRESSABLE(TYPE,NAME)                                        \
  WRAP_CT_PTR(TYPE,NAME)                                                \
  WRAP_CT_PTR(TYPE*,ss_PASTE2(NAME,P))                                 

#define WRAP_CT_UNDEFINED(TYPE,NAME)                                    \
  WRAP_CT_UNDEFINED_1(TYPE,NAME);                                       \
  WRAP_CT_PTR_UNDEFINED(TYPE,NAME)                                      \
  WRAP_CT_PTR(TYPE*,ss_PASTE2(NAME,P))                                  \

#define WRAP_CT_INTRINSIC(TYPE,NAME)                                    \
  WRAP_CT_ADDRESSABLE_0(TYPE,NAME)                                      \
  WRAP_CT_PTR(TYPE,NAME)                                                \
  WRAP_CT_PTR(TYPE*,ss_PASTE2(NAME,P))

#endif
