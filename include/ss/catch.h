#ifndef _ss_catch_h
#define _ss_catch_h

#include <setjmp.h>
#include <assert.h>

#ifndef ss_CATCH_DEBUG
#define ss_CATCH_DEBUG 0
#endif

typedef struct ss_s_catch_data {
  void *apply; // apply
  ss value;
  ss data[4];
  ss_s_env *env;
  ss expr;
  const char *file;
  ss_fixnum_t line;
  const char *funcname;
} ss_s_catch_data;

typedef struct ss_s_throwable {
  ss_s_catch_data data;
  struct ss_s_catch *src, *dst;
  unsigned
    value_of_catch    : 1, // if true, this throwable's value is the result value of the dst catch.
    cannot_be_rescued : 1, // if true, this throwable cannot be rescued.
    cannot_be_ensured : 1  // if true, this throwable cannot be ensured.
    ;
} ss_s_throwable;

typedef struct ss_s_catch {
  ss_s_catch_data data;
  jmp_buf *jmp;
  struct ss_s_catch *prev;
  struct ss_s_throwable *thrown;
  ss body, rescue, ensure;
  ss_fixnum_t level;
  unsigned
    jmp_to   : 2,
    in_begin : 1,
    in_body  : 1,
    in_rescue : 1,
    in_ensure : 1,
    in_end    : 1,
    no_rescue : 1, // if true, this catch will not run rescue block.
    no_ensure : 1  // if true, this catch will not run ensure block.
  ;
#define ss_CATCH_PARAMS ss_s_env *ss_env, ss_s_catch *catch, const char *file, int line, const char *funcname
#define ss_CATCH_ARGS ss_env, _catch, __FILE__, __LINE__, __FUNCTION__
#define ss_CATCH_SET_INFO(X) (X)->data.env = ss_env; (X)->data.expr = ss_env->expr; (X)->data.file = (X)->data.file; (X)->data.line = line; (X)->data.funcname = funcname
} ss_s_catch;

static inline
ss _ss_longjmp(ss_s_env *ss_env, ss_s_catch *catch, ss_s_catch *dst, int sig)
{
  jmp_buf *jmp;
  if ( ss_CATCH_DEBUG )
    fprintf(stderr, " longjmp(%d) %p prev %p dst %p env %p env->catch %p\n", sig, catch, catch->prev, dst, ss_env, ss_env->catch);
  if ( ! dst )
    return ss_error(ss_env, "no-catch-dst", catch, 0);
  assert(dst->jmp);
  jmp = dst->jmp;
  // dst->jmp = 0; // disable for ss_rethrow.
  ss_env->catch = dst;
  longjmp(*jmp, sig);
  return 0;
}

static inline
void _ss_catch_in_begin(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO(catch);
  catch->in_begin = 1;
  catch->prev = ss_env->catch;
  catch->level = catch->prev ? catch->prev->level + 1 : 0;
  catch->jmp_to = 0;
  catch->jmp = 0;
  catch->thrown = 0;
  catch->in_body = catch->in_rescue = catch->in_ensure = catch->in_end = 0;
  catch->no_rescue = catch->no_ensure = 0;
}

#define ss_CATCH(C) do {                                                \
  ss_s_catch *_catch = (C);                                             \
  jmp_buf _catch_jmp;                                                   \
  _ss_catch_in_begin(ss_CATCH_ARGS);                                    \
_catch_again:                                                           \
 switch ( _catch->jmp_to ? _catch->jmp_to : setjmp(*(_catch->jmp = &_catch_jmp)) ) { \
 case 0: _ss_catch_in_body(ss_CATCH_ARGS); {
static inline
void _ss_catch_in_body(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO(catch);
  catch->in_body = 1;
  catch->prev = ss_env->catch;
  ss_env->catch = catch;
  catch->thrown = 0;
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "    %3d BODY    %p thrown %p prev %p env %p env->catch %p\n", (int) catch->level, catch, catch->thrown, catch->prev, ss_env, ss_env->catch);
}

#define ss_CATCH_RESCUE                                                 \
  } break; case 1: if ( _ss_catch_in_rescue(ss_CATCH_ARGS) ) {
static inline
int _ss_catch_in_rescue(ss_CATCH_PARAMS)
{
  catch->in_rescue = 1;
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "    %3d RESCUE  %p thrown %p prev %p env %p env->catch %p\n", (int) catch->level, catch, catch->thrown, catch->prev, ss_env, ss_env->catch);
  if ( catch->thrown && catch->thrown->cannot_be_rescued ) return 0;
  return ! catch->no_rescue;
}

#define ss_CATCH_ENSURE                                 \
  } break; case 2: default: if ( _ss_catch_in_ensure(ss_CATCH_ARGS) ) {
static inline
int _ss_catch_in_ensure(ss_CATCH_PARAMS)
{
  catch->in_ensure = 1;
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "    %3d ENSURE  %p thrown %p prev %p env %p env->catch %p\n", (int) catch->level, catch, catch->thrown, catch->prev, ss_env, ss_env->catch);
  if ( catch->thrown && catch->thrown->cannot_be_ensured ) return 0;
  return ! catch->no_ensure;
}

#define ss_CATCH_END                                             \
  } break;                                                       \
  }                                                              \
  if ( _ss_catch_in_end(ss_CATCH_ARGS) ) goto _catch_again;      \
} while (0)
static inline
int _ss_catch_in_end(ss_CATCH_PARAMS)
{
  catch->in_end = 1;
  assert(ss_env->catch == catch);

  // if the ENSURE block was not run, jump back to it.
  if ( ! catch->no_ensure && ! catch->in_ensure ) {
    catch->in_ensure = 1;
    catch->jmp_to = 2;
    return 1;
  }
  // Continue raising if we haven't rescued or reached our dst.
  if ( catch->thrown && catch->thrown->dst && catch->thrown->dst != ss_env->catch ) {
    ss_s_catch *dst = catch->prev;
    if ( ss_CATCH_DEBUG )
      fprintf(stderr, "    %3d RETHROW %p thrown %p prev %p dst %p env %p env->catch %p\n", (int) catch->level, catch, catch->thrown, catch->prev, dst, ss_env, ss_env->catch);
    if ( ! dst ) {
      ss_error(ss_env, "no-catch", catch, 0);
      return 0;
    }
    dst->thrown = catch->thrown;
    _ss_longjmp(ss_env, catch, dst, 1);
  }
  // We are done.
  catch->in_begin = catch->in_rescue = catch->in_ensure = catch->in_end = 0;
  // Restore previous catch.
  ss_env->catch = catch->prev;
  return 0;
}

static inline
ss __ss_throw(ss_s_env *ss_env, ss_s_catch *catch, ss_s_throwable *thrown)
{
  ss_s_catch *cur, *dst; int sig;
  assert(ss_env);
  assert(thrown);
  assert(catch);

  cur = ss_env->catch;
  thrown->src = cur;
  thrown->dst = catch;
  if ( ! cur )
    return ss_error(ss_env, "no-active-catch", thrown, 0);
  assert(ss_env->catch);

  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "   %3d THROW   %p thrown %p prev %p env %p env->catch %p\n", (int) catch->level, catch, thrown, catch->prev, ss_env, ss_env->catch);

  //   If THROW within current ENSURE,
  //     Jump to parent RESCUE or ENSURE.
  if ( cur->in_ensure ) {
    // fprintf(stderr, "      in_ensure\n");
    dst = cur->prev;
    sig = 1;
  } else
    // If THROW within current RESCUE,
    //   Jump to current ENSURE, then RERAISE after ENSURE.
  if ( cur->in_rescue ) {
    // fprintf(stderr, "      in_rescue\n");
    dst = cur;
    sig = 2;
  } else {
    // Jump to current RESCUE or ENSURE.
    // fprintf(stderr, "      in_OTHER\n");
    dst = cur;
    sig = 1;
  }
  dst->thrown = thrown;
  _ss_longjmp(ss_env, catch, dst, sig);
  return 0;
}

static inline
ss _ss_throw_INFO(ss_s_throwable *thrown, ss_CATCH_PARAMS)
{
  assert(thrown);
  ss_CATCH_SET_INFO(thrown);
  return __ss_throw(ss_env, catch, thrown);
}

#define ss_throw_INFO(ENV,CATCH,THROWABLE) _ss_throw_INFO(THROWABLE,ENV,CATCH,__FILE__,__LINE__,__FUNCTION__)
#define ss_throw(ENV,CATCH,THROWABLE)   ss_throw_INFO(ENV,CATCH,THROWABLE)

static inline
ss ss_rethrow(ss_s_env *ss_env)
{
  ss_s_catch *catch, *dst;

  if ( ! (catch = ss_env->catch) )
    return ss_error(ss_env, "no-active-catch", ss_undef, 0);
  catch = ss_env->catch;
  if ( ! catch->thrown )
    return ss_error(ss_env, "no-throwable", catch, 0);
  dst = catch->thrown->dst;
  if ( ss_CATCH_DEBUG )
    fprintf(stderr, "    %3d RETHROW %p thrown %p prev %p dst %p env %p env->catch %p\n", (int) catch->level, catch, catch->thrown, catch->prev, dst, ss_env, ss_env->catch);
  __ss_throw(ss_env, dst, catch->thrown);
  return 0;
}

#endif
