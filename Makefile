
CFILES = \
  threadcomp.c

HFILES = \
  ss.h \
  sym.def \
  prim.def

all : ss

sym.def : Makefile symdef.pl $(CFILES)
	$(CC) -E -Dss_sym=ss_sym $(CFILES) | perl symdef.pl > $@
#	open $@

prim.def : Makefile primdef.pl $(CFILES)
	$(CC) -E -D_ss_prim=_ss_prim $(CFILES) | perl primdef.pl > $@

ss : $(CFILES) $(HFILES)
	cc -o $@ $(CFILES)

