#!/usr/bin/env perl
%syms = ();
while ( <> ) {
  while ( s/\bss_sym\((\w+)\)// ) {
    $syms{$1} = 1;
  }
}
foreach ( sort keys %syms ) {
  print "ss_sym_def(", $_, ")\n";
}
print "#undef ss_sym_def\n";

