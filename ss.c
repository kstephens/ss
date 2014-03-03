#include "ss.h"

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

static inline
ss ss_typecheck_error(ss t, ss v)
{
  return ss_error(ss_current_env, "typecheck", t, "given %s", ss_type_(v)->name);
}
static inline
ss ss_typecheck(ss t, ss v)
{
  if ( ss_type(v) != t )
    return ss_typecheck_error(t, v);
  return v;
}

static inline
void _ss_min_args_error(ss_s_env *ss_env, ss op, const char *DOCSTRING, int ss_argc, int MINARGS)
{
  ss_error(ss_env, "not-enough-args", op, "(%s) given %d expected at least %d", DOCSTRING, ss_argc, MINARGS);
}

static inline
void _ss_max_args_error(ss_s_env *ss_env, ss op, const char *DOCSTRING, int ss_argc, int MAXARGS)
{
  ss_error(ss_env, "too-many-args", op, "(%s) given %d expected %d", DOCSTRING, ss_argc, MAXARGS);
}

#include "src/memory.c"
#include "src/type.c"
#include "src/slot.c"
#include "src/catch.c"
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
#include "src/hash.c"
#include "src/repl.c"

#include "src/main.c"

#include "src/read.c"
#include "src/cfunc.c"
#include "src/prim.c"
#include "src/cwrap.c"
