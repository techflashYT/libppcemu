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

	if (a < simm) /* TODO: sign-extend */
		c = 0b1000;
	else if (a > simm) /* TODO: sign-extend */
		c = 0b0100;
	else
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);

	cr_set_field(state, crfD, c);
}
