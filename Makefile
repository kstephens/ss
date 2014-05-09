UNAME_S:=$(shell uname -s 2>/dev/null)#
CFLAGS += -g
ifneq "$(NO_OPTIMIZE)" ""
CFLAGS += -O3
endif
CPPFLAGS += $(CINCLUDES)
CFLAGS += $(CPPFLAGS)
LDLIBS += $(LIBS)
CPP = $(CC) $(CPPFLAGS) -E

CINCLUDES += -I.
CINCLUDES += -Iinclude
CINCLUDES += -Igen
CINCLUDES += -Iboot
CINCLUDES += -Isrc

ifeq "$(UNAME_S)" "Linux"
CC=colorgcc
endif

ifneq "$(NO_GC)" ""
CFLAGS += -DNO_GC=$(NO_GC)
else
LIBS += -lgc
endif

ifeq "$(UNAME_S)" "Darwin"
CINCLUDES += -I/opt/local/include
CINCLUDES += -I/opt/local/lib/libffi-3.0.13/include # MacPorts
CINCLUDES += -fmacro-backtrace-limit=0
LIBS += -L/opt/local/lib
LIBS += -L/opt/local/lib/x86_64 -ljit -ljitdynamic -ljitplus
endif
ifeq "$(UNAME_S)" "Linux"
CINCLUDES += -I/usr/include/x86_64-linux-gnu # libffi
LIBS      += -L/usr/lib/x86_64-linux-gnu     # libffi
endif
LIBS += -lffi
LIBS += -lm

# CFLAGS += -Wall
# CLANG only?
CFLAGS += -Wno-deprecated-declarations
CFLAGS += -Wno-int-to-void-pointer-cast
CFLAGS += -Wno-unused-label
CFLAGS += -Wno-implicit-function-declaration
CFLAGS += -Wno-incompatible-pointer-types-discards-qualifiers

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
lispread/lispread.c \
boot/t.def \
boot/sym.def    \
boot/prim.def   \
boot/syntax.def \
boot/cdefine.def \
boot/cwrap.def

SILENT=@

all : show-uname ss

show-uname :
	@echo "  UNAME_S=$(UNAME_S)"

boot/%.def : gen/%.def.gen
	@echo "GEN $@"
	$(SILENT)$< </dev/null >$@

gen/%.def : gen/%.def.gen
	@echo "GEN $@"
	$(SILENT) d=_ss_$(notdir $(basename $@)); $(CPP) $($(notdir $(basename $@)_CPP_OPTS)) -D$$d=$$d $(CFILES) $(OTHER_C_FILES) | tee $@.i | $@.gen > $@.tmp; mv $@.tmp $@; wc -l $@

cdefine_CPP_OPTS=-dM

gen/*.def : Makefile $(CFILES) $(OTHER_C_FILES)

gen/t.def   : gen/cwrap.def
gen/sym.def : gen/prim.def gen/syntax.def

lispread/lispread.c:
	git submodule init
	git submodule update

early-files : $(EARLY_FILES)

ss : $(EARLY_FILES) $(CFILES) $(HFILES) $(OTHER_C_FILES)
	@echo "LINK $@"
	$(SILENT)$(CC) $(CFLAGS) -Dss_cwrap_c=1 -o $@ ss.c $(LDLIBS)

ss.s : early-files $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -Dss_cwrap_c=1 -S -o $@.tmp $(CFILES) $(LDLIBS)
	tool/asm-source $@.tmp > $@
	rm $@.tmp

ss.i : early-files $(CFILES) $(HFILES)
	$(CPP) -Dss_cwrap_c=1 -o $@ $(CFILES) $(LDLIBS)

system-defines :
	$(CPP) -dM - < /dev/null | sort

prog-defines :
	$(CPP) -dM $(CFILES) | sort

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
	tool/code-stats *.[hc] lib src include gen/*.gen lispread/*.c

