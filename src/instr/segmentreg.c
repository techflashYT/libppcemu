/*
 * libppcemu - PowerPC Instruction handling - Segment Registers
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#include "../state.h"

static void do_mtsr(struct _ppcemu_state *state, uint sreg, uint rS) {
	u32 val;

	val = state->gpr[rS];
	if (!(val & 0x80000000))
		puts("Writing a non-direct-store segment which will not be interpreted!!!");

	state->sr[sreg] = val;
}

static void do_mfsr(struct _ppcemu_state *state, uint sreg, uint rD) {
	state->gpr[rD] = state->sr[sreg];
}
