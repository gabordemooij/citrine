SHELL = /bin/sh

CTRLANG = langUS
CFLAGS = -mtune=native -Wall
ALL_CFLAGS = $(REQUIRED_CFLAGS) $(CFLAGS)
SRC = siphash.c utf8.c memory.c util.c base.c collections.c file.c system.c \
       world.c lexer.c parser.c walker.c citrine.c
OBJ = $(addprefix $(BUILDDIR)/, $(SRC:%.c=%.o))
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
	IMPORTLIBRARY = $(BUILDDIR)/libctr.a
	LDFLAGS += -static -L /usr/lib -Wl,--out-implib=$(IMPORTLIBRARY)
	PLUGIN_LDFLAGS = -L $(BUILDDIR)
	PLUGIN_LDLIBS = -lctr
	LDLIBS += -lpsapi -ltre -lsodium  -lws2_32
	LIBRARYSUFFIX = .dll
else
	UNAME_S = $(shell uname -s)
	LDFLAGS += -rdynamic
	LIBRARYSUFFIX = .so
	REQUIRED_PLUGIN_FLAGS = -fpic
	ifeq ($(UNAME_S), Linux)
		OSPREFIX = linux
		REQUIRED_CFLAGS += -D forLinux
		LDLIBS += -lbsd
	else
		OSPREFIX = bsd
	endif
endif

BUILDDIR = build/$(OSPREFIX)-$(BUILDTYPE)
DEP = $(OBJ:%.o=%.d)

all:		$(BUILDDIR)/$(EXENAME) 	plugins

clean:
			if [ -d "$(BUILDDIR)" ]; then \
				rm -f $(OBJ) $(DEP) $(IMPORTLIBRARY) $(BUILDDIR)/$(EXENAME); \
			fi

install:
ifeq ($(DEBUG), 0)
			cp $(BUILDDIR)/$(EXENAME) $(DESTDIR)/usr/bin/$(EXENAME)
else
			$(warning Debug build, not installing.)
endif

plugins:	$(BUILDDIR)/mods/libctrpercolator$(LIBRARYSUFFIX) \
			$(BUILDDIR)/mods/libctrcurl$(LIBRARYSUFFIX)

.PHONY: clean plugins

.SECONDEXPANSION:
percolator_SRC = percolator.c
percolator_OBJ = $(percolator_SRC:%.c=$(BUILDDIR)/mods/percolator/%.o)

curl_SRC = src/curl.c
curl_OBJ = $(curl_SRC:%.c=$(BUILDDIR)/mods/curl/%.o)
curl_REQUIRED_FLAGS = -DCTRPLUGIN_CURL_CURLOPT_URL
curl_LDLIBS =-lcurl

request_SRC = request.c
request_OBJ = $(request_SRC:%.c=$(BUILDDIR)/mods/request/%.o)
request_EXTDEPS = $(BUILDDIR)/plugin/ext/ccgi/libccgi.a
request_LDFLAGS = -L $(BUILDDIR)/plugins/ext/ccgi
request_LDLIBS = -lccgi

$(BUILDDIR)/$(EXENAME) $(IMPORTLIBRARY): $(OBJ)
			$(CC) $(OBJ) $(ALL_CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILDDIR)/mods/libctr%$(LIBRARYSUFFIX): $$($$*_OBJ) $$($$*_EXTDEPS)
			$(CC) $^ -shared $(CFLAGS) $(PLUGIN_LDFLAGS) $($*_LDFLAGS) $(PLUGIN_LDLIBS) $($*_LDLIBS) -o $@

$(BUILDDIR)/%.o: %.c
			mkdir -p $(@D)
			$(CC) $(ALL_CFLAGS) -MMD -c $< -o $@

$(BUILDDIR)/mods/curl/%.o: REQUIRED_FLAGS = $(curl_REQUIRED_FLAGS)

$(BUILDDIR)/mods/%.o: plugins/%.c citrine.h
			mkdir -p $(@D)
			$(CC) -c $(REQUIRED_FLAGS) $(REQUIRED_PLUGIN_FLAGS) $(CFLAGS) -c $< -o $@

$(request_EXTDEPS):
			if [ ! -d "$(@D)" ]; then \
				mkdir -p $(@D); \
				cp -r plugins/request/ccgi-1.2/* $(@D); \
			fi
			$(MAKE) -C $(@D)

.PRECIOUS: $(BUILDDIR)/mods/%.o

-include $(DEP)
