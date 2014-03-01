  switch ( ss_argc ) {
  case 0:
    R(((T(*)()) ss_prim->c_func)());
  case 1:
    R(((T(*)(T)) ss_prim->c_func)(A(ss_argv[0])));
  case 2:
    R(((T(*)(T,T)) ss_prim->c_func)(A(ss_argv[0]), A(ss_argv[1])));
  case 3:
    R(((T(*)(T,T,T)) ss_prim->c_func)(A(ss_argv[0]), A(ss_argv[1]), A(ss_argv[2])));
  case 4:
    R(((T(*)(T,T,T,T)) ss_prim->c_func)(A(ss_argv[0]), A(ss_argv[1]), A(ss_argv[2]), A(ss_argv[3])));
  case 5:
    R(((T(*)(T,T,T,T,T)) ss_prim->c_func)(A(ss_argv[0]), A(ss_argv[1]), A(ss_argv[2]), A(ss_argv[3]), A(ss_argv[4])));
  case 6:
    R(((T(*)(T,T,T,T,T,T)) ss_prim->c_func)(A(ss_argv[0]), A(ss_argv[1]), A(ss_argv[2]), A(ss_argv[3]), A(ss_argv[4]), A(ss_argv[5])));
  case 7:
    R(((T(*)(T,T,T,T,T,T,T)) ss_prim->c_func)(A(ss_argv[0]), A(ss_argv[1]), A(ss_argv[2]), A(ss_argv[3]), A(ss_argv[4]), A(ss_argv[5]), A(ss_argv[6])));
  default: abort();
  }
#undef T
#undef A
#undef R
