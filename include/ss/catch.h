#ifndef _ss_catch_h
#define _ss_catch_h

#include <setjmp.h>
#include <assert.h>

typedef struct ss_s_catch {
  jmp_buf *jmp;
  ss value;
  struct ss_s_catch *prev, *dst, *src;
  unsigned jmp_to : 2, in_begin : 1, in_body : 1, in_rescue : 1, in_ensure : 1, in_end : 1;
  int level;
  ss body, rescue, ensure;
  struct ss_s_env *env;
  const char *file, *func; int line;
#define ss_CATCH_SET_INFO() catch->file = file; catch->line = line; catch->func = func
#define ss_CATCH_PARAMS ss_s_env *ss_env, ss_s_catch *catch, const char *file, int line, const char *func
#define ss_CATCH_ARGS ss_env, _catch, __FILE__, __LINE__, __FUNCTION__
} ss_s_catch;

#ifndef ss_CATCH_DEBUG
#define ss_CATCH_DEBUG 0
#endif

static inline
void _ss_longjmp(ss_s_env *ss_env, ss_s_catch *catch, ss_s_catch *dst, int sig)
{
  jmp_buf *jmp;
  if ( ss_CATCH_DEBUG )
    fprintf(stderr, " longjmp(%d) %p prev %p dst %p env %p env->catch %p\n", sig, catch, catch->prev, dst, ss_env, ss_env->catch);
  assert(dst->jmp);
  jmp = dst->jmp;
  // dst->jmp = 0; // disable for ss_rethrow.
  ss_env->catch = dst;
  longjmp(*jmp, sig);
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
    fprintf(stderr, "    %3d BODY    %p prev %p env %p env->catch %p\n", catch->level, catch, catch->prev, ss_env, ss_env->catch);
}

#if 0
#define ss_CATCH_RESTORE_ENV() ss_env = _catch->env; ss_env->catch = _catch
#else
#define ss_CATCH_RESTORE_ENV()
#endif

#define ss_CATCH_RESCUE                                                 \
  break; case 1: ss_CATCH_RESTORE_ENV(); _ss_catch_in_rescue(ss_CATCH_ARGS);
static inline
void _ss_catch_in_rescue(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  catch->in_rescue = 1;
  assert(ss_env->catch == catch);
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "    %3d RESCUE  %p prev %p env %p env->catch %p\n", catch->level, catch, catch->prev, ss_env, ss_env->catch);
}

#define ss_CATCH_ENSURE                                 \
  break; case 2: default: ss_CATCH_RESTORE_ENV(); _ss_catch_in_ensure(ss_CATCH_ARGS);
static inline
void _ss_catch_in_ensure(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  catch->in_ensure = 1;
  assert(ss_env->catch == catch);
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "    %3d ENSURE  %p prev %p env %p env->catch %p\n", catch->level, catch, catch->prev, ss_env, ss_env->catch);
}

#define ss_CATCH_END                                             \
  break;                                                         \
  } ss_CATCH_RESTORE_ENV();                                      \
  if ( _ss_catch_in_end(ss_CATCH_ARGS) ) goto _catch_again;      \
} while (0)
static inline
int _ss_catch_in_end(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
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
      fprintf(stderr, "    %3d RETHROW %p prev %p dst %p env %p env->catch %p\n", catch->level, catch, catch->prev, dst, ss_env, ss_env->catch);
    dst->dst = catch->dst;
    dst->src = catch->src;
    _ss_longjmp(ss_env, catch, dst, 1);
  }
  // Restore previous catch.
  ss_env->catch = catch->prev;
  return 0;
}

static inline
ss ss_throw(ss_s_env *ss_env, ss_s_catch *catch, ss value)
{
  ss_s_catch *cur, *dst; int sig;
  assert(catch);
  assert(ss_env->catch);
  cur = ss_env->catch;
  catch->value = value;

  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "   %3d THROW   %p prev %p env %p env->catch %p\n", catch->level, catch, catch->prev, ss_env, ss_env->catch);

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
ss ss_rethrow(ss_s_env *ss_env)
{
  ss_s_catch *catch, *dst;
  assert(ss_env->catch);
  catch = ss_env->catch;
  assert(catch->dst);
  dst = catch->dst;
  if ( ss_CATCH_DEBUG )
    fprintf(stderr, "    %3d RETHROW %p prev %p dst %p env %p env->catch %p\n", catch->level, catch, catch->prev, dst, ss_env, ss_env->catch);
  ss_throw(ss_env, dst, dst->value);
  return 0;
}

#endif
