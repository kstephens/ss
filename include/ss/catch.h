#ifndef _ss_catch_h
#define _ss_catch_h

#include <setjmp.h>
#include <assert.h>

typedef struct ss_s_catch {
  void *apply; // apply
  ss data[4];
  jmp_buf *jmp;
  ss value;
  struct ss_s_catch *prev, *dst, *src;
  ss body, rescue, ensure;
  struct ss_s_env *env;
  const char *file, *funcname;
  ss_fixnum_t line, level;
  ss expr;
  unsigned jmp_to : 2, in_begin : 1, in_body : 1, in_rescue : 1, in_ensure : 1, in_end : 1;
#define ss_CATCH_PARAMS ss_s_env *ss_env, ss_s_catch *catch, const char *file, int line, const char *funcname
#define ss_CATCH_ARGS ss_env, _catch, __FILE__, __LINE__, __FUNCTION__
#define ss_CATCH_SET_INFO() catch->file = file; catch->line = line; catch->funcname = funcname
} ss_s_catch;

#ifndef ss_CATCH_DEBUG
#define ss_CATCH_DEBUG 0
#endif

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
  ss_CATCH_SET_INFO();
  catch->in_begin = 1;
  catch->env = ss_env;
  catch->prev = ss_env->catch;
  catch->level = catch->prev ? catch->prev->level + 1 : 0;
  catch->jmp_to = 0;
  catch->jmp = 0;
  catch->in_body = catch->in_rescue = catch->in_ensure = catch->in_end = 0;
}

#define ss_CATCH(C) do {                                                \
  ss_s_catch *_catch = (C);                                             \
  jmp_buf _catch_jmp;                                                   \
  _ss_catch_in_begin(ss_CATCH_ARGS);                                    \
_catch_again:                                                           \
 switch ( _catch->jmp_to ? _catch->jmp_to : setjmp(*(_catch->jmp = &_catch_jmp)) ) { \
 case 0: _ss_catch_in_body(ss_CATCH_ARGS);
static inline
void _ss_catch_in_body(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  catch->in_body = 1;
  catch->env = ss_env;
  catch->prev = ss_env->catch;
  ss_env->catch = catch;
  catch->dst = catch->src = 0;
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "    %3d BODY    %p prev %p env %p env->catch %p\n", (int) catch->level, catch, catch->prev, ss_env, ss_env->catch);
}

#define ss_CATCH_RESCUE                                                 \
  break; case 1: _ss_catch_in_rescue(ss_CATCH_ARGS);
static inline
void _ss_catch_in_rescue(ss_CATCH_PARAMS)
{
  catch->in_rescue = 1;
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "    %3d RESCUE  %p prev %p env %p env->catch %p\n", (int) catch->level, catch, catch->prev, ss_env, ss_env->catch);
}

#define ss_CATCH_ENSURE                                 \
  break; case 2: default: _ss_catch_in_ensure(ss_CATCH_ARGS);
static inline
void _ss_catch_in_ensure(ss_CATCH_PARAMS)
{
  catch->in_ensure = 1;
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "    %3d ENSURE  %p prev %p env %p env->catch %p\n", (int) catch->level, catch, catch->prev, ss_env, ss_env->catch);
}

#define ss_CATCH_END                                             \
  break;                                                         \
  }                                                              \
  if ( _ss_catch_in_end(ss_CATCH_ARGS) ) goto _catch_again;      \
} while (0)
static inline
int _ss_catch_in_end(ss_CATCH_PARAMS)
{
  catch->in_end = 1;
  assert(ss_env->catch == catch);

  // if the ENSURE block was not run, jump back to it.
  if ( ! catch->in_ensure ) {
    catch->in_ensure = 1;
    catch->jmp_to = 2;
    return 1;
  }
  // Continue raising if we haven't rescued or reached our dst.
  if ( catch->dst && catch->dst != ss_env->catch ) {
    ss_s_catch *dst = catch->prev;
    if ( ss_CATCH_DEBUG )
      fprintf(stderr, "    %3d RETHROW %p prev %p dst %p env %p env->catch %p\n", (int) catch->level, catch, catch->prev, dst, ss_env, ss_env->catch);
    if ( ! dst ) {
      ss_error(ss_env, "no-catch", catch, 0);
      return 0;
    }
    dst->dst = catch->dst;
    dst->src = catch->src;
    _ss_longjmp(ss_env, catch, dst, 1);
  }
  // We are done.
  catch->in_begin = catch->in_rescue = catch->in_ensure = catch->in_end = 0;
  // Restore previous catch.
  ss_env->catch = catch->prev;
  return 0;
}

static inline
ss __ss_throw(ss_s_env *ss_env, ss_s_catch *catch, ss value)
{
  ss_s_catch *cur, *dst; int sig;
  assert(catch);
  catch->value = value;

  cur = ss_env->catch;
  if ( ! cur )
    return ss_error(ss_env, "no-active-catch", catch, 0);
  assert(ss_env->catch);

  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "   %3d THROW   %p prev %p env %p env->catch %p\n", (int) catch->level, catch, catch->prev, ss_env, ss_env->catch);

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
  dst->dst = dst->src = catch;
  _ss_longjmp(ss_env, catch, dst, sig);
  return 0;
}

static inline
ss _ss_throw_INFO(ss value, ss_CATCH_PARAMS)
{
  assert(catch);
  ss_CATCH_SET_INFO();
  return __ss_throw(ss_env, catch, value);
}

#define ss_throw_INFO(ENV,CATCH,VALUE) _ss_throw_INFO(VALUE,ENV,CATCH,__FILE__,__LINE__,__FUNCTION__)
#define ss_throw(ENV,CATCH,VALUE)   ss_throw_INFO(ENV,CATCH,VALUE)

static inline
ss ss_rethrow(ss_s_env *ss_env)
{
  ss_s_catch *catch, *dst;

  if ( ! (catch = ss_env->catch) )
    return ss_error(ss_env, "no-active-catch", ss_undef, 0);
  catch = ss_env->catch;
  if ( ! catch->dst )
    return ss_error(ss_env, "no-catch-dst", catch, 0);
  assert(catch->dst);
  dst = catch->dst;
  if ( ss_CATCH_DEBUG )
    fprintf(stderr, "    %3d RETHROW %p prev %p dst %p env %p env->catch %p\n", (int) catch->level, catch, catch->prev, dst, ss_env, ss_env->catch);
  __ss_throw(ss_env, dst, dst->value);
  return 0;
}

#endif
