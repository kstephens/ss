CFLAGS = -g -I/opt/local/include

LIBS = -L/opt/local/lib -lgc

CFILES = \
  ss.c

HFILES = \
  ss.h \
  sym.def \
  prim.def

all : ss

sym.def : Makefile symdef.pl $(CFILES) prim.def
	$(CC) $(CFLAGS) -E -Dss_sym=ss_sym $(CFILES) | perl symdef.pl > $@
#	open $@

prim.def : Makefile primdef.pl $(CFILES)
	$(CC) $(CFLAGS) -E -D_ss_prim=_ss_prim $(CFILES) | perl primdef.pl > $@

ss : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -o $@ $(CFILES) $(LIBS)

clean:
	rm -f ss *.o *.def

