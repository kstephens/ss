#include "cwrap.h"

#if 0
#define ss_cfunc_def(CT,MT,RT,NAME,NPARAM,PARAMS,SPARAMS,FILE,LINE) DECLARE_CF(CT,MT,RT,NAME,PARAMS);
#include "cwrap.def"
#endif

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

#define ss_B_C_char(V) ss_i(V)
#define ss_U_C_char(V) (char) ss_fixnum_(V)

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

#define ss_cfunc_def(CT,MT,RT,NAME,NPARAM,PARAMS,SPARAMS,FILE,LINE) DEFINE_CF(CT,MT,RT,NAME,PARAMS);
#include "cwrap.def"

void ss_init_cwrap(ss_s_env *ss_env)
{
  static struct init {
    void *cfunc, *primfunc;
    int nparams;
    const char *fname, *docstr;
  } inits[] = {
#define ss_cfunc_def(CT,MT,RT,NAME,NPARAMS,PARAMS,SPARAMS,FILE,LINE) { &NAME, &ss_Cf_##NAME, NPARAMS, #NAME, #CT "(" SPARAMS ")"},
#include "cwrap.def"
#define ss_cstruct_def(ST,NAME,FILE,LINE)  { &ss_B0_C_##ST##_##NAME, 0, 0, "ss_B0_C_" #ST "_" #NAME, #ST " " #NAME },
#define ss_cstruct_decl(ST,NAME,FILE,LINE) { &ss_B0_C_##ST##_##NAME##P, 0, 0, "ss_B0_C_" #ST "_" #NAME "P", #ST " " #NAME "*" },
#include "cwrap.def"
    { 0, }
  };
  for ( int i = 0; inits[i].fname; ++ i ) {
    char buf[64];
    ss_define_cfunc(ss_env, 0, inits[i].primfunc, inits[i].nparams, inits[i].fname, inits[i].docstr);
    if ( inits[i].cfunc ) {
      snprintf(buf, 63, "%%%s", inits[i].fname);
      ss_define_cfunc(ss_env, buf, inits[i].cfunc, inits[i].nparams, inits[i].fname, inits[i].docstr);
    }
  }
}
#endif
