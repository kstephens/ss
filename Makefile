CFLAGS = -g
# CFLAGS += -O3
CFLAGS += -I.
CFLAGS += -Iboot
CFLAGS += -I/opt/local/include

LIBS = -L/opt/local/lib -lgc

CFILES = \
  ss.c

HFILES = \
  ss.h \
  sym.def \
  prim.def \
  cfunc.def \
  syntax.def

all : ss

sym.def : Makefile symdef.pl $(CFILES) prim.def syntax.def cfunc.def
	$(CC) $(CFLAGS) -E -Dss_sym=ss_sym $(CFILES) | perl symdef.pl > $@
#	open $@

prim.def : Makefile primdef.pl $(CFILES)
	$(CC) $(CFLAGS) -E -D_ss_prim=_ss_prim $(CFILES) | perl primdef.pl > $@

syntax.def : Makefile syntax.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -Dss_syntax=ss_syntax $(CFILES) | perl $@.pl > $@

cfunc.def : Makefile cfunc.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -Dss_prim=ss_prim -D_ss_cfunc_def=_ss_cfunc_def $(CFILES) | perl $@.pl > $@
#	cat $(CFILES) | perl $@.pl > $@

ss : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -o $@ $(CFILES) $(LIBS)

ss.s : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -S -o $@.tmp $(CFILES) $(LIBS)
	tool/asm-source $@.tmp > $@
	rm $@.tmp

ss.i : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -E -o $@ $(CFILES) $(LIBS)

clean:
	rm -f ss *.o *.s *.i *.tmp sym.def prim.def cfunc.def

