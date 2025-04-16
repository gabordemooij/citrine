#
# Makefile
#
CC ?= gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= .



WARNINGS ?= -Wall -Wshadow -Wundef -Wno-discarded-qualifiers -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith \
					-fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits \
					-Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security \
					-Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body \
					-Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -std=gnu99
CFLAGS 			?= ${EXTRACFLAGS} -D LIBSECRET -D FFI -D LIBCURL -shared -O3 -g0 -I ./i18n/${ISO}/ -I ../../i18n/${ISO} -I ../media/i18n/${ISO} -I$(LVGL_DIR)/ $(WARNINGS) -fPIC


CFLAGS += `pkg-config --cflags libsecret-1`

CXXFLAGS        ?= -fPIC
LDFLAGS 		=  -lcurl -lSDL2 -lSDL2_ttf -lSDL2_image -lm -shared

LDFLAGS += `pkg-config --libs libsecret-1`

BIN 			= libctrgui.so
BUILD_DIR 		= ../../../build/Linux
BUILD_OBJ_DIR 	= $(BUILD_DIR)/obj/mods/gui
BUILD_BIN_DIR 	= $(BUILD_DIR)/bin/mods/gui

prefix ?= /usr
bindir ?= $(prefix)/bin

#Collect the files to compile
MAINSRC = ./gui.c

include $(LVGL_DIR)/lvgl/lvgl.mk
CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c json.c vault.c fficonnect.c jsmn/jsmn.c
OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:.cpp=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC) $(CXXSRCS)
OBJS = $(AOBJS) $(COBJS) $(CXXOBJS) $(MAINOBJ)
TARGET = $(addprefix $(BUILD_OBJ_DIR)/, $(patsubst ./%, %, $(OBJS)))

all: default

$(BUILD_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX)  $(CXXFLAGS) -c $< -o $@
	@echo "CXX $<"

$(BUILD_OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

$(BUILD_OBJ_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
	
default: $(TARGET)
	@mkdir -p $(dir $(BUILD_BIN_DIR)/)
	$(CXX) -o $(BUILD_BIN_DIR)/$(BIN) $(TARGET) $(LDFLAGS)

clean: 
	rm -rf $(BUILD_OBJ_DIR)/*
	rm -rf $(BUILD_BIN_DIR)/*
