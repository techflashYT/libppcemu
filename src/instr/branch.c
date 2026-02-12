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
