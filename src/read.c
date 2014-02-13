#define VALUE ss
#define READ_DECL ss ss_read(ss_s_env *ss_env, ss stream)
#define READ_CALL() ss_read(ss_env, stream)
#define GETC(stream) getc(FP(stream))
#define UNGETC(stream,c) ungetc(c, FP(stream))
#define EQ(X,Y) ((X) == (Y))
#define NIL ss_nil
#define EOS ss_eos
#define CONS ss_cons
#define CAR ss_car
#define T ss_t
#define F ss_f
#define E ss_eos
#define BRACKET_LISTS 1
#define SET_CDR(C,R) (ss_CDR(C) = (R))
#define MAKE_CHAR(I) ss_c((I))
#define LIST_2_VECTOR(X) ss_list_to_vector(X)
#define STRING(S,L) ss_strnv((L), (S))
#define SYMBOL_DOT ss_sym(DOT)
#define SYMBOL(N) ss_sym(N)
#define STRING_2_NUMBER(s, radix) ss_string_TO_number(s, radix)
#define STRING_2_SYMBOL(s) ss_box(symbol, ss_string_V(s))
#define ERROR(fmt,args...) ss_error(ss_env, "read", stream, fmt, ##args)
#define RETURN(X) return X
#define MALLOC(S) GC_malloc_atomic(S)
#define REALLOC(P,S) GC_realloc(P,S)
#define FREE(P) GC_free(P)
#define CALL_MACRO_CHAR(c) ss_apply_sym(ss_call_macro_char, 2, c, stream)
#include "lispread/lispread.c"

