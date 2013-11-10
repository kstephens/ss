#!/usr/bin/env perl
%syms = ();
while ( <> ) {
  while ( s/\b_ss_prim\(([^)]+)\)// ) {
    $syms{$1} = 1;
  }
}
foreach ( sort keys %syms ) {
  print "ss_prim_def(", $_, ")\n";
}
print "#undef ss_prim_def\n";

