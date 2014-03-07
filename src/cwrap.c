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

#define ss_B_C_void(V) ((V), ss_unspec)
#define ss_U_C_void(V) (void) (V)

#define BU_C(TYPE,NAME,B,U)                      \
  ss ss_B_C_##NAME(TYPE V) {                     \
    return B;                                    \
  }                                              \
  TYPE ss_U_C_##NAME(ss V) {                     \
    return (TYPE) U;                             \
  }

#define ITYPE(TYPE,NAME) BU_C(TYPE,NAME,ss_i(V),ss_fixnum_(V))
#define FTYPE(TYPE,NAME) BU_C(TYPE,NAME,ss_box_flonum(V),ss_flonum_(V))
#include "cintrinsics.def"

// void is special.
// WRAP_CT_PTR_UNDEFINED(void,void)
WRAP_CT(void*,voidP)

// C char* as scheme string.
BU_C(char*,charP,ss_s(V),ss_S(V))
WRAP_CT_ADDRESSABLE_0(char,char)
WRAP_CT_ADDRESSABLE_0(char*,charP)
WRAP_CT_PTR_0(char,char)
ss ss_A_C_char (ss self) {
  return BOX(charP,(char*) self);
}
WRAP_CT_PTR(char*,charP)

#define CTYPE(T,N)
#define ITYPE(T,N) WRAP_CT_INTRINSIC(T,N)
#define FTYPE(T,N) ITYPE(T,N)
#include "cintrinsics.def"

#define ITYPE(TYPE,NAME)                                        \
  ss ss_US_C_##NAME (ss x) { return BOX(NAME,*(TYPE*)x); }
#define FTYPE(T,N) ITYPE(T,N)
ITYPE(void*,voidP)
#include "cintrinsics.def"

#define ss_cstruct_def(ST,NAME,FILE,LINE)  WRAP_CT(ST NAME, ss_PASTE3(ST,_,NAME))
#define ss_cstruct_decl(ST,NAME,FILE,LINE) WRAP_CT_UNDEFINED(ST NAME, ss_PASTE3(ST,_,NAME))
#include "cwrap.def"

// ss requires no boxing.
BU_C(ss,ss,(V),(V))
WRAP_CT(ss*,ssP)

#undef BU_C

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

// ss_s_env* requires no boxing.
#define ss_B_C_struct_ss_s_envP(V) (V)
#define ss_U_C_struct_ss_s_envP(V) (V)

#define ss_cfunc_def(CT,MT,RT,NAME,NPARAM,PARAMS,SPARAMS,FILE,LINE) DEFINE_CF(CT,MT,RT,NAME,PARAMS);
#include "cwrap.def"

#undef ss_B_C_struct_ss_s_envP
#undef ss_U_C_struct_ss_s_envP

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

static
char *c_name(const char *cname)
{
  static char name[64];
  strcpy(name, cname);
  { char *s = name; while ( *s ) { *s = *s == ' ' ? '-' : *s; ++ s; } }
  return name;
}

void ss_init_ctypes(ss_s_env *ss_env)
{
  static struct {
    void *var; const char *name;
  } inits[] = {
#define ss_cintrinsic_def(CT,TN)                  \
    { &ss_t_C_##TN,              #CT      },      \
    { &ss_t_C_##TN##P,           #CT "*"  },      \
    { &ss_t_C_##TN##PP,          #CT "**" },
#define ITYPE(CT,TN) ss_cintrinsic_def(CT,TN)
#define FTYPE(CT,TN) ss_cintrinsic_def(CT,TN)
ss_cintrinsic_def(void*,voidP)
#include "cintrinsics.def"
#define ss_cstruct_decl(ST,SN,FILE,LINE)                                \
    { &ss_t_C_##ST##_##SN##P,   #ST "-" #SN "*"  },                     \
    { &ss_t_C_##ST##_##SN##PP,  #ST "-" #SN "**" },
#define ss_cstruct_def(ST,SN,FILE,LINE)                                 \
  ss_cstruct_decl(ST,SN,FILE,LINE)                                      \
    { &ss_t_C_##ST##_##SN,      #ST "-" #SN      },

#include "cwrap.def"
    { 0 }
  }, *d;
  for ( d = inits; d->var; ++ d ) {
    ss_s_type *t = * (ss*) d->var;
    ss_define(ss_env, ss_c_sym(c_name(d->name)), t);
    t->name = ss_sym_charP(ss_c_sym(d->name));
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
#define CT_P(TN,CS)                                                     \
    { F(ss_B0_C_##TN##P),              0, 0, 0, "new-" CS "*"        , 0, 0  }, \
    { F(ss_BV_C_##TN##P),              0, 0, 0, "new-" CS "*:"       , 2, 0  }, \
    { F(ss_D_C_##TN##P),               0, 0, 0,    "*" CS "*"        , 1, 0  }, \
    { F(ss_R_C_##TN##P),               0, 0, 0,        CS "*-ref"    , 2, 0  }, \
    { F(ss_S_C_##TN##P),               0, 0, 0,        CS "*-set!"   , 3, 0  },
#define ss_cintrinsic_def(CT,TN)                                        \
    { F(ss_B0_C_##TN),                 0, 0, 0, "new-" #CT           , 0, 0  }, \
    { F(ss_B1_C_##TN),                 0, 0, 0, "new-" #CT ":"       , 1, 0  }, \
    { F(ss_US_C_##TN),                 0, 0, 0,        #CT "->"      , 1, 0  }, \
    { F(ss_BS_C_##TN),                 0, 0, 0,        #CT "="       , 2, 0  }, \
    CT_P(TN,    #CT)                                                    \
    CT_P(TN##P, #CT "*")
#define ITYPE(CT,TN) ss_cintrinsic_def(CT,TN)
#define FTYPE(CT,TN) ss_cintrinsic_def(CT,TN)
    ITYPE(void*,voidP)
#include "cintrinsics.def"
#define ss_cstruct_def(ST,SN,FILE,LINE)                                 \
    { F(ss_B0_C_##ST##_##SN),          0, 0, 0, "new-" #ST "-" #SN     ,        0, 0 }, \
    { F(ss_A_C_##ST##_##SN),           0, 0, 0,        #ST "-" #SN "&" ,        1, 0 }, \
    CT_P(ST##_##SN,    #ST " " #SN)                                     \
    CT_P(ST##_##SN##P, #ST " " #SN "*")
#define ss_cstruct_decl(ST,SN,FILE,LINE)                                \
    { F(ss_B0_C_##ST##_##SN##P),       0, 0, 0, "new-" #ST "-" #SN "*" ,        0, 0 }, \
    { F(ss_B0_C_##ST##_##SN##PP),      0, 0, 0, "new-" #ST "-" #SN "**",        0, 0 }, \
    { F(ss_D_C_##ST##_##SN##PP),       0, 0, 0,    "*" #ST "-" #SN "**",        1, 0 },
#define ss_cstruct_element_def(ST,SN,CT,RT,MT,EN,BF,FILE,LINE)          \
    { F(ss_SR_C_##ST##_##SN##__##EN),  0, 0, 0,        #ST "-" #SN "." #EN    , 1, 0 }, \
    { F(ss_SS_C_##ST##_##SN##__##EN),  0, 0, 0,        #ST "-" #SN "." #EN "=", 2, 0 },
#define ss_cstruct_elemptr_def(ST,SN,CT,RT,MT,EN,BF,FILE,LINE)          \
    { F(ss_SP_C_##ST##_##SN##__##EN),  0, 0, 0,        #ST "-" #SN "." #EN "&", 1, 0 },
#define ss_cfunc_def(CT,MT,RT,NAME,NPARAMS,PARAMS,SPARAMS,FILE,LINE)    \
    { F(NAME), #NAME, F(ss_Cf_##NAME), #NAME, NPARAMS, #CT "(*)(" SPARAMS ")"},
#include "cwrap.def"
    { 0, }
  }, *d;
  ss_init_ctypes(ss_env);
  ss_init_cdefine(ss_env);
  for ( d = inits; d->cfunc; ++ d ) {
    char cname[64];
    if ( ! d->wfunc )  d->wfunc  = d->cfunc;
    if ( ! d->wfuncn ) d->wfuncn = d->cfuncn;
    if ( ! d->docstr ) d->docstr = d->wname;
    snprintf(cname, 63, "%%%s", d->cname ? d->cname : d->wname);
    ss_define_cfunc(ss_env, c_name(cname)   , d->cfunc, -1        , d->cfuncn, d->docstr);
    ss_define_cfunc(ss_env, c_name(d->wname), d->wfunc, d->nparams, d->wfuncn, d->docstr);
  }
}
#undef F
#endif
