CFLAGS += -std=c99 
CFLAGS += -g
CFLAGS += -O3
CFLAGS += -I.
CFLAGS += -Igen
CFLAGS += -Iboot
CFLAGS += -I/opt/local/include
CFLAGS += -Wall
CFLAGS += -Wno-deprecated-declarations
CFLAGS += -Wno-int-to-void-pointer-cast
CFLAGS += -Wno-unused-label

LIBS += -L/opt/local/lib
ifneq "$(NO_GC)" ""
CFLAGS += -DNO_GC=$(NO_GC)
else
LIBS += -lgc
endif
LIBS += -lm

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

SILENT=@

all : ss

boot/sym.def    : gen/sym.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@
boot/prim.def   : gen/prim.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@
boot/syntax.def : gen/syntax.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@
boot/cfunc.def  : gen/cfunc.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@

gen/sym.def : Makefile gen/sym.def.gen $(CFILES) gen/prim.def gen/syntax.def gen/cfunc.def
	@echo "GEN $@"
	$(SILENT)$(CC) $(CFLAGS) -E -Dss_sym=ss_sym $(CFILES) | $@.gen > $@
gen/prim.def : Makefile gen/prim.def.gen $(CFILES)
	@echo "GEN $@"
	$(SILENT)$(CC) $(CFLAGS) -E -D_ss_prim=_ss_prim $(CFILES) | $@.gen > $@
gen/syntax.def : Makefile gen/syntax.def.gen $(CFILES)
	@echo "GEN $@"
	$(SILENT)$(CC) $(CFLAGS) -E -Dss_syntax=ss_syntax $(CFILES) | $@.gen > $@
gen/cfunc.def : Makefile gen/cfunc.def.gen $(CFILES)
	@echo "GEN $@"
	$(SILENT)$(CC) $(CFLAGS) -E -Dss_prim=ss_prim -D_ss_cfunc_def=_ss_cfunc_def $(CFILES) | $@.gen > $@

lispread/lispread.c:
	git submodule init
	git submodule update

ss : $(EARLY_FILES) $(CFILES) $(HFILES)
	@echo "LINK $@"
	$(SILENT)$(CC) $(CFLAGS) -o $@ $(CFILES) $(LIBS)

ss.s : $(EARLY_FILES) $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -S -o $@.tmp $(CFILES) $(LIBS)
	tool/asm-source $@.tmp > $@
	rm $@.tmp

ss.i : $(EARLY_FILES) $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -E -o $@ $(CFILES) $(LIBS)

test : all
	echo '(load "t/test.scm")' | ./ss

TEST_FILE = t/test*.scm
test-file : all
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

