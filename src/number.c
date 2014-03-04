static inline
void ss_number_coerce_2(ss *a0, ss *a1)
{
  switch ( ss_type_te(*a0) ) {
  case ss_te_fixnum:
    switch ( ss_type_te(*a1) ) {
    case ss_te_flonum:
      *a0 = ss_box(flonum, ss_UNB(fixnum, *a0));
      break;
    case ss_te_fixnum:
      break;
    default: ss_typecheck_error(ss_t_number, *a1);
    }
    break;
  case ss_te_flonum:
    switch ( ss_type_te(*a1) ) {
    case ss_te_fixnum:
      *a1 = ss_box(flonum, ss_UNB(fixnum, *a1));
      break;
    case ss_te_flonum: break;
    default: ss_typecheck_error(ss_t_number, *a1);
    }
    break;
  default: ss_typecheck_error(ss_t_number, *a1);
  }
}

ss_syntax(ADD,0,-1,1,"+ z...")
  switch ( ss_argc ) {
  case 0:  ss_return(ss_box(fixnum,0));
  case 1:  ss_return(ss_argv[0]);
  case 2:  ss_return(ss_vec(3, ss_sym(ss_ADD), ss_argv[0], ss_argv[1]));
  default: ss_return(ss_vec(3, ss_sym(ss_ADD), ss_argv[0],
                            ss_cons(ss_sym(ADD), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(SUB,1,-1,1,"- z...")
  switch ( ss_argc ) {
  case 1:  ss_return(ss_vec(2, ss_sym(ss_NEG), ss_argv[0]));
  case 2:  ss_return(ss_vec(3, ss_sym(ss_SUB), ss_argv[0], ss_argv[1]));
  default: ss_return(ss_vec(3, ss_sym(ss_SUB), ss_argv[0],
                            ss_cons(ss_sym(ADD), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(MUL,0,-1,1,"* z...")
  switch ( ss_argc ) {
  case 0:  ss_return(ss_box(fixnum,1));
  case 1:  ss_return(ss_argv[0]);
  case 2:  ss_return(ss_vec(3, ss_sym(ss_MUL), ss_argv[0], ss_argv[1]));
  default: ss_return(ss_vec(3, ss_sym(ss_MUL), ss_argv[0],
                            ss_cons(ss_sym(MUL), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

ss_syntax(DIV,1,-1,1,"/ z...")
  switch ( ss_argc ) {
  case 1:  ss_return(ss_vec(3, ss_sym(ss_DIV), ss_box(flonum, 1.0), ss_argv[0]));
  case 2:  ss_return(ss_vec(3, ss_sym(ss_DIV), ss_argv[0], ss_argv[1]));
  default: ss_return(ss_vec(3, ss_sym(ss_DIV), ss_argv[0], ss_cons(ss_sym(MUL), ss_vecnv(ss_argc - 1, ss_argv + 1))));
  }
ss_end

#define PRIM_BOP(NAME,OP)                                               \
  ss ss_##NAME##W(ss a0, ss a1)                                         \
  {                                                                     \
    return (ss) (ss_word_t) ( ((ss_word_t) a0) OP ((ss_word_t) a1) );               \
  }                                                                     \
  ss ss_##NAME##I(ss a0, ss a1)                                         \
  {                                                                     \
    return (ss) (ss_word_t) ( ((ss_fixnum_t) a0) OP ((ss_fixnum_t) a1) );           \
  }                                                                     \
  ss_prim(ss_##NAME,2,2,1,#OP " z...")                                  \
  ss_return(ss_##NAME(ss_argv[0], ss_argv[1]));                         \
  ss_end                                                                \
  ss_prim(ss_##NAME##I,2,2,1,#OP " int1 int2")                        \
    ss_return(ss_##NAME##I(ss_argv[0], ss_argv[1]));                    \
  ss_end                                                                \
  ss_prim(ss_##NAME##W,2,2,1,#OP " word1 word2")                        \
    ss_return(ss_##NAME##W(ss_argv[0], ss_argv[1]));                    \
  ss_end

#define PRIM_UOP(NAME,OP)                                               \
  ss ss_##NAME##W(ss a0)                                                \
  {                                                                     \
    return (ss) (ss_word_t) (OP (ss_word_t) a0);                                    \
  }                                                                     \
  ss ss_##NAME##I(ss a0)                                                \
  {                                                                     \
    return (ss) (ss_word_t) (OP (ss_fixnum_t) a0);                                  \
  }                                                                     \
  ss_prim(ss_##NAME##W,1,1,1,#OP " word")                               \
  ss_return(ss_##NAME##W(ss_argv[0]));                                  \
  ss_end                                                                \
  ss_prim(ss_##NAME##I,1,1,1,#OP " int")                                \
  ss_return(ss_##NAME##I(ss_argv[0]));                                  \
  ss_end                                                                \
  ss_prim(ss_##NAME,1,1,1,#OP " z")                                     \
    ss_return(ss_##NAME(ss_argv[0]));                                   \
  ss_end

#define BOP(NAME,OP)                                                    \
  ss ss_##NAME(ss a0, ss a1)                                            \
  {                                                                     \
    ss_number_coerce_2(&a0, &a1);                                       \
    switch ( ss_type_te(a0) ) {                                         \
    case ss_te_fixnum:                                                  \
      return ss_box(fixnum, ss_UNB(fixnum,a0) OP ss_UNB(fixnum,a1));    \
    case ss_te_flonum:                                                  \
      return ss_box(flonum, ss_UNB(flonum,a0) OP ss_UNB(flonum,a1));    \
    default: return ss_typecheck_error(ss_t_number, a0);                \
    }                                                                   \
  }                                                                     \
  PRIM_BOP(NAME,OP)
  
#define UOP(NAME,OP)                                                    \
  ss ss_##NAME(ss a0)                                                   \
  {                                                                     \
    switch ( ss_type_te(a0) ) {                                         \
    case ss_te_fixnum:                                                  \
      return ss_box(fixnum, OP ss_UNB(fixnum,a0));                      \
    case ss_te_flonum:                                                  \
      return ss_box(flonum, OP ss_UNB(flonum,a0));                      \
    default: return ss_typecheck_error(ss_t_number, a0);                \
    }                                                                   \
  }                                                                     \
  PRIM_UOP(NAME,OP)

#define ROP(NAME,OP)                                                    \
  ss ss_##NAME(ss a0, ss a1)                                            \
  {                                                                     \
    ss_number_coerce_2(&a0, &a1);                                       \
    switch ( ss_type_te(a0) ) {                                         \
    case ss_te_fixnum:                                                  \
      return ss_box(boolean, ss_UNB(fixnum,a0) OP ss_UNB(fixnum,a1));   \
    case ss_te_flonum:                                                  \
      return ss_box(boolean, ss_UNB(flonum,a0) OP ss_UNB(flonum,a1));   \
    default: return ss_typecheck_error(ss_t_number, a0);                \
    }                                                                   \
  }                                                                     \
  PRIM_BOP(NAME,OP)

#define IBOP(NAME,OP)                                                   \
  ss ss_##NAME(ss a0, ss a1)                                            \
  {                                                                     \
    ss_typecheck(ss_t_fixnum, a0);                                      \
    ss_typecheck(ss_t_fixnum, a1);                                      \
    return ss_box(fixnum, ss_UNB(fixnum,a0) OP ss_UNB(fixnum,a1));      \
  }                                                                     \
  PRIM_BOP(NAME,OP)

#define IUOP(NAME,OP)                                                   \
  ss ss_##NAME(ss a0)                                                   \
  {                                                                     \
    ss_typecheck(ss_t_fixnum, a0);                                      \
    return ss_box(fixnum, OP ss_UNB(fixnum,a0));                        \
  }                                                                     \
  PRIM_UOP(NAME,OP)

#include "cops.def"

