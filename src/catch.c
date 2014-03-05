
ss_prim(ss_apply_throwable,1,2,0,"throwable catch")
{
  ss throwable = ss_prim;
  ss catch = 0;
  ss_typecheck(ss_t_throwable, throwable);
  if ( ss_argc >= 1 ) {
    catch = ss_argv[0];
    ss_typecheck(ss_t_catch, catch);
  }
  ss_return(ss_throw(ss_env, catch, throwable));
}
ss_end

ss ss_m_throwable(ss value)
{
  ss_s_throwable *self = ss_alloc(ss_t_throwable, sizeof(*self));
  memset(self, 0, sizeof(*self));
  self->data.prim = _ss_pf_ss_apply_throwable;
  self->data.value = value;
  return self;
}

ss ss_throwable_value(ss_s_throwable *thrown)
{
  return thrown->data.value;
}

ss_prim(ss_apply_catch,1,1,0,"catch value")
  ss catch = ss_prim;
  ss_typecheck(ss_t_catch, catch);
  ss_return(ss_throw(ss_env, catch, ss_m_throwable(ss_argv[0])));
ss_end

ss ss_m_catch(ss value)
{
  ss_s_catch *self = ss_alloc(ss_t_catch, sizeof(*self));
  memset(self, 0, sizeof(*self));
  self->data.prim = _ss_pf_ss_apply_catch;
  self->body = self->rescue = self->ensure = ss_undef;
  self->data.value = value;
  return self;
}

ss ss_m_catch_value(ss_s_catch *catch)
{
  if ( catch->thrown )
    return ss_throwable_value(catch->thrown);
  return ss_undef;
}

ss ss_catch(ss_s_env *ss_env, ss body, ss rescue, ss ensure)
{
  ss_s_catch *c = ss_m_catch(ss_undef);
  ss rtn = ss_undef;
  c->body = body;
  c->rescue = rescue;
  c->ensure = ensure;
  ss_CATCH(c) {
    rtn = ss_apply(ss_env, c->body, ss_vec1(c));
  }
  ss_CATCH_RESCUE {
    if ( c->rescue != ss_f && c->thrown ) {
      rtn = ss_apply(ss_env, c->rescue, ss_vec2(c, c->thrown->data.value));
    } else {
      ss_rethrow(ss_env);
    }
  }
  ss_CATCH_ENSURE {
    if ( c->ensure != ss_f )
      ss_apply(ss_env, c->ensure, ss_vec1(c));
  }
  ss_CATCH_END;
  return rtn;
}

ss ss_throw_ (ss_s_env *ss_env, ss _1, ss _2)
{
  ss_s_catch *catch = _1;
  ss_s_throwable *thrown = _2;
  assert(catch);
  assert(thrown);
  thrown->data.env = ss_env;
  thrown->data.expr = ss_env->expr;
  return __ss_throw(ss_env, catch, thrown);
}

