#ifndef _ss_catch_h
#define _ss_catch_h

#include <setjmp.h>

typedef struct ss_s_catch {
  jmp_buf *jmp;
  ss value;
  struct ss_s_catch *prev, *dst, *src;
  unsigned jmp_to : 2, in_begin : 1, in_body : 1, in_rescue : 1, in_ensure : 1, in_end : 1;
  ss body, rescue, ensure;
  const char *file, *func; int line;
#define ss_CATCH_SET_INFO() catch->file = file; catch->line = line; catch->func = func
#define ss_CATCH_PARAMS ss_s_env *ss_env, ss_s_catch *catch, const char *file, int line, const char *func
#define ss_CATCH_ARGS ss_env, _catch, __FILE__, __LINE__, __FUNCTION__
} ss_s_catch;

#if 0
#define ss_CATCH_DEBUG(X) ({ \
      fprintf(stderr, "  %s:%d %s %p => %p\n", __FILE__, __LINE__, #X, catch, catch->dst); \
  X; })
#else
#define ss_CATCH_DEBUG(X) X
#endif

static inline
void _ss_catch_in_begin(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  ss_CATCH_DEBUG(catch->in_begin = 1);
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
  ss_CATCH_DEBUG(catch->in_body = 1);
  catch->file = file;
  catch->line = line;
  catch->func = func;
  ss_env->catch = catch;
  catch->dst = 0;
}

static inline
ss ss_throw(ss_s_env *ss_env, ss_s_catch *catch, ss value)
{
  ss_s_catch *dst; int sig;
  if ( ! catch ) abort();
  if ( ! ss_env->catch ) abort();
  catch->value = value;
  // If the dst is the current catch,
  if ( catch == ss_env->catch ) {
    // Jump to its RESCUE block.
    dst = catch;
    sig = 1;
  } else {
    // Jump to the current ENSURE block (or END).
    dst = ss_env->catch;
    sig = 2;
  }
  dst->dst = dst->src = catch;
  ss_CATCH_DEBUG(longjmp(*dst->jmp, sig));
  return 0;
}

#define ss_CATCH_RESCUE                                 \
  break; case 1: _ss_catch_in_rescue(ss_CATCH_ARGS);
static inline
void _ss_catch_in_rescue(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  ss_CATCH_DEBUG(catch->in_rescue = 1);
  ss_env->catch = catch->prev;
}

#define ss_CATCH_ENSURE                                 \
  break; case 2: _ss_catch_in_ensure(ss_CATCH_ARGS);
static inline
void _ss_catch_in_ensure(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  ss_CATCH_DEBUG(catch->in_ensure = 1);
  ss_env->catch = catch->prev;
}

#define ss_CATCH_END                                            \
  break; default: ;                                             \
  } if ( _ss_catch_in_end(ss_CATCH_ARGS) ) goto _catch_again;  \
} while (0)
static inline
int _ss_catch_in_end(ss_CATCH_PARAMS)
{
  ss_CATCH_SET_INFO();
  ss_CATCH_DEBUG(catch->in_end = 1);
  // if the ensure block was not run:
  // jump back to it, and restore previous catch.
  if ( ! catch->in_ensure ) {
    ss_CATCH_DEBUG(catch->in_ensure = 1);
    ss_CATCH_DEBUG(catch->jmp_to = 2);
    ss_env->catch = catch->prev;
    return 1;
  }
  // Jump to the ENSURE block if we haven't reached our dst.
  if ( catch->dst && catch != catch->dst ) {
    catch->prev->dst = catch->dst;
    catch->prev->src = catch->src;
    ss_CATCH_DEBUG(longjmp(*catch->prev->jmp, 2));
  }
  // Restore previous catch.
  ss_env->catch = catch->prev;
  return 0;
}

#endif
