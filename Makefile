CFLAGS += -g
ifneq "$(NO_OPTIMIZE)" ""
CFLAGS += -O3
endif
CPPFLAGS += -I.
CPPFLAGS += -Iinclude
CPPFLAGS += -Igen
CPPFLAGS += -Iboot
CPPFLAGS += -Isrc
CPPFLAGS += -I/opt/local/include
# CFLAGS += -Wall
CFLAGS += $(CPPFLAGS)
CFLAGS += -Wno-deprecated-declarations
CFLAGS += -Wno-int-to-void-pointer-cast
CFLAGS += -Wno-unused-label
CFLAGS += -Wno-implicit-function-declaration
CFLAGS += -Wno-incompatible-pointer-types-discards-qualifiers

CC=colorgcc

LIBS += -L/opt/local/lib
ifneq "$(NO_GC)" ""
CFLAGS += -DNO_GC=$(NO_GC)
else
LIBS += -lgc
endif
LIBS += -L/opt/local/lib/x86_64 -ljit -ljitdynamic -ljitplus
LIBS += -lm

CPP = $(CC) $(CPPFLAGS) -E 

CFILES = \
  ss.c

HFILES = \
  ss.h \
  gen/t.def \
  gen/sym.def \
  gen/prim.def \
  gen/syntax.def \
  gen/cdefine.def \
  gen/cwrap.def \
  lispread/lispread.c \
  include/ss/*.h \
  $(OTHER_C_FILES)

OTHER_C_FILES = \
  src/*.c \
  src/*.h \
  src/*.def

EARLY_FILES = \
boot/t.def \
boot/sym.def    \
boot/prim.def   \
boot/syntax.def \
boot/cdefine.def \
boot/cwrap.def

SILENT=@

all : ss

boot/t.def      : gen/t.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@
boot/sym.def    : gen/sym.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@
boot/prim.def   : gen/prim.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@
boot/syntax.def : gen/syntax.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@
boot/cwrap.def  : gen/cwrap.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@
boot/cdefine.def  : gen/cdefine.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@

gen/t.def : Makefile gen/t.def.gen $(CFILES) $(OTHER_C_FILES) gen/cwrap.def
	@echo "GEN $@"
	$(SILENT)$(CPP) $(CFILES) | tee $@.i | $@.gen > $@.tmp; mv $@.tmp $@
gen/sym.def : Makefile gen/sym.def.gen $(CFILES) $(OTHER_C_FILES) gen/prim.def gen/syntax.def
	@echo "GEN $@"
	$(SILENT)$(CPP) -Dss_sym=ss_sym $(CFILES) | tee $@.i | $@.gen > $@.tmp; mv $@.tmp $@
gen/prim.def : Makefile gen/prim.def.gen $(CFILES)
	@echo "GEN $@"
	$(SILENT)$(CPP) -D_ss_prim=_ss_prim $(CFILES) $(OTHER_C_FILES) | tee $@.i | $@.gen > $@.tmp; mv $@.tmp $@
gen/syntax.def : Makefile gen/syntax.def.gen $(CFILES)
	@echo "GEN $@"
	$(SILENT)$(CPP) -Dss_syntax=ss_syntax $(CFILES) $(OTHER_C_FILES)| tee $@.i | $@.gen > $@.tmp; mv $@.tmp $@
gen/cwrap.def : Makefile gen/cwrap.def.gen $(CFILES) $(OTHER_C_FILES)
	@echo "GEN $@"
	$(SILENT)$(CPP) $(CFILES) $(OTHER_C_FILES)| tee $@.i | $@.gen > $@.tmp; mv $@.tmp $@
gen/cdefine.def : Makefile gen/cdefine.def.gen $(CFILES) $(OTHER_C_FILES)
	$(SILENT)$(CPP) -dM $(CFILES) $(OTHER_C_FILES) | tee $@.i | $@.gen > $@.tmp; mv $@.tmp $@

lispread/lispread.c:
	git submodule init
	git submodule update

early-files : $(EARLY_FILES)

ss : $(EARLY_FILES) $(CFILES) $(HFILES) $(OTHER_C_FILES)
	@echo "LINK $@"
	$(SILENT)$(CC) $(CFLAGS) -Dss_cwrap_c=1 -o $@ ss.c $(LIBS)

ss.s : early-files $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -Dss_cwrap_c=1 -S -o $@.tmp $(CFILES) $(LIBS)
	tool/asm-source $@.tmp > $@
	rm $@.tmp

ss.i : early-files $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -E -Dss_cwrap_c=1 -o $@ $(CFILES) $(LIBS)

system-defines :
	$(CC) $(CFLAGS) -E -dM - < /dev/null | sort

prog-defines :
	$(CC) $(CFLAGS) -E -dM $(CFILES) | sort

test : all
	echo '(load "t/test.scm")' | ./ss

sh : all
	./sssh

db : all
	lldb -f ./ss

db-test : all
	lldb -f ./ss t/test.scm

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
	rm -f ss *.o *.s *.i *.tmp gen/*.def gen/*.i boot/*.def

code-stats :
	tool/code-stats *.[hc] lib src gen/*.gen

