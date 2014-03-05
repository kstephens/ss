ss ss_box_quote(ss v)
{
  if ( ss_literalQ(v) && ss_type_te(v) != ss_te_quote ) {
    return(v);
  } else {
    ss_s_quote *self = ss_alloc(ss_t_quote, sizeof(*self));
    self->value = v;
    return self;
  }
}

#if ss_EVAL_TRACING
int ss_rewrite_verbose = 0;
int ss_eval_verbose = 0;
ss ss_set_eval_verbose(ss x)
{
  ss_eval_verbose = ss_unb(fixnum, x); return x;
}
ss ss_set_rewrite_verbose(ss x)
{
  ss_rewrite_verbose = ss_unb(fixnum, x); return x;
}
static inline
void _ss_rewrite_expr(ss *_ss_expr, ss X, const char *REASON, const char *func, int line)
{
  if ( ss_rewrite_verbose || ss_eval_verbose ) {
    fprintf(*ss_stderr, ";; rewrite: ");
    ss_write_3(ss_expr, ss_stderr, ss_sym(internal));
    fprintf(*ss_stderr, "\n;;      at: #@%p", _ss_expr);
    fprintf(*ss_stderr, "\n;;      in: %s line:%d", func, line);
    fprintf(*ss_stderr, "\n;;  reason: %s\n", (REASON));
  }
  ss_expr = X;
  if ( ss_rewrite_verbose || ss_eval_verbose ) {
    fprintf(*ss_stderr, ";;      as: ");
    ss_write_3(ss_expr, ss_stderr, ss_sym(internal);
    fprintf(*ss_stderr, "\n\n");
  }
}
#define ss_rewrite_expr(X,REASON) _ss_rewrite_expr(&ss_expr, (X), REASON, __FUNCTION__, __LINE__)
#else
#define ss_rewrite_verbose 0
#define ss_eval_verbose    0
ss ss_set_eval_verbose(ss x)    { return x; }
ss ss_set_rewrite_verbose(ss x) { return x; }
#define ss_rewrite_expr(X,REASON) (ss_expr = (X))
#endif

