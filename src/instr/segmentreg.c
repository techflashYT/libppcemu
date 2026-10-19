/*
 * libppcemu - PowerPC Instruction handling - Segment Registers
 *
 * Copyright (C) 2026 Techflash
 */

#include "../mem.h"
#include "../state.h"

void do_mtsr(struct _ppcemu_state *state, uint sreg, uint rS) {
	state->sr[sreg] = state->gpr[rS];
	ppcemu_tlb_flush_segment(state, sreg);
}

void do_mfsr(struct _ppcemu_state *state, uint sreg, uint rD) {
	state->gpr[rD] = state->sr[sreg];
}
