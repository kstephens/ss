#include "cwrap.h"

#define ITYPE(T,N) extern ss_s_type *ss_t_C_##N, *ss_t_C_##N##P, *ss_t_C_##N##PP;
#define FTYPE(T,N) ITYPE(T,N)
ITYPE(void*,voidP)
#include "cintrinsics.def"

#define ss_cstruct_def(ST,NAME,FILE,LINE) extern ss_s_type *ss_t_C_##ST##_##NAME, *ss_t_C_##ST##_##NAME##P, *ss_t_C_##ST##_##NAME##PP;
#define ss_cstruct_decl(ST,NAME,FILE,LINE) extern ss_s_type *ss_t_C_##ST##_##NAME##P, *ss_t_C_##ST##_##NAME##PP, *ss_t_C_##ST##_##NAME##PPP;
extern ss_s_type *ss_t_C_ssP, *ss_t_C_ssPP, *ss_t_C_ssPPP;
#include "cwrap.def"

#if ss_cwrap_c

#define ITYPE(T,N) WRAP_CT(T,N)
#define FTYPE(T,N) WRAP_CT(T,N)
ITYPE(void*,voidP)
#include "cintrinsics.def"

#define ss_B_C_void(V) ((V), ss_unspec)
#define ss_U_C_void(V) (void) (V)

#define ss_B_C_char(V) ss_c(V)
#define ss_U_C_char(V) (int) ss_char_(V)

#define ss_B_C_charP(V) ss_s(V)
#define ss_U_C_charP(V) ss_S(V)

#define ss_B_C_short(V) ss_i(V)
#define ss_U_C_short(V) (short) ss_fixnum_(V)

#define ss_B_C_int(V) ss_i(V)
#define ss_U_C_int(V) (int) ss_fixnum_(V)

#define ss_B_C_long(V) ss_i(V)
#define ss_U_C_long(V) (long) ss_fixnum_(V)

#define ss_B_C_long_long(V) ss_i(V)
#define ss_U_C_long_long(V) (long long) ss_fixnum_(V)

#define ss_B_C_unsigned_char(V) ss_i(V)
#define ss_U_C_unsigned_char(V) (unsigned char) ss_fixnum_(V)

#define ss_B_C_unsigned_short(V) ss_i(V)
#define ss_U_C_unsigned_short(V) (unsigned short) ss_fixnum_(V)

#define ss_B_C_unsigned_int(V) ss_i(V)
#define ss_U_C_unsigned_int(V) (unsigned int) ss_fixnum_(V)

#define ss_B_C_unsigned_long(V) ss_i(V)
#define ss_U_C_unsigned_long(V) (unsigned long) ss_fixnum_(V)

#define ss_B_C_unsigned_long_long(V) ss_i(V)
#define ss_U_C_unsigned_long_long(V) (unsigned long long) ss_fixnum_(V)

#define ss_B_C_float(V) ss_box_flonum(V)
#define ss_U_C_float(V) ss_flonum_(V)

#define ss_B_C_double(V) ss_box_flonum(V)
#define ss_U_C_double(V) ss_flonum_(V)

#define ss_B_C_long_double(V) ss_box_flonum(V)
#define ss_U_C_long_double(V) ss_flonum_(V)

#define ITYPE(TYPE,NAME)                                                \
  ss ss_US_C_##NAME (ss x)       { return BOX(NAME, *(TYPE*) x); }      \
  ss ss_RS_C_##NAME (ss x, ss v) { *(TYPE*) x = UNBOX(NAME, v); return x; } \
  ss ss_PASTE2(ss_B1_C_,NAME) (ss value) {                              \
    struct ss_PASTE2(ss_ts_,NAME) *self = ss_alloc(ss_PASTE2(ss_t_C_,NAME), sizeof(*self)); \
    self->value = UNBOX(NAME, value);                                   \
    return self;                                                        \
  }
#define FTYPE(T,N) ITYPE(T,N)
ITYPE(void*,voidP)
#include "cintrinsics.def"

#define ss_cstruct_def(ST,NAME,FILE,LINE)  WRAP_CT(ST NAME,  ss_PASTE3(ST,_,NAME))
#define ss_cstruct_decl(ST,NAME,FILE,LINE) WRAP_CT(ST NAME*, ss_PASTE4(ST,_,NAME,P))
WRAP_CT(ss*,ssP)
#include "cwrap.def"

#define ss_B_C_ss(V) (V)
#define ss_U_C_ss(V) (V)

#define ss_B_C_struct_ss_s_envP(V) (V)
#define ss_U_C_struct_ss_s_envP(V) (V)

#ifdef memcpy
#undef memcpy
#endif
#ifdef memset
#undef memset
#endif
#ifdef memmove
#undef memmove
#endif
#ifdef strcat
#undef strcat
#endif
#ifdef strcpy
#undef strcpy
#endif
#ifdef strncpy
#undef strncpy
#endif
#ifdef strncat
#undef strncat
#endif
#ifdef strncpy
#undef strncpy
#endif
#ifdef memccpy
#undef memccpy
#endif
#ifdef strlcat
#undef strlcat
#endif
#ifdef strlcpy
#undef strlcpy
#endif
#ifdef stpcpy
#undef stpcpy
#endif

#define ss_cstruct_element_def(ST,SN,CT,RT,MT,EN,BF,FILE,LINE)    \
  ss ss_SR_C_##ST##_##SN##__##EN(ss x) {                          \
    ST SN *ptr = x;                                               \
    return BOX(MT,ptr->EN);                                       \
  }                                                               \
  ss ss_SS_C_##ST##_##SN##__##EN(ss x, ss v) {                    \
    ST SN *ptr = x;                                               \
    ptr->EN = UNBOX(MT,v);                                        \
    return x;                                                     \
  }
#define ss_cstruct_elemptr_def(ST,SN,CT,RT,MT,EN,BF,FILE,LINE)  \
  ss ss_SP_C_##ST##_##SN##__##EN(ss x) {                        \
    ST SN *ptr = x;                                             \
    return BOX(MT##P,&ptr->EN);                                 \
  }
#include "cwrap.def"

#define ss_cfunc_def(CT,MT,RT,NAME,NPARAM,PARAMS,SPARAMS,FILE,LINE) DEFINE_CF(CT,MT,RT,NAME,PARAMS);
#include "cwrap.def"

void ss_init_cdefine(ss_s_env *ss_env)
{
  static struct {
    const char *n, *v, *rv;
  } inits[] = {
#define ss_cdefine_def(N,V,RV,EXPR,FILE,LINE) { N, V, RV, },
#include "cdefine.def"
    { 0 },
  }, *d;
  for ( d = inits; d->n; ++ d ) {
    if ( d->rv[0] == '"' && d->rv[1] && d->rv[strlen(d->rv) - 1] == '"' ) {
      ss_define(ss_env, ss_c_sym(d->n), ss_strnv(strlen(d->rv) - 2, d->rv + 1));
    } else {
      ss expr = ss_s(d->rv);
      ss num = ss_string_TO_number(expr, 0);
      if ( num != ss_f ) {
        ss_define(ss_env, ss_c_sym(d->n), num);
      }
    }
  }
}

void ss_init_cwrap(ss_s_env *ss_env)
{
  static struct {
    void *cfunc; const char *cfuncn, *cname; // raw cfunc
    void *wfunc; const char *wfuncn, *wname; // wrapped cfunc
    int nparams;
    const char *docstr;
  } inits[] = {
#undef F
#define F(NAME) &NAME, ss_STRINGTIZE(NAME)
#define ss_cintrinsic_def(CT,TN)                                        \
    { F(ss_B0_C_##TN),                 0, 0, 0,     #TN     , 0,     #TN      }, \
    { F(ss_B1_C_##TN),                 0, 0, 0,     #TN ":" , 1,     #TN ":"  }, \
    { F(ss_US_C_##TN),                 0, 0, 0,     #TN "->", 1,     #TN "->" }, \
    { F(ss_RS_C_##TN),                 0, 0, 0,     #TN "=" , 2,     #TN "="  }, \
    { F(ss_P_C_##TN),                  0, 0, 0,     #TN "&" , 1,     #TN "&"  }, \
    { F(ss_B0_C_##TN##P),              0, 0, 0,     #TN "*" , 0,     #TN "*"  }, \
    { F(ss_B0_C_##TN##PP),             0, 0, 0,     #TN "**", 0,     #TN "**" }, \
    { F(ss_D_C_##TN##P),               0, 0, 0, "*" #TN "*" , 1, "*" #TN "*"  }, \
    { F(ss_D_C_##TN##PP),              0, 0, 0, "*" #TN "**", 1, "*" #TN "**" },
#define ITYPE(CT,TN) ss_cintrinsic_def(CT,TN)
#define FTYPE(CT,TN) ss_cintrinsic_def(CT,TN)
    ITYPE(void*,voidP)
#include "cintrinsics.def"
#define ss_cstruct_def(ST,SN,FILE,LINE)                               \
    { F(ss_B0_C_##ST##_##SN),          0, 0, 0,     #ST "-" #SN     ,        0,     #ST " " #SN      }, \
    { F(ss_P_C_##ST##_##SN),           0, 0, 0,     #ST "-" #SN "&" ,        1,     #ST " " #SN "&"  }, \
    { F(ss_B0_C_##ST##_##SN##P),       0, 0, 0,     #ST "-" #SN "*" ,        0,     #ST "-" #SN "*"  }, \
    { F(ss_B0_C_##ST##_##SN##PP),      0, 0, 0,     #ST "-" #SN "**",        0,     #ST "-" #SN "**" }, \
    { F(ss_D_C_##ST##_##SN##P),        0, 0, 0, "*" #ST "-" #SN "*" ,        1, "*" #ST "-" #SN "*"  }, \
    { F(ss_D_C_##ST##_##SN##PP),       0, 0, 0, "*" #ST "-" #SN "**",        1, "*" #ST "-" #SN "**" },
#define ss_cstruct_decl(ST,SN,FILE,LINE)                              \
    { F(ss_B0_C_##ST##_##SN##P),       0, 0, 0,     #ST "-" #SN "*" ,        0,     #ST "-" #SN "*"  }, \
    { F(ss_B0_C_##ST##_##SN##PP),      0, 0, 0,     #ST "-" #SN "**",        0,     #ST "-" #SN "**" }, \
    { F(ss_D_C_##ST##_##SN##PP),       0, 0, 0, "*" #ST "-" #SN "**",        1, "*" #ST "-" #SN "**" },
#define ss_cstruct_element_def(ST,SN,CT,RT,MT,EN,BF,FILE,LINE)          \
    { F(ss_SR_C_##ST##_##SN##__##EN),  0, 0, 0,     #ST "-" #SN "." #EN    , 1,    #ST " " #SN "." #EN     }, \
    { F(ss_SS_C_##ST##_##SN##__##EN),  0, 0, 0,     #ST "-" #SN "." #EN "=", 2,    #ST " " #SN "." #EN "=" },
#define ss_cstruct_elemptr_def(ST,SN,CT,RT,MT,EN,BF,FILE,LINE)          \
    { F(ss_SP_C_##ST##_##SN##__##EN),  0, 0, 0,     #ST "-" #SN "." #EN "&", 1,    #ST " " #SN "." #EN "&" },
#define ss_cfunc_def(CT,MT,RT,NAME,NPARAMS,PARAMS,SPARAMS,FILE,LINE)    \
    { F(NAME), #NAME, F(ss_Cf_##NAME), #NAME, NPARAMS, #CT "(*)(" SPARAMS ")"},
#include "cwrap.def"
    { 0, }
  }, *d;
  ss_init_cdefine(ss_env);
  for ( d = inits; d->cfunc; ++ d ) {
    char cname[64];
    if ( ! d->wfunc )  d->wfunc  = d->cfunc;
    if ( ! d->wfuncn ) d->wfuncn = d->cfuncn;
    snprintf(cname, 63, "%%%s", d->cname ? d->cname : d->wname);
    ss_define_cfunc(ss_env, cname   , d->cfunc, -1        , d->cfuncn, d->docstr);
    ss_define_cfunc(ss_env, d->wname, d->wfunc, d->nparams, d->wfuncn, d->docstr);
  }
}
#undef F
#endif
