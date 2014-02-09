CFLAGS += -std=c99 
CFLAGS += -g
#CFLAGS += -O3
CFLAGS += -I.
CFLAGS += -Igen
CFLAGS += -Iboot
CFLAGS += -I/opt/local/include

LIBS += -L/opt/local/lib
ifneq "$(NO_GC)" ""
CFLAGS += -DNO_GC=$(NO_GC)
else
LIBS += -lgc
endif

CFILES = \
  ss.c

HFILES = \
  ss.h \
  gen/sym.def \
  gen/prim.def \
  gen/cfunc.def \
  gen/syntax.def \
  lispread/lispread.c

EARLY_FILES = \
boot/sym.def    \
boot/prim.def   \
boot/syntax.def \
boot/cfunc.def

all : ss

boot/sym.def    : gen/sym.def.pl
	perl $< /dev/null >$@
boot/prim.def   : gen/prim.def.pl
	perl $< /dev/null >$@
boot/syntax.def : gen/syntax.def.pl
	perl $< /dev/null >$@
boot/cfunc.def  : gen/cfunc.def.pl
	perl $< /dev/null >$@

gen/sym.def : Makefile gen/sym.def.pl $(CFILES) gen/prim.def gen/syntax.def gen/cfunc.def
	$(CC) $(CFLAGS) -E -Dss_sym=ss_sym $(CFILES) | perl $@.pl > $@
#	open $@

gen/prim.def : Makefile gen/prim.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -D_ss_prim=_ss_prim $(CFILES) | perl $@.pl > $@

gen/syntax.def : Makefile gen/syntax.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -Dss_syntax=ss_syntax $(CFILES) | perl $@.pl > $@

gen/cfunc.def : Makefile gen/cfunc.def.pl $(CFILES)
	$(CC) $(CFLAGS) -E -Dss_prim=ss_prim -D_ss_cfunc_def=_ss_cfunc_def $(CFILES) | perl $@.pl > $@

lispread/lispread.c:
	git submodule init
	git submodule update

ss : $(EARLY_FILES) $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -o $@ $(CFILES) $(LIBS)

ss.s : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -S -o $@.tmp $(CFILES) $(LIBS)
	tool/asm-source $@.tmp > $@
	rm $@.tmp

ss.i : $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -E -o $@ $(CFILES) $(LIBS)

TEST_FILE = t/test*.scm
test : all
	errors=0; for f in $(TEST_FILE) ;\
	do \
          cmd="./ss < $$f" ;\
	  echo " + $${cmd}" ;\
	  if ! eval $$cmd ;\
	  then \
            echo " ! $${cmd}" ;\
	    errors=1 ;\
	  fi; \
	done ;\
	exit $$errors

clean:
	rm -f ss *.o *.s *.i *.tmp gen/*.def boot/*.def

