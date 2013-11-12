CFLAGS = -g -I/opt/local/include
CFLAGS += -O3

LIBS = -L/opt/local/lib -lgc

CFILES = \
  ss.c

HFILES = \
  ss.h \
  sym.def \
  prim.def

all : ss

sym.def : Makefile symdef.pl $(CFILES) prim.def cfunc.def
	$(CC) $(CFLAGS) -E -Dss_sym=ss_sym $(CFILES) | perl symdef.pl > $@
#	open $@

prim.def : Makefile primdef.pl $(CFILES)
	$(CC) $(CFLAGS) -E -D_ss_prim=_ss_prim $(CFILES) | perl primdef.pl > $@

cfunc.def : Makefile cfunc.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -Dss_prim=ss_prim -D_ss_cfunc_def=_ss_cfunc_def $(CFILES) | perl $@.pl > $@
#	cat $(CFILES) | perl $@.pl > $@

ss : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -o $@ $(CFILES) $(LIBS)

ss.s : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -S -o $@.tmp $(CFILES) $(LIBS)
	tool/asm-source $@.tmp > $@
	rm $@.tmp

clean:
	rm -f ss *.o *.s *.tmp sym.def prim.def cfunc.def

