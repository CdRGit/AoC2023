TARGET_EXEC ?= day14

CC=clang

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

DEBUG_CFLAGS=-g

REL_CFLAGS=-O3 -flto=thin -pipe -funroll-loops

.PHONY: all
all: CFLAGS += $(DEBUG_CFLAGS)
all:
	make clean
	make $(BUILD_DIR)/$(TARGET_EXEC) CFLAGS='$(CFLAGS)'

.PHONY: release
release: CFLAGS += $(REL_CFLAGS)
release:
	make clean
	make $(BUILD_DIR)/$(TARGET_EXEC) CFLAGS='$(CFLAGS)'

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
