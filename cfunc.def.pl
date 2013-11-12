#!/usr/bin/env perl
%syms = ();
while ( <> ) {
  chomp;
  if ( /^([a-z0-9_]{2,}[*]*)\s+([a-z0-9_]{3,})(\([^)]+\))/i ) {
    $type = $1;
    $func = $2;
    $args = $3;
    next if ( $func eq '__typeof__' );
    next if ( /^(ss_prim|ss_syntax|ss_end)/ );
    if ( 0 ) {
      print STDERR "  line = ", $_, "\n";
      print STDERR "    type = ", $type, "\n";
      print STDERR "    func = ", $func, "\n";
      print STDERR "    args = ", $args, "\n";
    }
    $key = "\"$type\",$func,\"$args\"";
    $syms{$key} = 1;
  }
}
foreach ( sort keys %syms ) {
  print "ss_cfunc_def(", $_, ")\n";
}
print "#undef ss_cfunc_def\n";

