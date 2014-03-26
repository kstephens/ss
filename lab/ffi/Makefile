CFLAGS += -g
ifneq "$(NO_OPTIMIZE)" ""
CFLAGS += -O3
endif
#CFLAGS += -v
CPP = $(CC) $(CPPFLAGS) -E 
LDLIBS += $(LIBS)

CPPFLAGS += -I.
CPPFLAGS += -I../../lib # ggrt.[hc]

CPPFLAGS += -I/opt/local/lib/libffi-3.0.13/include # MacPorts
CPPFLAGS += -I/usr/include/x86_64-linux-gnu # Linux

LIBS += -L/opt/local/lib # MacPorts
LIBS += -L/usr/lib/x86_64-linux-gnu # Linux libffi.a
LIBS += -lffi
LIBS += -lm

CPPFLAGS += -DUNIT_TEST

CFILES := $(shell ls *.t.c)

TFILES = $(CFILES:%.c=%)

all : $(TFILES)

clean :
	rm -f $(TFILES)

test : all
	for t in $(TFILES); \
	do \
	  echo "$$t" ;\
	  ./$$t ;\
	done

