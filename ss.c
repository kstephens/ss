#include "ss.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h> /* memcpy(), strerror() */
#ifdef __APPLE__
#define _DONT_USE_CTYPE_INLINE_ 1
#define _ANSI_SOURCE 1
#endif
#include <ctype.h>
#include <unistd.h>
#include <assert.h>

ss_s_env *ss_top_level_env, *ss_current_env;
FILE **ss_stdin = &stdin, **ss_stdout = &stdout, **ss_stderr = &stderr;

ss ss_write(ss obj, ss port);
ss ss_write_3(ss v, ss port, ss mode);

ss _ss_eval(ss_s_env *ss_env, ss *_ss_expr, ss *ss_argv);
#define ss_expr (*_ss_expr)
#define ss_eval(X) _ss_eval(ss_env, &(X), 0)
#define ss_callv(P,ARGS) _ss_eval(ss_env, &(P), ARGS)
ss ss_apply(ss_s_env *ss_env, ss func, ss args);

static inline
ss ss_cons(ss a, ss d);

#include "src/memory.c"
#include "src/slot.c"
#include "src/error.c"
#include "src/fixnum.c"
#include "src/number.c"
#include "src/flonum.c"
#include "src/string.c"
#include "src/pair.c"
#include "src/list.c"
#include "src/symbol.c"
#include "src/vector.c"
#include "src/write.c"
#include "src/rewrite.c"
#include "src/env.c"
#include "src/eval.c"
#include "src/syntax.c"
#include "src/port.c"
#include "src/sort.c"
#include "src/repl.c"

#include "src/main.c"

#include "src/read.c"
#include "src/cfunc.c"
#include "src/prim.c"

