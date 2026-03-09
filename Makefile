#
# libppcemu - Top-level Makefile
# Copyright (C) 2026 Techflash
#

override SRC := init.c run.c mem.c decode.c state.c cache.c
override SRC += instr/arith.c instr/bitwise.c instr/branch.c instr/cache.c
override SRC += instr/cond.c instr/floating_point.c instr/loadstore.c
override SRC += instr/msr.c instr/paired_singles.c instr/segmentreg.c
override SRC += instr/spr.c

override OBJ := $(patsubst %.c,build/%.o,$(SRC))
override OUT_SO := bin/libppcemu.so
override OUT_ST := bin/libppcemu.a

CFLAGS ?= -Wall -Wextra -Wformat=2 -Wno-variadic-macros -Wno-format-nonliteral -std=gnu89 -pedantic -fno-permissive -g -O3 -flto=auto
override CFLAGS := $(CFLAGS) -fPIC -Iinclude

LDFLAGS ?= -flto=auto

.PHONY: all clean FORCE
all: $(OUT_SO) $(OUT_ST)

$(OUT_SO): $(OBJ)
	@mkdir -p $(@D)
	$(info $s  LD    $@)
	@$(CC) $(LDFLAGS) -shared $^ -o $@

$(OUT_ST): $(OBJ)
	@mkdir -p $(@D)
	$(info $s  AR    $@)
	@$(AR) cr $@ $^

build/%.o: src/%.c
	@mkdir -p $(@D)
	$(info $s  CC    $<)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf bin build
