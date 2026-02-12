/*
 * libppcemu - PowerPC Instruction handling - Branches
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"
#include <ppcemu/spr.h>

static void do_rfi(struct _ppcemu_state *state, u32 inst) {
	state->pc = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_SRR0)];
	/* TODO: mask MSR */
	state->msr = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_SRR1)];

	state->branched = true;
}

static void do_branch(struct _ppcemu_state *state, u32 li, uint aa, uint lk) {
	u32 oldpc = state->pc;

	if (aa)
		state->pc = li << 2; /* TODO: sign-extend */
	else
		state->pc += (li << 2); /* TODO: sign-extend */

	if (lk)
		state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_LR)] = oldpc + 4;

	state->branched = true;
}
