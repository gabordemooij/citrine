SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

CTRLANG = langUS

CFLAGS = -mtune=native -Wall -D$(CTRLANG)
SRC = siphash.c utf8.c memory.c util.c base.c collections.c file.c system.c \
       world.c lexer.c parser.c walker.c citrine.c
OBJ = $(SRC:.c=.o)
IMPORTLIBRARY = libctr.dll.a
EXENAME = ctr
LDLIBS = -lm -ldl 

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g
	BUILDTYPE = debug
else
    CFLAGS += -DNDEBUG -o2
	BUILDTYPE = release
endif

ifeq ($(OS), Windows_NT)
	OSPREFIX = win
	LDFLAGS += -static -L /usr/lib -Wl,--out-implib=$(BUILDDIR)/$(IMPORTLIBRARY)
	LDLIBS += -lpsapi -ltre -lsodium  -lws2_32
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		OSPREFIX = linux
		CFLAGS += -D forLinux
		LDFLAGS += -rdynamic
		LDLIBS += -lbsd
	else
		OSPREFIX = bsd
		LDFLAGS += -rdynamic
	endif
endif

BUILDDIR = build/$(OSPREFIX)-$(BUILDTYPE)
BUILDOBJS = $(addprefix $(BUILDDIR)/, $(OBJ))

all:		$(BUILDDIR)/$(EXENAME)

clean:
			if [ -d "$(BUILDDIR)" ]; then \
				rm -f $(BUILDOBJS) $(BUILDDIR)/$(IMPORTLIBRARY) $(BUILDDIR)/$(EXENAME); \
			fi

install:
ifeq ($(DEBUG), 0)
			cp $(BUILDDIR)/$(EXENAME) $(DESTDIR)/usr/bin/$(EXENAME)
else
			$(warning Debug build, not installing.)
endif

$(BUILDDIR)/$(EXENAME): $(BUILDOBJS)
			$(CC) $(BUILDOBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@
 
$(BUILDDIR)/%.o: %.c
			mkdir -p $(BUILDDIR)
			$(CC) -c $(CFLAGS) -o $@ $<
