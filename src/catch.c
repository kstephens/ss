
ss_prim(ss_apply_catch,1,1,0,"catch value")
{
  ss catch = ss_prim;
  ss_typecheck(ss_t_catch, catch);
  ss_return(ss_throw(ss_env, catch, ss_argv[0]));
}
ss_end

ss ss_m_catch()
{
  ss_s_catch *self = ss_alloc(ss_t_catch, sizeof(*self));
  memset(self, 0, sizeof(*self));
  self->apply = _ss_pf_ss_apply_catch;
  self->body = self->rescue = self->ensure = self->value = ss_undef;
  return self;
}

ss ss_catch(ss_s_env *ss_env, ss body, ss rescue, ss ensure)
{
  ss_s_catch *c = ss_m_catch();
  ss rtn = ss_undef;
  c->body = body;
  c->rescue = rescue;
  c->ensure = ensure;
  ss_CATCH(c) {
    rtn = ss_apply(ss_env, c->body, ss_vec1(c));
  }
  ss_CATCH_RESCUE {
    if ( c->rescue != ss_f )
      rtn = ss_apply(ss_env, c->rescue, ss_vec2(c->src, c->src->value));
  }
  ss_CATCH_ENSURE {
    if ( c->ensure != ss_f )
      ss_apply(ss_env, c->ensure, ss_vec1(c));
  }
  ss_CATCH_END;
  return rtn;
}

