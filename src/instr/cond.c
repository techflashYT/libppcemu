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

static void do_cmpli(struct _ppcemu_state *state, uint crfD, uint rA, u16 uimm) {
	u32 c, a = state->gpr[rA];

	if (a < (u32)uimm)
		c = 0b1000;
	else if (a > (u32)uimm)
		c = 0b0100;
	else /* equal */
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);

	cr_set_field(state, crfD, c);
}

static void do_cmp(struct _ppcemu_state *state, uint crfD, uint rA, uint rB) {
	u32 c, a = state->gpr[rA], b = state->gpr[rB];

	if ((i32)a < (i32)b)
		c = 0b1000;
	else if ((i32)a > (i32)b)
		c = 0b0100;
	else /* equal */
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);

	cr_set_field(state, crfD, c);
}

static void do_cmpl(struct _ppcemu_state *state, uint crfD, uint rA, uint rB) {
	u32 c, a = state->gpr[rA], b = state->gpr[rB];

	if (a < b)
		c = 0b1000;
	else if (a > b)
		c = 0b0100;
	else /* equal */
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);

	cr_set_field(state, crfD, c);
}

static void do_mfcr(struct _ppcemu_state *state, uint rD) {
	state->gpr[rD] = state->cr;
}


static void do_mtcrf(struct _ppcemu_state *state, uint rS, uint crm) {
	u32 mask;
	uint i;

	if (crm == 0xff) /* fast path for "set all" */
		state->cr = state->gpr[rS];
	else {
		/* actually mask it */
		mask = 0;
		for (i = 0; i < 8; i++) {
			if (crm & (1 << i))
				mask |= (0xf << (i * 4));
		}

		state->cr &= ~mask;
		state->cr |= (state->gpr[rS] & mask);
	}
}
