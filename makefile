BUILD_DIR         = ./build
BUILD_OBJ_DIR     = $(BUILD_DIR)/Linux/obj
BUILD_BIN_DIR     = $(BUILD_DIR)/Linux/bin
BIN               = ctren
CFLAGS            = -O2 -g -mtune=native -Wpedantic -Wall -I src/i18n/en -D CTRLANG=en -D INCLUDETESTS
LDFLAGS           = -g -rdynamic -lm -ldl
CSRCS             = $(shell find ./src -maxdepth 1 -type f -name '*.c')
COBJS             = $(patsubst ./src/%.c, $(BUILD_OBJ_DIR)/%.o, $(CSRCS))
TARGET            = $(COBJS)

all: ctr

ctr: $(TARGET)
	@mkdir -p $(BUILD_BIN_DIR)
	$(CC) $(TARGET) $(LDFLAGS) -o $(BUILD_BIN_DIR)/$(BIN)

$(BUILD_OBJ_DIR)/%.o: ./src/%.c
	@mkdir -p $(BUILD_OBJ_DIR)
	$(CC) $(CFLAGS) $(EXTRACFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_OBJ_DIR) $(BUILD_BIN_DIR)/$(BIN)

plugin:
	cd src/plugins/${PACKAGE} ; make all

plugin-clean:
	cd src/plugins/${PACKAGE} ; make clean

