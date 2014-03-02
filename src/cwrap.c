#if ss_cwrap_c

#include "cwrap.h"

#if 0
#define ss_cfunc_def(CT,MT,RT,NAME,NPARAM,PARAMS,SPARAMS,FILE,LINE) DECLARE_CF(CT,MT,RT,NAME,PARAMS);
#include "ctype.def"
#endif

#define ss_cfunc_def(CT,MT,RT,NAME,NPARAM,PARAMS,SPARAMS,FILE,LINE) DEFINE_CF(CT,MT,RT,NAME,PARAMS);
#include "ctype.def"

#endif
