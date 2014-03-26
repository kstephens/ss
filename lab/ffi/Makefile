CFLAGS += -g
ifneq "$(NO_OPTIMIZE)" ""
CFLAGS += -O3
endif
#CFLAGS += -v
CPPFLAGS += -I.

CPPFLAGS += -I/opt/local/lib/libffi-3.0.13/include # MacPorts
CPPFLAGS += -I/usr/include/x86_64-linux-gnu # Linux

LIBS += -L/opt/local/lib # MacPorts
LIBS += -L/usr/lib/x86_64-linux-gnu # Linux libffi.a
# LIBS += -L/opt/local/lib/x86_64 -ljit -ljitdynamic -ljitplus
LIBS += -lffi
LIBS += -lm

CPP = $(CC) $(CPPFLAGS) -E 

LDLIBS += $(LIBS)

CFILES = \
  ffi-ex0.t.c \
  ss_ffi.t.c

#.c.i:
#	$(CPP) $< -o $@

all : $(CFILES:%.c=%)

clean :
	rm -f $(CFILES:%.c=%)


