-include .config

CROSS_COMPILE = $(CONFIG_CROSS_COMPILE:"%"=%)

TOP_DIR := $(shell pwd)

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
CFLAGS = -I$(TOP_DIR)/include -Wall -O2

ifdef DEFAULT_DEFINE
DEFAULT_DEFINE := $(DEFAULT_DEFINE:"%"=%)
CFLAGS := $(CFLAGS) -D$(DEFAULT_DEFINE)
endif

ifdef DEFAULT_DATA_FILE
DEFAULT_DATA_FILE := $(DEFAULT_DATA_FILE:"%"=%)
CFLAGS := $(CFLAGS) -D__DATA_FILE__=\"$(TOP_DIR)/$(DEFAULT_DATA_FILE)\"
endif

builtin-obj = built-in.o

export CC LD CFLAGS builtin-obj

DEF_CONFIG_PATH = build/config
DEF_CONFIG_LIST = $(shell cd $(DEF_CONFIG_PATH) && ls *_config)

include build/rules/common.mk

BUILD_OBJ := $(BUILD_OBJ:"%"=%)
dir-y := $(BUILD_OBJ)_main
dir-y += tty font graphic

subdir-objs := $(foreach n, $(dir-y), $(n)/$(builtin-obj))

all: $(dir-y) $(BUILD_OBJ)
	@echo

$(BUILD_OBJ): $(subdir-objs)
	@$(CC) $(CFLAGS) $^ -o $@

.PHONY: $(dir-y)
$(dir-y):
	@make $(obj_build)$@

.PHONY: $(DEF_CONFIG_LIST)
$(DEF_CONFIG_LIST):
	@echo "configure for "$(BUILD_OBJ)" "$(@:%_defconfig=%)
	@cp -v $(DEF_CONFIG_PATH)/$@ $(TOP_DIR)/.config
	@echo

.PHONY: install
install:
	@mv tetris /usr/bin

.PHONY: clean
clean:
	@for dir in $(dir-y); do \
		make $(obj_build)$$dir clean; \
	 done
	@rm -vf *.o
	@rm -vf tetris
	@echo

.PHONY: distclean
distclean: clean
	@rm -vf .config
	@echo
