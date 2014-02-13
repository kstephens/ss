#ifndef _ss_catch_h
#define _ss_catch_h

#include <setjmp.h>
#include <assert.h>

typedef struct ss_s_catch {
  jmp_buf *jmp;
  ss value;
  struct ss_s_catch *prev, *dst, *src;
  unsigned jmp_to : 2, in_begin : 1, in_body : 1, in_rescue : 1, in_ensure : 1, in_end : 1;
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
#define ss_CATCH_DEBUG_(X) ({ \
      if ( ss_CATCH_DEBUG ) fprintf(stderr, "  %s:%d %s : catch %p prev %p dst %p env %p env->catch %p\n", __FILE__, __LINE__, #X, catch, catch->prev, catch->dst, ss_env, ss_env->catch); \
  X; })

static inline
void _ss_catch_in_begin(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  ss_CATCH_DEBUG_(catch->in_begin = 1);
  catch->env = ss_env;
  catch->prev = ss_env->catch;
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
  ss_CATCH_DEBUG_(catch->in_body = 1);
  if ( ss_CATCH_DEBUG ) 
    fprintf(stderr, "  body catch %p prev %p env %p env->catch %p\n", catch, catch->prev, ss_env, ss_env->catch);
  ss_env->catch = catch;
  catch->env = ss_env;
  catch->dst = catch->src = 0;
}

static inline
void _ss_longjump(ss_s_env *ss_env, ss_s_catch *catch, ss_s_catch *dst)
{
  jmp_buf *jmp;
  if ( ss_CATCH_DEBUG )
    fprintf(stderr, "  longjmp %p prev %p dst %p env %p env->catch %p\n", catch, catch->prev, dst, ss_env, ss_env->catch);
  assert(dst->jmp);
  jmp = dst->jmp;
  dst->jmp = 0;
  ss_CATCH_DEBUG_(longjmp(*jmp, 1));
}

static inline
ss ss_throw(ss_s_env *ss_env, ss_s_catch *catch, ss value)
{
  assert(catch);
  assert(ss_env->catch);
  catch->value = value;
  // Jump to catch RESCUE (or ENSURE) block.
  catch->dst = catch->src = catch;
  ss_CATCH_DEBUG_(_ss_longjump(ss_env, catch, catch));
  return 0;
}

#if 0
#define ss_CATCH_RESTORE_ENV() ss_env = _catch->env; _catch->env = 0;
#else
#define ss_CATCH_RESTORE_ENV()
#endif

#define ss_CATCH_RESCUE                                                 \
  break; case 1: ss_CATCH_RESTORE_ENV(); _ss_catch_in_rescue(ss_CATCH_ARGS);
static inline
void _ss_catch_in_rescue(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  ss_CATCH_DEBUG_(catch->in_rescue = 1);
  ss_env->catch = catch->prev;
}

#define ss_CATCH_ENSURE                                 \
  break; case 2: default: ss_CATCH_RESTORE_ENV(); _ss_catch_in_ensure(ss_CATCH_ARGS);
static inline
void _ss_catch_in_ensure(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  ss_CATCH_DEBUG_(catch->in_ensure = 1);
  ss_env->catch = catch->prev;
}

#define ss_CATCH_END                                             \
  break;                                                         \
  } ss_CATCH_RESTORE_ENV();                                      \
  if ( _ss_catch_in_end(ss_CATCH_ARGS) ) goto _catch_again;      \
  ss_env->catch = _catch->prev;                                  \
} while (0)
static inline
int _ss_catch_in_end(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  ss_CATCH_DEBUG_(catch->in_end = 1);
  // if the ensure block was not run:
  // jump back to it, and restore previous catch.
  if ( ! catch->in_ensure ) {
    ss_CATCH_DEBUG_(catch->in_ensure = 1);
    ss_CATCH_DEBUG_(catch->jmp_to = 2);
    ss_env->catch = catch->prev;
    return 1;
  }
  // Continue raising if we haven't reached our dst.
  if ( catch->dst && catch != catch->dst ) {
    ss_s_catch *dst = catch->prev;
    fprintf(stderr, "  continue %p prev %p dst %p env %p env->catch %p\n", catch, catch->prev, dst, ss_env, ss_env->catch);
    dst->dst = catch->dst;
    dst->src = catch->src;
    ss_CATCH_DEBUG_(_ss_longjump(ss_env, catch, dst));
  }
  // Restore previous catch.
  ss_env->catch = catch->prev;
  return 0;
}

#endif
