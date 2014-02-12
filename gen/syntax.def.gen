#!/usr/bin/env perl
%syms = ();
while ( <> ) {
  while ( s/\bss_syntax\(([^)]+)\)// ) {
    $syms{$1} = 1;
  }
}
foreach ( sort keys %syms ) {
  print "ss_syntax_def(", $_, ")\n";
}
print "#undef ss_syntax_def\n";

