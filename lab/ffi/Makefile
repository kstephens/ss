CFLAGS += -g
ifneq "$(NO_OPTIMIZE)" ""
CFLAGS += -O3
endif
CPPFLAGS += -I.

CPPFLAGS += -I/opt/local/lib/libffi-3.0.13/include

LIBS += -L/opt/local/lib
# LIBS += -L/opt/local/lib/x86_64 -ljit -ljitdynamic -ljitplus
LIBS += -lffi
LIBS += -lm

CPP = $(CC) $(CPPFLAGS) -E 

LDFLAGS += $(LIBS)

CFILES = \
  ffi-ex0.t.c \
  ss_ffi.t.c

all : $(CFILES:%.c=%)

clean :
	echo rm -f $(CFILES:%.c=%)


