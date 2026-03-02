/*
 * libppcemu - PowerPC Instruction handling - Conditionals
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"
#include "../cr.h"
#include <ppcemu/spr.h>

static void do_cmpi(struct _ppcemu_state *state, uint crfD, uint rA, u16 simm) {
	u32 c, a = state->gpr[rA];

	if ((i32)a < (i32)(i16)simm)
		c = 0b1000;
	else if ((i32)a > (i32)(i16)simm)
		c = 0b0100;
	else /* equal */
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);

	cr_set_field(state, crfD, c);
}

static void do_mfcr(struct _ppcemu_state *state, uint rD) {
	state->gpr[rD] = state->cr;
}
