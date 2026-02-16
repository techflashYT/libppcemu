/*
 * libppcemu - Internal Headers - Condition Register
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_CR_H
#define _LIBPPCEMU_INTERNAL_CR_H

#include "types.h"
#include "state.h"

#define CR0_LT (1 << 31)
#define CR0_GT (1 << 30)
#define CR0_EQ (1 << 29)
#define CR0_SO (1 << 28)

static uint cr_get_bit(struct _ppcemu_state *state, uint bit) {
	return (state->cr >> bit) & 1;
}

static void cr_set_bit(struct _ppcemu_state *state, uint bit, bool set) {
	u32 mask = 1 << bit;
	state->cr = set ? (state->cr | mask) : (state->cr & ~mask);
}

static void cr_set_field(struct _ppcemu_state *state, uint field, u32 val) {
	u32 mask = 15 << (field * 4);
	state->cr &= mask;
	state->cr |= ((val & 15) << (field * 4));
}

#endif /* _LIBPPCEMU_INTERNAL_CR_H */
