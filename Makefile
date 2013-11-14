CFLAGS += -std=c99 
CFLAGS += -g
#CFLAGS += -O3
CFLAGS += -I.
CFLAGS += -Igen
CFLAGS += -Iboot
CFLAGS += -I/opt/local/include

LIBS += -L/opt/local/lib
LIBS += -lgc

CFILES = \
  ss.c

HFILES = \
  ss.h \
  gen/sym.def \
  gen/prim.def \
  gen/cfunc.def \
  gen/syntax.def

all : ss

gen/sym.def : Makefile gen/sym.def.pl $(CFILES) gen/prim.def gen/syntax.def gen/cfunc.def
	$(CC) $(CFLAGS) -E -Dss_sym=ss_sym $(CFILES) | perl $@.pl > $@
#	open $@

gen/prim.def : Makefile gen/prim.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -D_ss_prim=_ss_prim $(CFILES) | perl $@.pl > $@

gen/syntax.def : Makefile gen/syntax.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -Dss_syntax=ss_syntax $(CFILES) | perl $@.pl > $@

gen/cfunc.def : Makefile gen/cfunc.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -Dss_prim=ss_prim -D_ss_cfunc_def=_ss_cfunc_def $(CFILES) | perl $@.pl > $@

ss : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -o $@ $(CFILES) $(LIBS)

ss.s : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -S -o $@.tmp $(CFILES) $(LIBS)
	tool/asm-source $@.tmp > $@
	rm $@.tmp

ss.i : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -E -o $@ $(CFILES) $(LIBS)

clean:
#	rm -f ss *.o *.s *.i *.tmp sym.def prim.def syntax.def cfunc.def
	rm -f ss *.o *.s *.i *.tmp gen/*.def

