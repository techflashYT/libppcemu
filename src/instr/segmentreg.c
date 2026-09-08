/*
 * libppcemu - PowerPC Instruction handling - Segment Registers
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"

void do_mtsr(struct _ppcemu_state *state, uint sreg, uint rS) {
	state->sr[sreg] = state->gpr[rS];
}

void do_mfsr(struct _ppcemu_state *state, uint sreg, uint rD) {
	state->gpr[rD] = state->sr[sreg];
}
